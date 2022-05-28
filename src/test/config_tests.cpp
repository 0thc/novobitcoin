// Copyright (c) 2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "chainparams.h"
#include "config.h"
#include "consensus/consensus.h"
#include "test/test_novobitcoin.h"

#include <boost/test/unit_test.hpp>
#include <string>

BOOST_FIXTURE_TEST_SUITE(config_tests, BasicTestingSetup)

static bool isSetDefaultBlockSizeParamsCalledException(const std::runtime_error &ex) {
    static std::string expectedException("GlobalConfig::SetDefaultBlockSizeParams must be called before accessing block size related parameters");
    return expectedException == ex.what();
}

BOOST_AUTO_TEST_CASE(max_block_size) {
    GlobalConfig config;

    // SetDefaultBlockSizeParams must be called before using config block size parameters
    // otherwise getters rise exceptions
    BOOST_CHECK_EXCEPTION(config.GetMaxBlockSize(), std::runtime_error, isSetDefaultBlockSizeParamsCalledException);
    BOOST_CHECK_EXCEPTION(config.GetMaxGeneratedBlockSize(), std::runtime_error, isSetDefaultBlockSizeParamsCalledException);

    config.SetDefaultBlockSizeParams(Params().GetDefaultBlockSizeParams());

    // Too small.
    std::string err = "";
    BOOST_CHECK(!config.SetMaxBlockSize(1, &err));
    BOOST_CHECK(!err.empty());
    err = "";
    BOOST_CHECK(!config.SetMaxBlockSize(12345, &err));
    BOOST_CHECK(!err.empty());
    BOOST_CHECK(!config.SetMaxBlockSize(LEGACY_MAX_BLOCK_SIZE - 1));
    BOOST_CHECK(!config.SetMaxBlockSize(LEGACY_MAX_BLOCK_SIZE));


    // LEGACY_MAX_BLOCK_SIZE + 1
    err = "";
    BOOST_CHECK(config.SetMaxBlockSize(LEGACY_MAX_BLOCK_SIZE + 1, &err));
    BOOST_CHECK(err.empty());
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), LEGACY_MAX_BLOCK_SIZE + 1);

    // 2MB
    BOOST_CHECK(config.SetMaxBlockSize(2 * ONE_MEGABYTE));
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), 2 * ONE_MEGABYTE);

    // 8MB
    BOOST_CHECK(config.SetMaxBlockSize(8 * ONE_MEGABYTE));
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), 8 * ONE_MEGABYTE);

    // Invalid size keep config.
    BOOST_CHECK(!config.SetMaxBlockSize(54321));
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), 8 * ONE_MEGABYTE);

    // Setting it back down
    BOOST_CHECK(config.SetMaxBlockSize(7 * ONE_MEGABYTE));
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), 7 * ONE_MEGABYTE);
    BOOST_CHECK(config.SetMaxBlockSize(ONE_MEGABYTE + 1));
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), ONE_MEGABYTE + 1);
}

BOOST_AUTO_TEST_CASE(max_block_size_related_defaults) {

    GlobalConfig config;

    // Make up some dummy parameters taking into account the following rules
    // - Block size should be at least 1000
    // - generated block size can not be larger than received block size - 1000
    DefaultBlockSizeParams defaultParams {
        // max block size
        6000,
        // max generated block size
        4000
    };

    config.SetDefaultBlockSizeParams(defaultParams);

    // Functions that do not take time parameter should return future data
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), defaultParams.maxBlockSize);
    BOOST_CHECK_EQUAL(config.GetMaxGeneratedBlockSize(), defaultParams.maxGeneratedBlockSize);

    // Override one of the values, the overriden value should be used regardless of time.
    // Minimum allowed received block size is 1 MB, so we use 8 MB
    uint64_t overridenMaxBlockSize { 8 * ONE_MEGABYTE };

    BOOST_CHECK(config.SetMaxBlockSize(overridenMaxBlockSize));
    BOOST_CHECK_EQUAL(config.GetMaxBlockSize(), overridenMaxBlockSize);
    BOOST_CHECK_EQUAL(config.GetMaxGeneratedBlockSize(), defaultParams.maxGeneratedBlockSize);

    // Override the generated block size, which must be smaller than received block size
    uint64_t overridenMagGeneratedBlockSize = overridenMaxBlockSize - ONE_MEGABYTE;

    BOOST_CHECK(config.SetMaxGeneratedBlockSize(overridenMagGeneratedBlockSize));
    BOOST_CHECK_EQUAL(config.GetMaxGeneratedBlockSize(), overridenMagGeneratedBlockSize);
}

