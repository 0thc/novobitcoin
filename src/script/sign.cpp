// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "script/sign.h"
#include "key.h"
#include "keystore.h"
#include "policy/policy.h"
#include "primitives/transaction.h"
#include "script/standard.h"
#include "taskcancellation.h"
#include "uint256.h"
#include "config.h"

TransactionSignatureCreator::TransactionSignatureCreator(
    const CKeyStore *keystoreIn, const CTransaction *txToIn, unsigned int nInIn,
    const Amount amountIn, SigHashType sigHashTypeIn)
    : BaseSignatureCreator(keystoreIn), txTo(txToIn), nIn(nInIn),
      amount(amountIn), sigHashType(sigHashTypeIn),
      checker(txTo, nIn, amountIn) {}

bool TransactionSignatureCreator::CreateSig(std::vector<uint8_t> &vchSig,
                                            const CKeyID &address,
                                            const CScript &scriptCode) const {
    CKey key;
    if (!keystore->GetKey(address, key)) {
        return false;
    }

    uint256 hash = SignatureHash(scriptCode, *txTo, nIn, sigHashType, amount);
    if (!key.Sign(hash, vchSig)) {
        return false;
    }

    vchSig.push_back(uint8_t(sigHashType.getRawSigHashType()));
    return true;
}

static bool Sign1(const CKeyID &address, const BaseSignatureCreator &creator,
                  const CScript &scriptCode, std::vector<valtype> &ret) {
    std::vector<uint8_t> vchSig;
    if (!creator.CreateSig(vchSig, address, scriptCode)) {
        return false;
    }

    ret.push_back(vchSig);
    return true;
}

static bool SignN(const std::vector<valtype> &multisigdata,
                  const BaseSignatureCreator &creator,
                  const CScript &scriptCode, std::vector<valtype> &ret) {
    int nSigned = 0;
    int nRequired = multisigdata.front()[0];
    for (size_t i = 1; i < multisigdata.size() - 1 && nSigned < nRequired;
         i++) {
        const valtype &pubkey = multisigdata[i];
        CKeyID keyID = CPubKey(pubkey).GetID();
        if (Sign1(keyID, creator, scriptCode, ret)) {
            ++nSigned;
        }
    }

    return nSigned == nRequired;
}

/**
 * Sign scriptPubKey using signature made with creator.
 * Signatures are returned in scriptSigRet (or returns false if scriptPubKey
 * can't be signed)
 * Returns false if scriptPubKey could not be completely satisfied.
 */
static bool SignStep(const BaseSignatureCreator &creator,
                     const CScript &scriptPubKey,
                     std::vector<valtype> &ret,
                     txnouttype &whichTypeRet) {
    CScript scriptRet;
    uint160 h160;
    ret.clear();

    std::vector<valtype> vSolutions;
    if (!Solver(scriptPubKey, whichTypeRet, vSolutions)) {
        return false;
    }

    CKeyID keyID;
    switch (whichTypeRet) {
        case TX_NONSTANDARD:
            return false;
        case TX_PUBKEY:
            keyID = CPubKey(vSolutions[0]).GetID();
            return Sign1(keyID, creator, scriptPubKey, ret);
        case TX_PUBKEYHASH: {
            keyID = CKeyID(uint160(vSolutions[0]));
            if (!Sign1(keyID, creator, scriptPubKey, ret)) {
                return false;
            }

            CPubKey vch;
            creator.KeyStore().GetPubKey(keyID, vch);
            ret.push_back(ToByteVector(vch));
            return true;
        }
        case TX_MULTISIG:
            // workaround CHECKMULTISIG bug
            ret.push_back(valtype());
            return (SignN(vSolutions, creator, scriptPubKey, ret));

        default:
            return false;
    }
}

static CScript PushAll(const std::vector<valtype> &values) {
    CScript result;
    for (const valtype &v : values) {
        if (v.size() == 0) {
            result << OP_0;
        } else if (v.size() == 1 && v[0] >= 1 && v[0] <= 16) {
            result << CScript::EncodeOP_N(v[0]);
        } else {
            result << v;
        }
    }

    return result;
}

