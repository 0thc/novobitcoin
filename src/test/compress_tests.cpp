// Copyright (c) 2012-2015 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "compressor.h"
#include "test/test_novobitcoin.h"
#include "util.h"

#include <cstdint>

#include <boost/test/unit_test.hpp>

// amounts 0.00000001 .. 0.00100000
#define NUM_MULTIPLES_UNIT 100000

// amounts 0.01 .. 100.00
#define NUM_MULTIPLES_CENT 10000

// amounts 1 .. 10000
#define NUM_MULTIPLES_1COIN 10000

// amounts 50 .. 21000000
#define NUM_MULTIPLES_2000000COIN 420000

BOOST_FIXTURE_TEST_SUITE(compress_tests, BasicTestingSetup)

static bool TestEncode(Amount in) {
    return in == CTxOutCompressor::DecompressAmount(
                     CTxOutCompressor::CompressAmount(in));
}

static bool TestDecode(uint64_t in) {
    return in == CTxOutCompressor::CompressAmount(
                     CTxOutCompressor::DecompressAmount(in));
}

static bool TestPair(Amount dec, uint64_t enc) {
    return CTxOutCompressor::CompressAmount(dec) == enc &&
           CTxOutCompressor::DecompressAmount(enc) == dec;
}

BOOST_AUTO_TEST_CASE(compress_amounts) {
    BOOST_CHECK(TestPair(Amount(0), 0x0));
    BOOST_CHECK(TestPair(Amount(1), 0x1));
    BOOST_CHECK(TestPair(CENT, 0x3));
    BOOST_CHECK(TestPair(COIN, 0x5));
    BOOST_CHECK(TestPair(2000000 * COIN, 0xc8));
    BOOST_CHECK(TestPair(840000000000 * COIN, 0x0501bd00));

    for (int64_t i = 1; i <= NUM_MULTIPLES_UNIT; i++) {
        BOOST_CHECK(TestEncode(Amount(i)));
    }

    for (int64_t i = 1; i <= NUM_MULTIPLES_CENT; i++) {
        BOOST_CHECK(TestEncode(i * CENT));
    }

    for (int64_t i = 1; i <= NUM_MULTIPLES_1COIN; i++) {
        BOOST_CHECK(TestEncode(i * COIN));
    }

    for (int64_t i = 1; i <= NUM_MULTIPLES_2000000COIN; i++) {
        BOOST_CHECK(TestEncode(i * 2000000 * COIN));
    }

    for (int64_t i = 0; i < 100000; i++) {
        BOOST_CHECK(TestDecode(i));
    }
}

BOOST_AUTO_TEST_SUITE_END()