BOOST_AUTO_TEST_CASE(max_tx_size) {

    GlobalConfig config;
    std::string reason;
    int64_t newMaxTxSizePolicy{ DEFAULT_MAX_TX_SIZE_POLICY + 1 };


    // default policy tx size
    BOOST_CHECK(config.GetMaxTxSize(false) == DEFAULT_MAX_TX_SIZE_POLICY);

    // default consensus tx size
    BOOST_CHECK(config.GetMaxTxSize(true) == MAX_TX_SIZE_CONSENSUS);


    // can not set policy tx size < policy tx size
    BOOST_CHECK(!config.SetMaxTxSizePolicy(DEFAULT_MAX_TX_SIZE_POLICY - 1, &reason)); // fixme

    // can not set policy tx size > consensus tx size
    BOOST_CHECK(!config.SetMaxTxSizePolicy(MAX_TX_SIZE_CONSENSUS + 1, &reason));

    // can not set policy tx size < 0
    BOOST_CHECK(!config.SetMaxTxSizePolicy(- 1, &reason));


    // set new max policy tx size
    BOOST_CHECK(config.SetMaxTxSizePolicy(newMaxTxSizePolicy, &reason));

    // policy tx size
    BOOST_CHECK(config.GetMaxTxSize(false) == static_cast<uint64_t>(newMaxTxSizePolicy));


    // set unlimited policy tx size
    BOOST_CHECK(config.SetMaxTxSizePolicy(0, &reason));

    // policy tx size
    BOOST_CHECK(config.GetMaxTxSize(false) == MAX_TX_SIZE_CONSENSUS);
}

BOOST_AUTO_TEST_CASE(max_bignum_length_policy) {

    GlobalConfig config;
    std::string reason;
    int64_t newMaxScriptNumLengthPolicy{ MIN_SCRIPT_NUM_LENGTH + 1 };

    // default policy max length
    BOOST_CHECK(config.GetMaxScriptNumLength(false) == DEFAULT_SCRIPT_NUM_LENGTH_POLICY);

    // default consensus max length
    BOOST_CHECK(config.GetMaxScriptNumLength(true) == MAX_SCRIPT_NUM_LENGTH);

    // can not set script number length policy > consensus script number length
    BOOST_CHECK(!config.SetMaxScriptNumLengthPolicy(MAX_SCRIPT_NUM_LENGTH + 1, &reason));

    // can not set policy script number length < 0
    BOOST_CHECK(!config.SetMaxScriptNumLengthPolicy(-1, &reason));

    // set new max policy script number length
    BOOST_CHECK(config.SetMaxScriptNumLengthPolicy(newMaxScriptNumLengthPolicy, &reason));

    // policy script number length
    BOOST_CHECK(config.GetMaxScriptNumLength(false) == static_cast<uint64_t>(newMaxScriptNumLengthPolicy));

    // set unlimited policy script number length
    BOOST_CHECK(config.SetMaxScriptNumLengthPolicy(0, &reason));

    // policy script number length
    BOOST_CHECK(config.GetMaxScriptNumLength(false) == MAX_SCRIPT_NUM_LENGTH);
}


BOOST_AUTO_TEST_CASE(hex_to_array) {
    const std::string hexstr = "0a0b0C0D";//Lower and Upper char should both work
    CMessageHeader::MessageMagic array;
    BOOST_CHECK(HexToArray(hexstr, array));
    BOOST_CHECK_EQUAL(array[0],10);
    BOOST_CHECK_EQUAL(array[1],11);
    BOOST_CHECK_EQUAL(array[2],12);
    BOOST_CHECK_EQUAL(array[3],13);
}

BOOST_AUTO_TEST_CASE(chain_params) {
    GlobalConfig config;

    // Global config is consistent with params.
    SelectParams(CBaseChainParams::MAIN);
    BOOST_CHECK_EQUAL(&Params(), &config.GetChainParams());

    SelectParams(CBaseChainParams::TESTNET);
    BOOST_CHECK_EQUAL(&Params(), &config.GetChainParams());

    SelectParams(CBaseChainParams::REGTEST);
    BOOST_CHECK_EQUAL(&Params(), &config.GetChainParams());
}

