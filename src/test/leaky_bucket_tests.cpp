// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <leaky_bucket.h>

#include <chrono>
#include <thread>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(LeakyBucketTests)

BOOST_AUTO_TEST_CASE(FillDrain)
{
    using namespace std::chrono_literals;
    LeakyBucket<std::chrono::milliseconds> bucket { 1000, 1ms };

    // Check we're not overflowing at start
    BOOST_CHECK(!bucket.Overflowing());
    BOOST_CHECK_EQUAL(bucket.GetFillLevel(), 0);

    // Part fill
    BOOST_CHECK(!(bucket += 500));
    BOOST_CHECK(bucket.GetFillLevel() > 0);

    // Check we drain at something like the correct rate
    size_t startLevel { bucket.GetFillLevel() };
    std::this_thread::sleep_for(5ms);
    BOOST_CHECK(bucket.GetFillLevel() < startLevel);
    BOOST_CHECK(bucket.GetFillLevel() > 0);
    std::this_thread::sleep_for(500ms);
    BOOST_CHECK_EQUAL(bucket.GetFillLevel(), 0);

    // Check overflow
    BOOST_CHECK(!(bucket += 1000));
    BOOST_CHECK(bucket += 1000);
    BOOST_CHECK(bucket.Overflowing());

    // Test creating partially filled
    LeakyBucket<std::chrono::milliseconds> bucket2 { 1000, 500, 1ms };
    BOOST_CHECK(bucket2.GetFillLevel() > 0);
    BOOST_CHECK(!bucket2.Overflowing());
}

BOOST_AUTO_TEST_SUITE_END()