bool ProduceSignature(const Config& config, bool consensus, const BaseSignatureCreator& creator,
                      const CScript& fromPubKey, SignatureData& sigdata) {
    CScript script = fromPubKey;
    bool solved = true;
    std::vector<valtype> result;
    txnouttype whichType;
    solved = SignStep(creator, script, result, whichType);
    CScript subscript;

    sigdata.scriptSig = PushAll(result);

    // no need to cancel script verification after n time
    // because wallet only produces standard transactions
    auto source = task::CCancellationSource::Make();

    // Test solution

    uint32_t flags = StandardScriptVerifyFlags();
    return solved &&
           VerifyScript(config, consensus, source->GetToken(), sigdata.scriptSig, fromPubKey,
                        flags, creator.Checker()).value();
}

SignatureData DataFromTransaction(const CMutableTransaction &tx,
                                  unsigned int nIn) {
    SignatureData data;
    assert(tx.vin.size() > nIn);
    data.scriptSig = tx.vin[nIn].scriptSig;
    return data;
}

void UpdateTransaction(CMutableTransaction &tx, unsigned int nIn,
                       const SignatureData &data) {
    assert(tx.vin.size() > nIn);
    tx.vin[nIn].scriptSig = data.scriptSig;
}

bool SignSignature(const Config& config, const CKeyStore& keystore,
                   const CScript& fromPubKey,
                   CMutableTransaction& txTo, unsigned int nIn,
                   const Amount amount, SigHashType sigHashType) {
    assert(nIn < txTo.vin.size());

    CTransaction txToConst(txTo);
    TransactionSignatureCreator creator(&keystore, &txToConst, nIn, amount,
                                        sigHashType);

    SignatureData sigdata;
    //Consensus parameter can be set to false or true here, because MULTISIG OP is a nonstandard transaction.
    //Method SignSignature handles only standard transactions
    bool ret = ProduceSignature(config, false, creator, fromPubKey, sigdata);
    UpdateTransaction(txTo, nIn, sigdata);
    return ret;
}

bool SignSignature(const Config &config, const CKeyStore &keystore,
                   const CTransaction &txFrom,
                   CMutableTransaction &txTo, unsigned int nIn,
                   SigHashType sigHashType) {
    assert(nIn < txTo.vin.size());
    CTxIn &txin = txTo.vin[nIn];
    assert(txin.prevout.GetN() < txFrom.vout.size());
    const CTxOut &txout = txFrom.vout[txin.prevout.GetN()];

    return SignSignature(config, keystore,
                         txout.scriptPubKey, txTo, nIn, txout.nValue,
                         sigHashType);
}

static std::vector<valtype> CombineMultisig(
    const CScript &scriptPubKey, const BaseSignatureChecker &checker,
    const std::vector<valtype> &vSolutions, const std::vector<valtype> &sigs1,
    const std::vector<valtype> &sigs2) {
    // Combine all the signatures we've got:
    std::set<valtype> allsigs;
    for (const valtype &v : sigs1) {
        if (!v.empty()) {
            allsigs.insert(v);
        }
    }

    for (const valtype &v : sigs2) {
        if (!v.empty()) {
            allsigs.insert(v);
        }
    }

    // Build a map of pubkey -> signature by matching sigs to pubkeys:
    assert(vSolutions.size() > 1);
    unsigned int nSigsRequired = vSolutions.front()[0];
    unsigned int nPubKeys = vSolutions.size() - 2;
    std::map<valtype, valtype> sigs;
    for (const valtype &sig : allsigs) {
        for (unsigned int i = 0; i < nPubKeys; i++) {
            const valtype &pubkey = vSolutions[i + 1];
            // Already got a sig for this pubkey
            if (sigs.count(pubkey)) {
                continue;
            }

            if (checker.CheckSig(sig, pubkey, scriptPubKey)) {
                sigs[pubkey] = sig;
                break;
            }
        }
    }
    // Now build a merged CScript:
    unsigned int nSigsHave = 0;
    // pop-one-too-many workaround
    std::vector<valtype> result;
    result.push_back(valtype());
    for (unsigned int i = 0; i < nPubKeys && nSigsHave < nSigsRequired; i++) {
        if (sigs.count(vSolutions[i + 1])) {
            result.push_back(sigs[vSolutions[i + 1]]);
            ++nSigsHave;
        }
    }

    // Fill any missing with OP_0:
    for (unsigned int i = nSigsHave; i < nSigsRequired; i++) {
        result.push_back(valtype());
    }

    return result;
}