BOOST_AUTO_TEST_CASE(max_stack_size) {

    std::string reason;

    BOOST_CHECK(testConfig.SetMaxStackMemoryUsage(0, 0));
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(true), INT64_MAX);
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(false), INT64_MAX);

    BOOST_CHECK(testConfig.SetMaxStackMemoryUsage(0, DEFAULT_STACK_MEMORY_USAGE_POLICY));
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(true), INT64_MAX);
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(false), DEFAULT_STACK_MEMORY_USAGE_POLICY);

    BOOST_CHECK(!testConfig.SetMaxStackMemoryUsage(1000000, 0, &reason));

    BOOST_CHECK(testConfig.SetMaxStackMemoryUsage(200000000, DEFAULT_STACK_MEMORY_USAGE_POLICY));
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(true), 200000000);
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(false), DEFAULT_STACK_MEMORY_USAGE_POLICY);

    BOOST_CHECK(!testConfig.SetMaxStackMemoryUsage(500, 600, &reason));

    BOOST_CHECK(testConfig.SetMaxStackMemoryUsage(600, 500));
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(true), 600);
    BOOST_CHECK_EQUAL(testConfig.GetMaxStackMemoryUsage(false), 500);

    BOOST_CHECK(!testConfig.SetMaxStackMemoryUsage(-1, -2));
}

BOOST_AUTO_TEST_CASE(max_send_queues_size) {

    std::string reason;

    uint64_t testBlockSize = LEGACY_MAX_BLOCK_SIZE + 1;
    gArgs.ForceSetArg("-excessiveblocksize", to_string(testBlockSize));
    BOOST_CHECK(testConfig.SetMaxBlockSize(testBlockSize, &reason));
    BOOST_CHECK_EQUAL(testConfig.GetMaxSendQueuesBytes(), testBlockSize * DEFAULT_FACTOR_MAX_SEND_QUEUES_BYTES);

    uint64_t testFactor = 3;
    testConfig.SetFactorMaxSendQueuesBytes(testFactor);
    BOOST_CHECK_EQUAL(testConfig.GetMaxSendQueuesBytes(), testBlockSize * testFactor);
}

BOOST_AUTO_TEST_CASE(block_download_config)
{
    GlobalConfig config {};
    std::string err {};

    BOOST_CHECK_EQUAL(config.GetBlockStallingMinDownloadSpeed(), DEFAULT_MIN_BLOCK_STALLING_RATE);
    BOOST_CHECK(config.SetBlockStallingMinDownloadSpeed(2 * DEFAULT_MIN_BLOCK_STALLING_RATE, &err));
    BOOST_CHECK_EQUAL(config.GetBlockStallingMinDownloadSpeed(), 2 * DEFAULT_MIN_BLOCK_STALLING_RATE);
    BOOST_CHECK(config.SetBlockStallingMinDownloadSpeed(0, &err));
    BOOST_CHECK(!config.SetBlockStallingMinDownloadSpeed(-1, &err));

    BOOST_CHECK_EQUAL(config.GetBlockStallingTimeout(), DEFAULT_BLOCK_STALLING_TIMEOUT);
    BOOST_CHECK(config.SetBlockStallingTimeout(2 * DEFAULT_BLOCK_STALLING_TIMEOUT, &err));
    BOOST_CHECK_EQUAL(config.GetBlockStallingTimeout(), 2 * DEFAULT_BLOCK_STALLING_TIMEOUT);
    BOOST_CHECK(!config.SetBlockStallingTimeout(0, &err));
    BOOST_CHECK(!config.SetBlockStallingTimeout(-1, &err));

    BOOST_CHECK_EQUAL(config.GetBlockDownloadWindow(), DEFAULT_BLOCK_DOWNLOAD_WINDOW);
    BOOST_CHECK(config.SetBlockDownloadWindow(2 * DEFAULT_BLOCK_DOWNLOAD_WINDOW, &err));
    BOOST_CHECK_EQUAL(config.GetBlockDownloadWindow(), 2 * DEFAULT_BLOCK_DOWNLOAD_WINDOW);
    BOOST_CHECK(!config.SetBlockDownloadWindow(0, &err));
    BOOST_CHECK(!config.SetBlockDownloadWindow(-1, &err));

    BOOST_CHECK_EQUAL(config.GetBlockDownloadSlowFetchTimeout(), DEFAULT_BLOCK_DOWNLOAD_SLOW_FETCH_TIMEOUT);
    BOOST_CHECK(config.SetBlockDownloadSlowFetchTimeout(2 * DEFAULT_BLOCK_DOWNLOAD_SLOW_FETCH_TIMEOUT, &err));
    BOOST_CHECK_EQUAL(config.GetBlockDownloadSlowFetchTimeout(), 2 * DEFAULT_BLOCK_DOWNLOAD_SLOW_FETCH_TIMEOUT);
    BOOST_CHECK(!config.SetBlockDownloadSlowFetchTimeout(0, &err));
    BOOST_CHECK(!config.SetBlockDownloadSlowFetchTimeout(-1, &err));

    BOOST_CHECK_EQUAL(config.GetBlockDownloadMaxParallelFetch(), DEFAULT_MAX_BLOCK_PARALLEL_FETCH);
    BOOST_CHECK(config.SetBlockDownloadMaxParallelFetch(2 * DEFAULT_MAX_BLOCK_PARALLEL_FETCH, &err));
    BOOST_CHECK_EQUAL(config.GetBlockDownloadMaxParallelFetch(), 2 * DEFAULT_MAX_BLOCK_PARALLEL_FETCH);
    BOOST_CHECK(!config.SetBlockDownloadMaxParallelFetch(0, &err));
    BOOST_CHECK(!config.SetBlockDownloadMaxParallelFetch(-1, &err));
}

