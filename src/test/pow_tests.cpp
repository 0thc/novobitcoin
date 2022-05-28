// Copyright (c) 2015 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chain.h"
#include "chainparams.h"
#include "config.h"
#include "pow.h"
#include "random.h"
#include "test/test_novobitcoin.h"
#include "util.h"
#include "block_index_store.h"

#include <boost/test/unit_test.hpp>

#include <map>

namespace{ class pow_tests_uid; } // only used as unique identifier

template <>
struct CBlockIndex::UnitTestAccess<pow_tests_uid>
{
    UnitTestAccess() = delete;

    static void AddTime( CBlockIndex* index, int64_t time)
    {
        index->nTime += time;
    }

    static void SetHeight( CBlockIndex& index, int32_t height)
    {
        index.nHeight = height;
    }
};
using TestAccessCBlockIndex = CBlockIndex::UnitTestAccess<pow_tests_uid>;

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)


BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test) {
    DummyConfig config(CBaseChainParams::MAIN);
    BlockIndexStore blockIndexStore;
    CChain blocks;

    uint256 prev;

    for (int i = 0; i < 10000; i++) {
        CBlockHeader header;
        header.nTime =
            1269211443 +
            i * config.GetChainParams().GetConsensus().nPowTargetSpacing;
        header.nBits = 0x207fffff; /* target 0x7fffff000... */
        header.hashPrevBlock = prev;

        blocks.SetTip( blockIndexStore.Insert( header ) );

        prev = blocks.Tip()->GetBlockHash();
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = blocks[InsecureRandRange(10000)];

        int64_t tdiff = GetBlockProofEquivalentTime(
            *p1, *p2, *p3, config.GetChainParams().GetConsensus());
        BOOST_CHECK_EQUAL(tdiff, p1->GetBlockTime() - p2->GetBlockTime());
    }
}

static CBlockIndex* GetBlockIndex(
    CBlockIndex* pindexPrev,
    int64_t nTimeInterval,
    uint32_t nBits,
    BlockIndexStore& blockIndexStore)
{
    CBlockHeader header;
    header.nTime = pindexPrev->GetBlockTime() + nTimeInterval;
    header.nBits = nBits;
    header.nNonce = blockIndexStore.Count();
    header.hashPrevBlock = pindexPrev->GetBlockHash();

    return blockIndexStore.Insert( header );
}


double TargetFromBits(const uint32_t nBits) {
    return (nBits & 0xff'ff'ff) * pow(256, (nBits >> 24)-3);
}

double GetASERTApproximationError(const CBlockIndex *pindexPrev,
                                  const uint32_t finalBits,
                                  const CBlockIndex *pindexAnchorBlock) {
    const int64_t nHeightDiff = pindexPrev->GetHeight() - pindexAnchorBlock->GetHeight();
    const int64_t nTimeDiff   = pindexPrev->GetBlockTime()   - pindexAnchorBlock->GetPrev()->GetBlockTime();
    const uint32_t initialBits = pindexAnchorBlock->GetBits();

    BOOST_CHECK(nHeightDiff >= 0);
    double dInitialPow = TargetFromBits(initialBits);
    double dFinalPow   = TargetFromBits(finalBits);

    // params.nPowTargetSpacing == 150
    double dExponent = double(nTimeDiff - (nHeightDiff+1) * 150) / double(3600);
    double dTarget = dInitialPow * pow(2, dExponent);

    LogPrintf("GetASERTApproximationError %f.\n", (dFinalPow - dTarget) / dTarget);
    return (dFinalPow - dTarget) / dTarget;
}

