// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


// NOTE: This file is intended to be customised by the end user, and includes
// only local node policy logic

#include "policy/policy.h"
#include "script/script_num.h"
#include "taskcancellation.h"
#include "validation.h"
#include "config.h"

/**
 * Check transaction inputs to mitigate two potential denial-of-service attacks:
 *
 * 1. scriptSigs with extra data stuffed into them, not consumed by scriptPubKey
 * (or P2SH script)
 * 2. P2SH scripts with a crazy number of expensive CHECKSIG/CHECKMULTISIG
 * operations
 *
 * Why bother? To avoid denial-of-service attacks; an attacker can submit a
 * standard HASH... OP_EQUAL transaction, which will get accepted into blocks.
 * The redemption script can be anything; an attacker could use a very
 * expensive-to-check-upon-redemption script like:
 *   DUP CHECKSIG DROP ... repeated 100 times... OP_1
 */
bool IsStandard(const Config &config, const CScript &scriptPubKey, txnouttype &whichType) {
    std::vector<std::vector<uint8_t>> vSolutions;
    if (!Solver(scriptPubKey, whichType, vSolutions)) {
        return false;
    }

    if (whichType == TX_MULTISIG) {
        // we don't require minimal encoding here because Solver method is already checking minimal encoding
        int m = CScriptNum(vSolutions.front(), false).getint();
        int n = CScriptNum(vSolutions.back(), false).getint();
        // Support up to x-of-3 multisig txns as standard
        if (n < 1 || n > 3) return false;
        if (m < 1 || m > n) return false;
    } else if (whichType == TX_NULL_DATA) {
        if (!fAcceptDatacarrier) {
            return false;
        }
    }

    return whichType != TX_NONSTANDARD;
}

bool IsDustReturnTxn (const CTransaction &tx)
{
    return tx.vout.size() == 1
        && tx.vout[0].nValue.GetSatoshis() == 0U
        && IsDustReturnScript(tx.vout[0].scriptPubKey);
}


// Check if a transaction is a consolidation transaction.
// A consolidation transaction is a transaction which reduces the size of the UTXO database to
// an extent that is rewarding enough for the miner to mine the transaction for free.
// However, if a consolidation transaction is donated to the miner, then we do not need to honour the consolidation factor
bool IsConsolidationTxn(const Config &config, const CTransaction &tx, const CCoinsViewCache &inputs, int32_t tipHeight)
{
    // Allow disabling free consolidation txns via configuring
    // the consolidation factor to zero
    if (config.GetMinConsolidationFactor() == 0)
        return false;

    const bool isDonation = IsDustReturnTxn(tx);

    const uint64_t factor = isDonation
            ? tx.vin.size()
            : config.GetMinConsolidationFactor();
    const int32_t minConf = isDonation
            ? int32_t(0)
            : config.GetMinConfConsolidationInput();

    const uint64_t maxSize = config.GetMaxConsolidationInputScriptSize();
    const bool stdInputOnly = !config.GetAcceptNonStdConsolidationInput();

    if (tx.IsCoinBase())
        return false;

    // The consolidation transaction needs to reduce the count of UTXOS
    if (tx.vin.size() < factor * tx.vout.size())
        return false;

    // Check all UTXOs are confirmed and prevent spam via big
    // scriptSig sizes in the consolidation transaction inputs.
    uint64_t sumScriptPubKeySizeOfTxInputs = 0;
    for (CTxIn const & u: tx.vin) {

        // accept only with many confirmations
        const auto& coin = inputs.GetCoinWithScript(u.prevout);
        assert(coin.has_value());
        const auto coinHeight = coin->GetHeight();

        if (minConf > 0 && coinHeight == MEMPOOL_HEIGHT)
            return false;

        if (minConf > 0 && coinHeight && (tipHeight + 1 - coinHeight < minConf)) // older versions did not store height
            return false;

        // spam detection
        if (u.scriptSig.size() > maxSize)
            return false;

        // if not acceptnonstdconsolidationinput then check if inputs are standard
        // and fail otherwise
        txnouttype dummyType;
        if (stdInputOnly  && !IsStandard(config, coin->GetTxOut().scriptPubKey, dummyType))
            return false;

        // sum up some script sizes
        sumScriptPubKeySizeOfTxInputs += coin->GetTxOut().scriptPubKey.size();
    }

    // check ratio between sum of tx-scriptPubKeys to sum of parent-scriptPubKeys
    uint64_t sumScriptPubKeySizeOfTxOutputs = 0;
    for (CTxOut const & o: tx.vout) {
        sumScriptPubKeySizeOfTxOutputs += o.scriptPubKey.size();
    }

    // prevent consolidation transactions that are not advantageous enough for miners
    if(sumScriptPubKeySizeOfTxInputs < factor * sumScriptPubKeySizeOfTxOutputs)
        return false;

    return true;
}