BOOST_AUTO_TEST_CASE(p2p_config)
{
    GlobalConfig config {};
    std::string err {};

    BOOST_CHECK_EQUAL(config.GetP2PHandshakeTimeout(), DEFAULT_P2P_HANDSHAKE_TIMEOUT_INTERVAL);
    BOOST_CHECK(config.SetP2PHandshakeTimeout(2 * DEFAULT_P2P_HANDSHAKE_TIMEOUT_INTERVAL, &err));
    BOOST_CHECK_EQUAL(config.GetP2PHandshakeTimeout(), 2 * DEFAULT_P2P_HANDSHAKE_TIMEOUT_INTERVAL);
    BOOST_CHECK(!config.SetP2PHandshakeTimeout(0, &err));
    BOOST_CHECK(!config.SetP2PHandshakeTimeout(-1, &err));

    BOOST_CHECK_EQUAL(config.GetStreamSendRateLimit(), Stream::DEFAULT_SEND_RATE_LIMIT);
    BOOST_CHECK(config.SetStreamSendRateLimit(1000, &err));
    BOOST_CHECK_EQUAL(config.GetStreamSendRateLimit(), 1000);
    BOOST_CHECK(config.SetStreamSendRateLimit(0, &err));
    BOOST_CHECK(config.SetStreamSendRateLimit(-1, &err));
    BOOST_CHECK_EQUAL(config.GetStreamSendRateLimit(), -1);

    BOOST_CHECK_EQUAL(config.GetBanScoreThreshold(), DEFAULT_BANSCORE_THRESHOLD);
    BOOST_CHECK(config.SetBanScoreThreshold(2 * DEFAULT_BANSCORE_THRESHOLD, &err));
    BOOST_CHECK_EQUAL(config.GetBanScoreThreshold(), 2 * DEFAULT_BANSCORE_THRESHOLD);
    BOOST_CHECK(!config.SetBanScoreThreshold(0, &err));
    BOOST_CHECK(!config.SetBanScoreThreshold(-1, &err));
}

BOOST_AUTO_TEST_CASE(dust_config_test)
{
    GlobalConfig config {};
    std::string err {};

    BOOST_CHECK(config.SetDustLimitFactor(0, &err));
    BOOST_CHECK_EQUAL(config.GetDustLimitFactor(), 0);
    BOOST_CHECK(config.SetDustLimitFactor(100, &err));
    BOOST_CHECK_EQUAL(config.GetDustLimitFactor(), 100);
    BOOST_CHECK(config.SetDustLimitFactor(200, &err));
    BOOST_CHECK_EQUAL(config.GetDustLimitFactor(), 200);
    BOOST_CHECK(config.SetDustLimitFactor(300, &err));
    BOOST_CHECK_EQUAL(config.GetDustLimitFactor(), 300);

    BOOST_CHECK(!config.SetDustLimitFactor(-1, &err));
    BOOST_CHECK(!config.SetDustLimitFactor(301, &err));
}



BOOST_AUTO_TEST_SUITE_END()
