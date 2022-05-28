// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_CONSENSUS_PARAMS_H
#define BITCOIN_CONSENSUS_PARAMS_H

#include "uint256.h"

// The half life for the ASERT DAA. For every (nASERTHalfLife) seconds behind schedule the blockchain gets,
// difficulty is cut in half. Doubled if blocks are ahead of schedule.
// One hour
static const uint64_t UNSTEADY_ASERT_HALF_LIFE = 60 * 60;
// Two days.
static const uint64_t STEADY_ASERT_HALF_LIFE = 2 * 24 * 60 * 60;

namespace Consensus {
/**
 * Parameters that influence chain consensus.
 */
struct Params {
    uint256 hashGenesisBlock;
    int nSubsidyHalvingInterval;
    /** Block height and hash at which BIP34 becomes active */
    int32_t BIP34Height;
    uint256 BIP34Hash;
    /** Block height at which BIP66 becomes active */
    int32_t BIP66Height;

    int32_t SteadyASERTHeight;

    /** Proof of work parameters */
    uint256 powLimit;
    bool fPowAllowMinDifficultyBlocks;
    bool fPowNoRetargeting;
    int64_t nPowTargetSpacing;
    int64_t nUnsteadyASERTHalfLife;
    int64_t nSteadyASERTHalfLife;
    uint256 nMinimumChainWork;
    uint256 defaultAssumeValid;

    /** Used by the ASERT DAA activated */
    struct ASERTAnchor {
        int nHeight;
        uint32_t nBits;
        int64_t nPrevBlockTime;
    };

    /** For chains with a checkpoint after the ASERT anchor block, this is always defined */
    ASERTAnchor asertAnchorParams;

};
} // namespace Consensus

#endif // BITCOIN_CONSENSUS_PARAMS_H
