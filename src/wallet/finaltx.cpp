// Copyright (c) 2017 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers

#include "config.h"
#include "consensus/validation.h"
#include "primitives/transaction.h"
#include "validation.h"

bool CheckFinalTx(
    const CTransaction &tx,
    int32_t nChainActiveHeight,
    int nMedianTimePast) {

    auto &config = GlobalConfig::GetConfig();
    CValidationState state;
    return ContextualCheckTransactionForCurrentBlock(
                config,
                tx,
                nChainActiveHeight,
                nMedianTimePast,
                state);
}