BOOST_AUTO_TEST_CASE(asert_difficulty_test) {
    DummyConfig config(CBaseChainParams::MAIN);
    BlockIndexStore blockIndexStore;
    CChain blocks;

    // std::vector<CBlockIndex*> blocks(3000 + 2*24*3600);

    Consensus::Params mutableParams = config.GetChainParams().GetConsensus(); // copy params
    const Consensus::Params &params = mutableParams; // take a const reference
    const arith_uint256 powLimit = UintToArith256(params.powLimit);
    arith_uint256 currentPow = powLimit >> 3;
    uint32_t initialBits = currentPow.GetCompact();
    double dMaxErr = 0.008;

    // Genesis block, and parent of ASERT anchor block in this test case.
    {
        CBlockHeader header;
        header.nTime = 1269211443;
        // The pre-anchor block's nBits should never be used, so we set it to a nonsense value in order to
        // trigger an error if it is ever accessed
        header.nBits = 0x0dedbeef;
        blocks.SetTip(blockIndexStore.Insert(header));
    }

    // ASERT anchor block. We give this one a solvetime of 150/4 seconds to ensure that
    // the solvetime between the pre-anchor and the anchor blocks is actually used.
    CBlockIndex* anchorBlock = GetBlockIndex(blocks.Tip(), 150/4, initialBits, blockIndexStore);
    blocks.SetTip(anchorBlock);

    mutableParams.asertAnchorParams = Consensus::Params::ASERTAnchor{
        1,            // anchor block height
        initialBits,   // anchor block nBits
        1269211443+150/4,   // anchor block previous block timestamp
    };

    // The nBits for the next block should not be equal to the anchor block's nBits
    CBlockHeader blkHeaderDummy;
    uint32_t nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr);
    BOOST_CHECK(nBits != initialBits);

    // If we add another block at 1050 seconds, we should return to the anchor block's nBits
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing*2, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(nBits == initialBits);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr);

    currentPow = arith_uint256().SetCompact(nBits);
    // Before we do anything else, check that timestamps *before* the anchor block work fine.
    // Jumping 1 hour into the past will give a timestamp before the achnor, and should halve the target
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing-3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    currentPow = arith_uint256().SetCompact(nBits);
    // Because nBits truncates target, we don't end up with exactly 1/2 the target
    BOOST_CHECK(currentPow <= arith_uint256().SetCompact(initialBits  ) / 2);
    BOOST_CHECK(currentPow >= arith_uint256().SetCompact(initialBits-1) / 2);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr);

    // Jumping forward 1 hour should return the target to the initial value
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing+3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    currentPow = arith_uint256().SetCompact(nBits);
    BOOST_CHECK(nBits == initialBits);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr);

    // Pile up some blocks every 2.5 mins to establish some history.
    for (size_t j = 0; j < 150; j++) {
        blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing, nBits, blockIndexStore));
        BOOST_CHECK_EQUAL(blocks.Tip()->GetBits(), nBits);
    }

    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);

    BOOST_CHECK_EQUAL(nBits, initialBits);

    // Difficulty stays the same as long as we produce a block every 10 mins.
    for (size_t j = 0; j < 10; j++) {
        blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing, nBits, blockIndexStore));
        BOOST_CHECK_EQUAL(
            GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params),
            nBits);
    }

    // If we add a two blocks whose solvetimes together add up to 1200s,
    // then the next block's target should be the same as the one before these blocks
    // (at this point, equal to initialBits).
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing/2, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing+params.nPowTargetSpacing/2, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    BOOST_CHECK_EQUAL(nBits, initialBits);
    BOOST_CHECK(nBits != blocks.Tip()->GetBits());

    // Same in reverse - this time slower block first, followed by faster block.
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing+params.nPowTargetSpacing/2, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing/2, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    BOOST_CHECK_EQUAL(nBits, initialBits);
    BOOST_CHECK(nBits != blocks.Tip()->GetBits());

    // Jumping forward 1 hour should double the target (halve the difficulty)
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing + 3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    currentPow = arith_uint256().SetCompact(nBits) / 2;
    BOOST_CHECK_EQUAL(currentPow.GetCompact(), initialBits);

    // Jumping backward 1 hour should bring target back to where we started
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing - 3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    BOOST_CHECK_EQUAL(nBits, initialBits);

    // Jumping backward 1 hour should halve the target (double the difficulty)
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing - 3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    currentPow = arith_uint256().SetCompact(nBits);
    // Because nBits truncates target, we don't end up with exactly 1/2 the target
    BOOST_CHECK(currentPow <= arith_uint256().SetCompact(initialBits  ) / 2);
    BOOST_CHECK(currentPow >= arith_uint256().SetCompact(initialBits-1) / 2);

    // And forward again
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing + 3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    BOOST_CHECK_EQUAL(nBits, initialBits);
    blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing + 3600, nBits, blockIndexStore));
    nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock)) < dMaxErr); // absolute
    BOOST_CHECK(fabs(GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev())) < dMaxErr); // relative
    currentPow = arith_uint256().SetCompact(nBits) / 2;
    BOOST_CHECK_EQUAL(currentPow.GetCompact(), initialBits);

    // Iterate over the entire -2*24*3600..+2*24*3600 range to check that our integer approximation:
    //   1. Should be monotonic
    //   2. Should change target at least once every 8 seconds (worst-case: 15-bit precision on nBits)
    //   3. Should never change target by more than XXXX per 1-second step
    //   4. Never exceeds dMaxError in absolute error vs a double float calculation
    //   5. Has almost exactly the dMax and dMin errors we expect for the formula
    double dMin = 0;
    double dMax = 0;
    double dErr;
    double dRelMin = 0;
    double dRelMax = 0;
    double dRelErr;
    double dMaxStep = 0;
    uint32_t nBitsRingBuffer[8];
    double dStep = 0;
    blocks.SetTip(GetBlockIndex(blocks.Tip(), -3600 - 30, nBits, blockIndexStore));
    for (size_t j = 0; j < 2*3600 + 660; j++) {
        TestAccessCBlockIndex::AddTime( blocks.Tip(), 1);
        nBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);

        if (j > 8) {
            // 1: Monotonic
            BOOST_CHECK(arith_uint256().SetCompact(nBits) >= arith_uint256().SetCompact(nBitsRingBuffer[(j-1)%8]));
            // 2: Changes at least once every 8 seconds (worst case: nBits = 1d008000 to 1d008001)
            BOOST_CHECK(arith_uint256().SetCompact(nBits) > arith_uint256().SetCompact(nBitsRingBuffer[j%8]));
            // 3: Check 1-sec step size
            dStep = (TargetFromBits(nBits) - TargetFromBits(nBitsRingBuffer[(j-1)%8])) / TargetFromBits(nBits);
            if (dStep > dMaxStep) dMaxStep = dStep;
            BOOST_CHECK(dStep < 0.0072); // from nBits = 1d008000 to 1d008001
        }
        nBitsRingBuffer[j%8] = nBits;

        // 4 and 5: check error vs double precision float calculation
        dErr    = GetASERTApproximationError(blocks.Tip(), nBits, anchorBlock);
        dRelErr = GetASERTApproximationError(blocks.Tip(), nBits, blocks.Tip()->GetPrev());
        if (dErr    < dMin)    dMin    = dErr;
        if (dErr    > dMax)    dMax    = dErr;
        if (dRelErr < dRelMin) dRelMin = dRelErr;
        if (dRelErr > dRelMax) dRelMax = dRelErr;
        BOOST_CHECK_MESSAGE(fabs(dErr) < dMaxErr,
                            strprintf("solveTime: %d\tStep size: %.8f%%\tdErr: %.8f%%\tnBits: %0x\n",
                                      int64_t(blocks.Tip()->GetBlockTime()) - blocks.Tip()->GetPrev()->GetBlockTime(), dStep*100, dErr*100, nBits));
        BOOST_CHECK_MESSAGE(fabs(dRelErr) < dMaxErr,
                            strprintf("solveTime: %d\tStep size: %.8f%%\tdRelErr: %.8f%%\tnBits: %0x\n",
                                      int64_t(blocks.Tip()->GetBlockTime()) - blocks.Tip()->GetPrev()->GetBlockTime(), dStep*100, dRelErr*100, nBits));
    }
    auto failMsg = strprintf("Min error: %16.14f%%\tMax error: %16.14f%%\tMax step: %16.14f%%\n", dMin*100, dMax*100, dMaxStep*100);
    BOOST_CHECK_MESSAGE(   dMin < -0.00719889603010
                        && dMin > -0.00719889603011
                        && dMax > -0.00000000000001
                        && dMax <  0.00000000000001,
                        failMsg);
    failMsg = strprintf("Min relError: %16.14f%%\tMax relError: %16.14f%%\n", dRelMin*100, dRelMax*100);
    BOOST_CHECK_MESSAGE(   dRelMin < -0.00010091533485
                        && dRelMin > -0.00010091533486
                        && dRelMax >  0.00011652359561
                        && dRelMax <  0.00011652359562,
                        failMsg);




    // Difficulty increases as long as we produce fast blocks
    for (size_t j = 0; j < 100; j++) {
        uint32_t nextBits;
        arith_uint256 currentTarget;
        currentTarget.SetCompact(nBits);

        blocks.SetTip(GetBlockIndex(blocks.Tip(), params.nPowTargetSpacing-30, nBits, blockIndexStore));
        nextBits = GetNextASERTWorkRequired(blocks.Tip(), &blkHeaderDummy, params);
        arith_uint256 nextTarget;
        nextTarget.SetCompact(nextBits);

        // Make sure that target is decreased
        BOOST_CHECK(nextTarget <= currentTarget);

        nBits = nextBits;
    }

}

