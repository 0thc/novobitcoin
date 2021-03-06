// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.



#include <array>
#include "script/script.h"
#include "script_macros.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_SUITE(cscript_tests)

BOOST_AUTO_TEST_CASE(GetOp2)
{
    // input script, expected status, expected opcode, expected output script]
    using test_data_type = tuple< vector<uint8_t>, bool, opcodetype, vector<uint8_t> >;

    vector<test_data_type> test_data {
        { {OP_0}, true, static_cast<opcodetype>(0), {} }, // Note: OP_0 = 0
        { {1, 1}, true, static_cast<opcodetype>(1), {1} },
        { {2, 1, 2}, true, static_cast<opcodetype>(2), {1, 2} },
        { {3, 1, 2, 3}, true, static_cast<opcodetype>(3), {1, 2, 3} },

        { {OP_PUSHDATA1, 3, 1, 2, 3}, true, OP_PUSHDATA1, {1, 2, 3} },
        { {OP_PUSHDATA2, 3, 0, 1, 2, 3}, true, OP_PUSHDATA2, {1, 2, 3} },
        { {OP_PUSHDATA4, 3, 0, 0, 0, 1, 2, 3}, true, OP_PUSHDATA4, {1, 2, 3} },

        { {OP_1}, true, OP_1, {} },
        { {OP_2}, true, OP_2, {} },

        { {OP_1, 42}, true, OP_1, {} },

        { {OP_INVALIDOPCODE}, true, OP_INVALIDOPCODE, {}},

        { {}, false, OP_INVALIDOPCODE, {}},
        { {1}, false, OP_INVALIDOPCODE, {} },
        { {2}, false, OP_INVALIDOPCODE, {} },
        { {2, 1}, false, OP_INVALIDOPCODE, {} },
        { {0x4b, 1}, false, OP_INVALIDOPCODE, {} },

        { {OP_PUSHDATA1 }, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA1, 1}, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA2 }, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA2, 1}, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA2, 0, 1}, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA4 }, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA4, 1}, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA4, 0, 1}, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA4, 0, 0, 1}, false, OP_INVALIDOPCODE, {} },
        { {OP_PUSHDATA4, 0, 0, 0, 1}, false, OP_INVALIDOPCODE, {} },
    };
    for(const auto& [ip, exp_status, exp_opcode, exp_v] : test_data)
    {
        const CScript script(begin(ip), end(ip));
        auto it{script.begin()};
        opcodetype opcode;
        vector<uint8_t> v;
        const auto s = script.GetOp2(it, opcode, &v);
        BOOST_CHECK_EQUAL(exp_status, s);
        BOOST_CHECK_EQUAL(exp_opcode, opcode);
        BOOST_CHECK_EQUAL_COLLECTIONS(begin(exp_v), end(exp_v), begin(v),
                                      end(v));
    }
}

BOOST_AUTO_TEST_CASE(OpCount_tests)
{
    uint8_t a[] = {OP_1, OP_2, OP_2};
    BOOST_CHECK_EQUAL(0, CountOp(a, OP_0));
    BOOST_CHECK_EQUAL(1, CountOp(a, OP_1));
    BOOST_CHECK_EQUAL(2, CountOp(a, OP_2));

    array<uint8_t, 3> arr;
    copy(begin(a), end(a), begin(arr));
    BOOST_CHECK_EQUAL(0, CountOp(arr, OP_0));
    BOOST_CHECK_EQUAL(1, CountOp(arr, OP_1));
    BOOST_CHECK_EQUAL(2, CountOp(arr, OP_2));

    vector<uint8_t> v{begin(arr), end(arr)};
    BOOST_CHECK_EQUAL(0, CountOp(v, OP_0));
    BOOST_CHECK_EQUAL(1, CountOp(v, OP_1));
    BOOST_CHECK_EQUAL(2, CountOp(v, OP_2));

    CScript script{begin(v), end(v)};
    BOOST_CHECK_EQUAL(0, CountOp(script, OP_0));
    BOOST_CHECK_EQUAL(1, CountOp(script, OP_1));
    BOOST_CHECK_EQUAL(2, CountOp(script, OP_2));
}

BOOST_AUTO_TEST_CASE(GetSigOpCount)
{
    // input script, accurate, expected_count, expected_error
    using test_data_type = tuple< vector<uint8_t>, uint64_t, bool >;
    vector<test_data_type> test_data {
        { {}, 0, false },

        { {OP_1}, 0, false },

        { {OP_CHECKSIG}, 1, false },

        { {OP_CHECKSIG, OP_CHECKSIG}, 2, false },

        { {OP_CHECKMULTISIG}, 0, false },

        { {OP_CHECKMULTISIG, OP_CHECKMULTISIG}, 0, false },

        { {MULTISIG_LOCKING_2}, 2, false },

        { {MULTISIG_LOCKING_32}, 32, false },

        { {MULTISIG_2_IF_LOCKING}, 3, false },

        { {P2SH_LOCKING}, 0, false },
    };
    for(const auto& [ip, exp_n, exp_error] : test_data)
    {
        const CScript script(begin(ip), end(ip));
        bool error{false};
        const auto n = script.GetSigOpCount(error);
        BOOST_CHECK_EQUAL(exp_n, n);
        BOOST_CHECK_EQUAL(exp_error, error);
    }
}

BOOST_AUTO_TEST_SUITE_END()
