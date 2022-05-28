// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "test/test_novobitcoin.h"
#include "blockstreams.h"
#include "serialize.h"
#include "stream_test_helpers.h"

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(object_stream_deserialization_tests, BasicTestingSetup)

BOOST_AUTO_TEST_CASE(block)
{
    CBlock block{BuildRandomTestBlock()};
    std::vector<uint8_t> serializedData{Serialize(block)};

    CBlockStreamReader<CMemoryReader> stream{
        serializedData,
        {SER_NETWORK, INIT_PROTO_VERSION}};

    BOOST_REQUIRE_EQUAL(
        stream.GetRemainingTransactionsCount(),
        block.vtx.size());

    size_t itemCounter = 0;
    do
    {
        // read transaction for counting but ignore the result as we are not
        // interested in the content
        [[maybe_unused]]
        const CTransaction& transaction = stream.ReadTransaction();

        ++itemCounter;
    } while(!stream.EndOfStream());

    BOOST_REQUIRE_EQUAL(itemCounter, 3);
}

BOOST_AUTO_TEST_SUITE_END()
