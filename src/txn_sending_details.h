// Copyright (c) 2018-2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

#include "protocol.h"
#include "txmempool.h"

/**
* Details of a transaction for sending out over P2P.
*/
class CTxnSendingDetails
{
  public:
    CTxnSendingDetails(const CInv& inv, const TxMempoolInfo& info)
        : mInv{inv}, mTxInfo{info} {}

    CTxnSendingDetails(const CInv& inv, const CTransactionRef& forcedRef)
        : mInv{inv}, mForcedTx{forcedRef} {}

    const CInv& getInv() const { return mInv; }
    const TxMempoolInfo& getInfo() const { return mTxInfo; }

    bool isForcedRelay() const { return static_cast<bool>(mForcedTx); }
    const CTransactionRef& getTxnRef() const
    {
        if(mForcedTx)
            return mForcedTx;
        else
            return mTxInfo.GetTx();
    }

  private:

    CInv mInv {};
    TxMempoolInfo mTxInfo {};
    CTransactionRef mForcedTx {};
};