namespace {
struct Stacks {
    std::vector<valtype> script;

    Stacks() {}
    explicit Stacks(const std::vector<valtype> &scriptSigStack_)
        : script(scriptSigStack_) {}
    explicit Stacks(const Config& config, bool consensus, const SignatureData &data) {
        LimitedStack stack(UINT32_MAX);
        auto source = task::CCancellationSource::Make();
        EvalScript(config, consensus, source->GetToken(), stack, data.scriptSig,
                   MANDATORY_SCRIPT_VERIFY_FLAGS, BaseSignatureChecker());
        stack.MoveToValtypes(script);
    }

    SignatureData Output() const {
        SignatureData result;
        result.scriptSig = PushAll(script);
        return result;
    }
};
} // namespace

static Stacks CombineSignatures(const CScript &scriptPubKey,
                                const BaseSignatureChecker &checker,
                                const txnouttype txType,
                                const std::vector<valtype> &vSolutions,
                                Stacks sigs1, Stacks sigs2) {
    switch (txType) {
        case TX_NONSTANDARD:
        case TX_NULL_DATA:
            // Don't know anything about this, assume bigger one is correct:
            if (sigs1.script.size() >= sigs2.script.size()) {
                return sigs1;
            }

            return sigs2;
        case TX_PUBKEY:
        case TX_PUBKEYHASH:
            // Signatures are bigger than placeholders or empty scripts:
            if (sigs1.script.empty() || sigs1.script[0].empty()) {
                return sigs2;
            }

            return sigs1;
        case TX_MULTISIG:
            return Stacks(CombineMultisig(scriptPubKey, checker, vSolutions,
                                          sigs1.script, sigs2.script));
        default:
            return Stacks();
    }
}

SignatureData CombineSignatures(const Config& config, bool consensus, const CScript& scriptPubKey,
                                const BaseSignatureChecker& checker,
                                const SignatureData& scriptSig1,
                                const SignatureData& scriptSig2) {
    txnouttype txType;
    std::vector<std::vector<uint8_t>> vSolutions;
    Solver(scriptPubKey, txType, vSolutions);

    return CombineSignatures(scriptPubKey, checker, txType, vSolutions,
                             Stacks(config, consensus, scriptSig1), Stacks(config, consensus, scriptSig2))
        .Output();
}

namespace {
/** Dummy signature checker which accepts all signatures. */
class DummySignatureChecker : public BaseSignatureChecker {
public:
    DummySignatureChecker() {}

    bool CheckSig(const std::vector<uint8_t> &scriptSig,
                  const std::vector<uint8_t> &vchPubKey,
                  const CScript &scriptCode) const override {
        return true;
    }
};
const DummySignatureChecker dummyChecker;
} // namespace

const BaseSignatureChecker &DummySignatureCreator::Checker() const {
    return dummyChecker;
}

bool DummySignatureCreator::CreateSig(std::vector<uint8_t> &vchSig,
                                      const CKeyID &keyid,
                                      const CScript &scriptCode) const {
    // Create a dummy signature that is a valid DER-encoding
    vchSig.assign(72, '\000');
    vchSig[0] = 0x30;
    vchSig[1] = 69;
    vchSig[2] = 0x02;
    vchSig[3] = 33;
    vchSig[4] = 0x01;
    vchSig[4 + 33] = 0x02;
    vchSig[5 + 33] = 32;
    vchSig[6 + 33] = 0x01;
    vchSig[6 + 33 + 32] = SIGHASH_ALL;
    return true;
}