bool IsStandardTx(const Config &config, const CTransaction &tx, std::string &reason) {
    if (tx.nVersion > CTransaction::MAX_STANDARD_VERSION || tx.nVersion < 1) {
        reason = "version";
        return false;
    }

    // Extremely large transactions with lots of inputs can cost the network
    // almost as much to process as they cost the sender in fees, because
    // computing signature hashes is O(ninputs*txsize). Limiting transactions
    // mitigates CPU exhaustion attacks.
    unsigned int sz = tx.GetTotalSize();
    if (sz < MIN_TX_SIZE_CONSENSUS || sz > config.GetMaxTxSize(false)) {
        reason = "tx-size";
        return false;
    }

    for (const CTxIn &txin : tx.vin) {
        if (!txin.scriptSig.IsPushOnly()) {
            reason = "scriptsig-not-pushonly";
            return false;
        }
    }

    unsigned int nDataSize = 0;
    txnouttype whichType;
    bool scriptpubkey = false;
    for (const CTxOut &txout : tx.vout) {
        if (!::IsStandard(config, txout.scriptPubKey, whichType)) {
            scriptpubkey = true;
        }

        if (whichType == TX_NULL_DATA) {
            nDataSize += txout.scriptPubKey.size();
        } else if ((whichType == TX_MULTISIG) && (!fIsBareMultisigStd)) {
            reason = "bare-multisig";
            return false;
        } else if (txout.IsDust(dustRelayFee, config.GetDustLimitFactor())) {
            reason = "dust";
            return false;
        }
    }

    // cumulative size of all OP_RETURN txout should be smaller than -datacarriersize
    if (nDataSize > config.GetDataCarrierSize()) {
        reason = "datacarrier-size-exceeded";
        return false;
    }

    if(scriptpubkey)
    {
        reason = "scriptpubkey";
        return false;
    }

    return true;
}

std::optional<bool> AreInputsStandard(
    const task::CCancellationToken& token,
    const Config& config,
    const CTransaction& tx,
    const CCoinsViewCache &mapInputs,
    const int32_t mempoolHeight)
{
    if (tx.IsCoinBase()) {
        // Coinbases don't use vin normally.
        return true;
    }

    for (size_t i = 0; i < tx.vin.size(); i++) {
        auto prev = mapInputs.GetCoinWithScript( tx.vin[i].prevout );
        assert(prev.has_value());
        assert(!prev->IsSpent());

        std::vector<std::vector<uint8_t>> vSolutions;
        txnouttype whichType;
        // get the scriptPubKey corresponding to this input:
        const CScript &prevScript = prev->GetTxOut().scriptPubKey;

        if (!Solver(prevScript, whichType, vSolutions)) {
            return false;
        }
    }

    return true;
}

CFeeRate dustRelayFee = CFeeRate(DUST_RELAY_TX_FEE);
static_assert(DUST_RELAY_TX_FEE == DEFAULT_MIN_RELAY_TX_FEE, "lowering only fees increases dust");
