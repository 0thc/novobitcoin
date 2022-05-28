// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/transaction.h"
#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

uint256 ComputeTransactionOutputsHash(const std::vector<CTxOut> &vout) {
    CHashWriter ssOutputs(SER_GETHASH, 0);
    for (size_t i = 0; i < vout.size(); i++) {
        CHashWriter ssScript(SER_GETHASH, 0);
        ssScript << CFlatData(vout[i].scriptPubKey);

        CHashWriter ssOut(SER_GETHASH, 0);
        ssOut << vout[i].nValue;
        ssOut << ssScript.GetSHA256();

        CScript script = vout[i].scriptPubKey;
        auto pc = script.end();
        if (script.GetStateIterator(pc)) {
            CScript codescript(script.begin(), pc);
            CScript datascript(pc, script.end());
            CHashWriter ssCodeScript(SER_GETHASH, 0);
            CHashWriter ssDataScript(SER_GETHASH, 0);
            ssCodeScript << CFlatData(codescript);
            ssDataScript << CFlatData(datascript);

            ssOut << ssCodeScript.GetSHA256();
            ssOut << ssDataScript.GetSHA256();
        }
        ssOutputs << ssOut.GetSHA256();
    }
    return ssOutputs.GetSHA256();
}

uint256 ComputeTransactionInputsHash(const std::vector<CTxIn> &vin) {
    CHashWriter ssInputs(SER_GETHASH, 0);
    for (size_t i = 0; i < vin.size(); i++) {
          CHashWriter ssScript(SER_GETHASH, 0);
          ssScript << CFlatData(vin[i].scriptSig);
          CHashWriter ssIn(SER_GETHASH, 0);
          ssIn << vin[i].prevout;
          ssIn << ssScript.GetSHA256();
          ssIn << vin[i].nSequence;
          ssInputs << ssIn.GetSHA256();
    }
    return ssInputs.GetSHA256();
}

template <typename TxType>
CRichTransaction GetRichTransaction(const TxType &tx) {
    CRichTransaction richtx;
    richtx.nVersion = tx.nVersion;
    richtx.nInputCount = tx.vin.size();
    richtx.hashInputs = ComputeTransactionInputsHash(tx.vin);
    richtx.nOutputCount = tx.vout.size();
    richtx.hashOutputs = ComputeTransactionOutputsHash(tx.vout);
    richtx.nLockTime = tx.nLockTime;
    return richtx;
}

std::string COutPoint::ToString() const {
    return strprintf("COutPoint(%s, %u)", txid.ToString().substr(0, 10), n);
}

std::string CTxIn::ToString() const {
    std::string str;
    str += "CTxIn(";
    str += prevout.ToString();
    if (prevout.IsNull()) {
        str += strprintf(", coinbase %s", HexStr(scriptSig));
    } else {
        str += strprintf(", scriptSig=%s", HexStr(scriptSig).substr(0, 24));
    }
    if (nSequence != SEQUENCE_FINAL) {
        str += strprintf(", nSequence=%u", nSequence);
    }
    str += ")";
    return str;
}

std::string CTxOut::ToString() const {
    return strprintf("CTxOut(nValue=%d.%04d, scriptPubKey=%s)",
                     nValue.GetSatoshis() / COIN.GetSatoshis(),
                     nValue.GetSatoshis() % COIN.GetSatoshis(),
                     HexStr(scriptPubKey).substr(0, 30));
}

CMutableTransaction::CMutableTransaction()
    : nVersion(CTransaction::CURRENT_VERSION), nLockTime(0) {}
CMutableTransaction::CMutableTransaction(const CTransaction &tx)
    : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout),
      nLockTime(tx.nLockTime) {}


static uint256 ComputeCMutableTransactionHash(const CMutableTransaction &tx) {
    if (tx.nVersion == 2) {
      return SerializeHash(GetRichTransaction(tx), SER_GETHASH, 0);
    }
    return SerializeHash(tx, SER_GETHASH, 0);
}

TxId CMutableTransaction::GetId() const {
    return TxId(ComputeCMutableTransactionHash(*this));
}

TxHash CMutableTransaction::GetHash() const {
    return TxHash(ComputeCMutableTransactionHash(*this));
}

uint256 CTransaction::ComputeHash() const {
    if (this->nVersion == 2) {
      return SerializeHash(GetRichTransaction(*this), SER_GETHASH, 0);
    }
    return SerializeHash(*this, SER_GETHASH, 0);
}

/**
 * For backward compatibility, the hash is initialized to 0.
 * TODO: remove the need for this default constructor entirely.
 */
CTransaction::CTransaction()
    : nVersion(CTransaction::CURRENT_VERSION), vin(), vout(), nLockTime(0),
      hash() {}
CTransaction::CTransaction(const CMutableTransaction &tx)
    : nVersion(tx.nVersion), vin(tx.vin), vout(tx.vout),
      nLockTime(tx.nLockTime), hash(ComputeHash()) {}
CTransaction::CTransaction(CMutableTransaction &&tx)
    : nVersion(tx.nVersion), vin(std::move(tx.vin)), vout(std::move(tx.vout)),
      nLockTime(tx.nLockTime), hash(ComputeHash()) {}

Amount CTransaction::GetValueOut() const {
    Amount nValueOut(0);
    for (std::vector<CTxOut>::const_iterator it(vout.begin()); it != vout.end();
         ++it) {
        nValueOut += it->nValue;
        if (!MoneyRange(it->nValue) || !MoneyRange(nValueOut))
            throw std::runtime_error(std::string(__func__) +
                                     ": value out of range");
    }
    return nValueOut;
}

unsigned int CTransaction::GetTotalSize() const {
    return ::GetSerializeSize(*this, SER_NETWORK, PROTOCOL_VERSION);
}

bool CTransaction::HasP2SHOutput() const {
     return std::any_of(vout.begin(), vout.end(),
            [](const CTxOut& o){
                return IsP2SH(o.scriptPubKey);
            }
        );
}

std::string CTransaction::ToString() const {
    std::string str;
    str += strprintf("CTransaction(txid=%s, ver=%d, vin.size=%u, vout.size=%u, "
                     "nLockTime=%u)\n",
                     GetId().ToString().substr(0, 10), nVersion, vin.size(),
                     vout.size(), nLockTime);
    for (unsigned int i = 0; i < vin.size(); i++)
        str += "    " + vin[i].ToString() + "\n";
    for (unsigned int i = 0; i < vout.size(); i++)
        str += "    " + vout[i].ToString() + "\n";
    return str;
}
