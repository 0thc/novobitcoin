// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

#include "invalid_txn_publisher.h"

namespace InvalidTxnPublisher
{
#if ENABLE_ZMQ
    class CInvalidTxnZmqSink : public CInvalidTxnSink
    {
        int64_t maxMessageSize;
    public:
        CInvalidTxnZmqSink(int64_t maxMsgSize)
            :maxMessageSize(maxMsgSize)
        {}

        void Publish(const InvalidTxnInfo& invalidTxInfo) override;
    };
#endif
}