std::string StrPrintCalcArgs(const arith_uint256 refTarget,
                             const int64_t targetSpacing,
                             const int64_t timeDiff,
                             const int64_t heightDiff,
                             const arith_uint256 expectedTarget,
                             const uint32_t expectednBits) {
    return strprintf("\n"
                     "ref=         %s\n"
                     "spacing=     %d\n"
                     "timeDiff=    %d\n"
                     "heightDiff=  %d\n"
                     "expTarget=   %s\n"
                     "exp nBits=   0x%08x\n",
                     refTarget.ToString(),
                     targetSpacing,
                     timeDiff,
                     heightDiff,
                     expectedTarget.ToString(),
                     expectednBits);
}


// Tests of the CalculateASERT function.
BOOST_AUTO_TEST_CASE(calculate_asert_test) {
    DummyConfig config(CBaseChainParams::MAIN);
    const Consensus::Params &params = config.GetChainParams().GetConsensus();
    const int64_t nHalfLife = params.nUnsteadyASERTHalfLife;

    const arith_uint256 powLimit = UintToArith256(params.powLimit);
    arith_uint256 initialTarget = powLimit >> 4;
    int64_t height = 0;

    // The CalculateASERT function uses the absolute ASERT formulation
    // and adds +1 to the height difference that it receives.
    // The time difference passed to it must factor in the difference
    // to the *parent* of the reference block.
    // We assume the parent is ideally spaced in time before the reference block.
    static const int64_t parent_time_diff = 150;

    // Steady
    arith_uint256 nextTarget = CalculateASERT(initialTarget, params.nPowTargetSpacing,
                                              parent_time_diff + params.nPowTargetSpacing /* nTimeDiff */, ++height, powLimit, nHalfLife);
    BOOST_CHECK(nextTarget == initialTarget);

    // A block that arrives in half the expected time
    nextTarget = CalculateASERT(initialTarget, params.nPowTargetSpacing,
                                parent_time_diff + params.nPowTargetSpacing + params.nPowTargetSpacing/2, ++height, powLimit, nHalfLife);
    BOOST_CHECK(nextTarget < initialTarget);

    // A block that makes up for the shortfall of the previous one, restores the target to initial
    arith_uint256 prevTarget = nextTarget;
    nextTarget = CalculateASERT(initialTarget, params.nPowTargetSpacing,
                                parent_time_diff + params.nPowTargetSpacing + params.nPowTargetSpacing + params.nPowTargetSpacing, ++height, powLimit, nHalfLife);
    BOOST_CHECK(nextTarget > prevTarget);
    BOOST_CHECK(nextTarget == initialTarget);

    // 1 hour ahead of schedule should double the target (halve the difficulty)
    prevTarget = nextTarget;
    nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing,
                                parent_time_diff + 24*150*2, 24, powLimit, nHalfLife);
    BOOST_CHECK(nextTarget == prevTarget * 2);

    // 1 hour behind schedule should halve the target (double the difficulty)
    prevTarget = nextTarget;
    nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing,
                                parent_time_diff + 24*0, 24, powLimit, nHalfLife);
    BOOST_CHECK(nextTarget == prevTarget / 2);
    BOOST_CHECK(nextTarget == initialTarget);

    // Ramp up from initialTarget to PowLimit - should only take 4 doublings...
    uint32_t powLimit_nBits = powLimit.GetCompact();
    uint32_t next_nBits;
    for (size_t k = 0; k < 3; k++) {
        prevTarget = nextTarget;
        nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing,
                                    parent_time_diff + 24*150*2, 24, powLimit, nHalfLife);
        BOOST_CHECK(nextTarget == prevTarget * 2);
        BOOST_CHECK(nextTarget < powLimit);
        next_nBits = nextTarget.GetCompact();
        BOOST_CHECK(next_nBits != powLimit_nBits);
    }

    prevTarget = nextTarget;
    nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing,
                                parent_time_diff + 24*150*2, 24, powLimit, nHalfLife);
    next_nBits = nextTarget.GetCompact();
    BOOST_CHECK(nextTarget == prevTarget * 2);
    BOOST_CHECK(next_nBits == powLimit_nBits);

    // Fast periods now cannot increase target beyond POW limit, even if we try to overflow nextTarget.
    // prevTarget is a uint256, so 256*2 = 512 days would overflow nextTarget unless CalculateASERT
    // correctly detects this error
    nextTarget = CalculateASERT(prevTarget, params.nPowTargetSpacing,
                                parent_time_diff + 512*576*150, 0, powLimit, nHalfLife);
    next_nBits = nextTarget.GetCompact();
    BOOST_CHECK(next_nBits == powLimit_nBits);

    // We also need to watch for underflows on nextTarget. We need to withstand an extra ~446 days worth of blocks.
    // This should bring down a powLimit target to the a minimum target of 1.
    nextTarget = CalculateASERT(powLimit, params.nPowTargetSpacing, 0, (256-33)*24, powLimit, nHalfLife);
    next_nBits = nextTarget.GetCompact();
    BOOST_CHECK_EQUAL(next_nBits, arith_uint256(1).GetCompact());

    // Define a structure holding parameters to pass to CalculateASERT.
    // We are going to check some expected results  against a vector of
    // possible arguments.
    struct calc_params {
        arith_uint256 refTarget;
        int64_t targetSpacing;
        int64_t timeDiff;
        int64_t heightDiff;
        arith_uint256 expectedTarget;
        uint32_t expectednBits;
    };

    // Define some named input argument values
    const arith_uint256 SINGLE_75_TARGET { "00000000fc56ffffffffffffffffffffffffffffffffffffffffffffffffffff" };
    const arith_uint256 FUNNY_REF_TARGET { "000000008000000000000000000fffffffffffffffffffffffffffffffffffff" };

    // Define our expected input and output values.
    // The timeDiff entries exclude the `parent_time_diff` - this is
    // added in the call to CalculateASERT in the test loop.
    const std::vector<calc_params> calculate_args = {

        /* refTarget, targetSpacing, timeDiff, heightDiff, expectedTarget, expectednBits */

        { powLimit, 150, 0, 24, powLimit >> 1, 0x1c7fffff },
        { powLimit, 150, 0, 2*24, powLimit >> 2, 0x1c3fffff },
        { powLimit >> 1, 150, 0, 24, powLimit >> 2, 0x1c3fffff },
        { powLimit >> 2, 150, 0, 24, powLimit >> 3, 0x1c1fffff },
        { powLimit >> 3, 150, 0, 24, powLimit >> 4, 0x1c0fffff },
        { powLimit, 150, 0, (256-34)*24, 3, 0x01030000 },
        { powLimit, 150, 0, (256-34)*24 + 9, 3, 0x01030000 },
        { powLimit, 150, 0, (256-34)*24 + 10, 2, 0x01020000 },
        { powLimit, 150, 0, (256-33)*24-1, 2, 0x01020000 },
        { powLimit, 150, 0, (256-33)*24, 1, 0x01010000 },  // 1 bit less since we do not need to shift to 0
        { powLimit, 150, 0, (256-32)*24, 1, 0x01010000 },  // more will not decrease below 1
        { 1, 150, 0, (256-32)*24, 1, 0x01010000 },
        { powLimit, 150, (512-32)*24, 0, powLimit, powLimit_nBits },
        { 1, 150, (256-32)*24*600, 0, powLimit, powLimit_nBits },
        { powLimit, 150, 75, 1, SINGLE_75_TARGET, 0x1d00fc56 },  // clamps to powLimit
        { FUNNY_REF_TARGET, 150, 150*33*24, 0, powLimit, powLimit_nBits }, // confuses any attempt to detect overflow by inspecting result
        { 1, 150, 150*256*24, 0, powLimit, powLimit_nBits }, // overflow to exactly 2^256
        { 1, 150, 150*224*24 - 1, 0, arith_uint256(0xfff3) << 208, 0x1d00fff3 }, // just under powlimit (not clamped) yet over powlimit_nbits
    };

    for (auto& v : calculate_args) {
        nextTarget = CalculateASERT(v.refTarget, v.targetSpacing, parent_time_diff + v.timeDiff, v.heightDiff, powLimit, nHalfLife);
        next_nBits = nextTarget.GetCompact();
        const auto failMsg =
            StrPrintCalcArgs(v.refTarget, v.targetSpacing, parent_time_diff + v.timeDiff, v.heightDiff, v.expectedTarget, v.expectednBits)
            + strprintf("nextTarget=  %s\nnext nBits=  0x%08x\n", nextTarget.ToString(), next_nBits);
        BOOST_CHECK_MESSAGE(nextTarget == v.expectedTarget && next_nBits == v.expectednBits, failMsg);
    }
}

BOOST_AUTO_TEST_SUITE_END()
