// Copyright (c) 2019 The Bitcoin developers.
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "txn_validation_data.h"
#include "logging.h"

// Enable enum_cast for TxSource, so we can log informatively
const enumTableT<TxSource>& enumTable(TxSource)
{
    static enumTableT<TxSource> table
    {
        { TxSource::unknown,      "unknown" },
        { TxSource::file,         "file" },
        { TxSource::reorg,        "reorg" },
        { TxSource::wallet,       "wallet" },
        { TxSource::rpc,          "rpc" },
        { TxSource::p2p,          "p2p" },
        { TxSource::finalised,    "finalised" }
    };
    return table;
}

// Enable enum_cast for TxValidationPriority, so we can log informatively
const enumTableT<TxValidationPriority>& enumTable(TxValidationPriority)
{
    static enumTableT<TxValidationPriority> table
    {
        { TxValidationPriority::low,      "low" },
        { TxValidationPriority::normal,   "normal" },
        { TxValidationPriority::high,     "high" }
    };
    return table;
}

/**
 * class CTxInputData
 */
// Constructor
CTxInputData::CTxInputData(
    TxIdTrackerWPtr pTxIdTracker,
    CTransactionRef ptx,
    TxSource txSource,
    TxValidationPriority txValidationPriority,
    TxStorage txStorage,
    int64_t nAcceptTime,
    Amount nAbsurdFee,
    std::weak_ptr<CNode> pNode,
    bool fOrphan)
: mpTx(ptx),
  mpNode(pNode),
  mpTxIdTracker(pTxIdTracker),
  mTxStorage(txStorage),
  mnAbsurdFee(nAbsurdFee),
  mnAcceptTime(nAcceptTime),
  mTxSource(txSource),
  mTxValidationPriority(txValidationPriority),
  mfOrphan(fOrphan)
{
    // A check on the tracker for nullness and availability
    if(mpTxIdTracker.expired()) {
        return;
    }
    // Add an entry to the TxIdTracker, if:
    // - a shared pointer to the tracker is reachable
    // - txid is not known yet
    TxIdTrackerSPtr pTracker = mpTxIdTracker.lock();
    if (pTracker && pTracker->Insert(mpTx->GetId())) {
        mfTxIdStored = true;
        LogPrint(BCLog::TXNVAL, "CTxInputData: txn= %s\n", mpTx->GetId().ToString());
    }
}

// Destructor
CTxInputData::~CTxInputData()
{
    // Remove txid from the TxIdTracker, if:
    // - it was added during construction
    // - a shared pointer to the tracker is reachable
    try {
        if (mfTxIdStored) {
            LogPrint(BCLog::TXNVAL, "~CTxInputData: txn= %s\n", mpTx->GetId().ToString());
            TxIdTrackerSPtr pTracker = mpTxIdTracker.lock();
            if (pTracker) {
                pTracker->Erase(mpTx->GetId());
                LogPrint(BCLog::TXNVAL, "~CTxInputData removed: txn= %s\n", mpTx->GetId().ToString());
            }
        }
    } catch(...) {
        LogPrint(BCLog::TXNVAL, "~CTxInputData: Unexpected exception during destruction, txn= %s\n",
            mpTx->GetId().ToString());
    }
}
