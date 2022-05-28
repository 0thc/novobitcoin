// Copyright (c) 2011-2015 The Bitcoin Core developers
// Copyright (c) 2018 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

//
// Unit tests for block-chain checkpoints
//

#include "checkpoints.h"

#include "chainparams.h"
#include "test/test_novobitcoin.h"
#include "uint256.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(checkpoints_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(sanity) {
    const auto params = CreateChainParams(CBaseChainParams::MAIN);
    const CCheckpointData &checkpoints = params->Checkpoints();
    uint256 p11111 = uint256S(
        "00000000e5ab5f4cc6ae918f997fe188d906690957e1f6a30c3e28c4cf4e561f");
    uint256 p55555 = uint256S(
        "00000000224682e5cb41eb91b04c3a872f11e3216ef354a79b48aa2c4e6717aa");
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 11111, p11111));
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 55555, p55555));

    // Wrong hashes at checkpoints should fail:
    BOOST_CHECK(!Checkpoints::CheckBlock(checkpoints, 11111, p55555));
    BOOST_CHECK(!Checkpoints::CheckBlock(checkpoints, 55555, p11111));

    // ... but any hash not at a checkpoint should succeed:
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 11111 + 1, p55555));
    BOOST_CHECK(Checkpoints::CheckBlock(checkpoints, 55555 + 1, p11111));
}

BOOST_AUTO_TEST_SUITE_END()
