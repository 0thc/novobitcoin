// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "mining/factory.h"
#include "mining/journal_builder.h"
#include "mining/journaling_block_assembler.h"

#include "block_index_store.h"
#include "chainparams.h"
#include "coins.h"
#include "config.h"
#include "consensus/consensus.h"
#include "consensus/merkle.h"
#include "consensus/validation.h"
#include "policy/policy.h"
#include "pow.h"
#include "pubkey.h"
#include "script/script_num.h"
#include "script/standard.h"
#include "txmempool.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "validation.h"

#include "mempool_test_access.h"

#include "test/test_novobitcoin.h"

#include <memory>

#include <boost/test/unit_test.hpp>

using mining::BlockAssemblerRef;
using mining::CBlockTemplate;
using mining::JournalingBlockAssembler;

namespace
{
    mining::CJournalChangeSetPtr nullChangeSet {nullptr};

    class JournalingTestingSetup : public TestingSetup
    {
    public:
        JournalingTestingSetup()
            : TestingSetup(CBaseChainParams::MAIN, mining::CMiningFactory::BlockAssemblerType::JOURNALING)
        {}
    };

    class miner_tests_uid; // only used as unique identifier
}

template <>
struct CoinsDB::UnitTestAccess<miner_tests_uid>
{
    UnitTestAccess() = delete;

    static void SetBestBlock(
        CoinsDB& provider,
        const uint256& hashBlock)
    {
        provider.hashBlock = hashBlock;
    }
};
using TestAccessCoinsDB = CoinsDB::UnitTestAccess<miner_tests_uid>;

template <>
struct CBlockIndex::UnitTestAccess<miner_tests_uid>
{
    UnitTestAccess() = delete;

    static void SetTime( CBlockIndex& index, int64_t time)
    {
        index.nTime = time;
    }

    static void AddTime( CBlockIndex& index, int64_t time)
    {
        index.nTime += time;
    }

    static void SubTime( CBlockIndex& index, int64_t time)
    {
        index.nTime -= time;
    }

    static void SetHeight( CBlockIndex& index, int32_t height)
    {
        index.nHeight = height;
    }
};
using TestAccessCBlockIndex = CBlockIndex::UnitTestAccess<miner_tests_uid>;

static CFeeRate blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);

static struct {
    uint8_t extranonce;
    uint32_t nonce;
} blockinfo[] = {
    {2, 0x00696535}, {0, 0x48694e6a}, {0, 0x6a420e30}, {1, 0x13450d66},
    {0, 0xb24a4978}, {0, 0x84c2a523}, {0, 0xc2749498}, {0, 0x9a8a525e},
    {2, 0x822553d2}, {0, 0xdcc1994d}, {0, 0x0d9b3b45}, {2, 0xf3957a60},
    {1, 0xa0dfbd5d}, {0, 0x8158e6fe}, {0, 0x92033128}, {0, 0xbb80f704},
    {1, 0x191a82bf}, {3, 0x0f0f03ce}, {0, 0x957eef03}, {1, 0x5ca32e94},
    {4, 0xf22daa22}, {3, 0x5ba32b6b}, {1, 0xb825ff75}, {0, 0x3daf4714},
    {0, 0x5bd0faa1}, {4, 0xe34067f3}, {0, 0x0751b845}, {1, 0x15c49dc9},
    {3, 0xa53a34ef}, {0, 0x6769a5c5}, {3, 0x4c53ccad}, {1, 0xf1ee83a2},
    {3, 0x3f6ef4ff}, {0, 0x513fc518}, {4, 0x1ab6ab28}, {1, 0x739aa44b},
    {9, 0x06c3781a}, {8, 0x59f14ef0}, {1, 0x9bf6301a}, {1, 0x7e42ca0f},
    {2, 0x65ac293c}, {1, 0x227f345a}, {1, 0xa8618d60}, {2, 0x895cad28},
    {5, 0xc1db74f2}, {1, 0x0ce73dca}, {0, 0x55fe6a6b}, {0, 0x9df6b846},
    {14, 0x8c4bfb4f}, {3, 0x4006e8a1}, {4, 0x4a93a161}, {1, 0x82072ce2},
    {1, 0x4a737379}, {2, 0x3adff156}, {0, 0xcb455139}, {4, 0x7813f57e},
    {2, 0x5d8ab529}, {3, 0xa6e21248}, {0, 0x7b61de2d}, {16, 0xcd7e9f57},
    {3, 0x5529527b}, {0, 0xa97bc5f3}, {0, 0xfd1d230e}, {2, 0x433f7234},
    {4, 0x28950b64}, {0, 0x26fa6e8d}, {3, 0x2d85f647}, {26, 0x1567649a},
    {3, 0x95b2521b}, {14, 0x9785fcf9}, {0, 0x134f70d1}, {5, 0xf11c05d8},
    {9, 0x765f57c0}, {28, 0xdb16659d}, {4, 0x4531250c}, {11, 0x3849e0e7},
    {8, 0x4bb558b1}, {0, 0xfcebe3ff}, {3, 0xe70fc56d}, {6, 0x787dcaee},
    {2, 0x45006d95}, {3, 0x66233a93}, {18, 0x5cec85b9}, {1, 0x4cbca70e},
    {0, 0x39350419}, {5, 0xcd8ce429}, {1, 0x53f574c4}, {1, 0x7c188bd3},
    {9, 0x726399e2}, {17, 0x604ab8a9}, {1, 0xacdb1fb1}, {2, 0xe9cf2145},
    {58, 0x7717527f}, {17, 0xfe4f8ff3}, {23, 0x2c12322a}, {14, 0x641bc9cc},
    {26, 0xc8290516}, {2, 0x63e95a83}, {4, 0x321d7f28}, {5, 0x725d35c2},
    {6, 0x55b29e1a}, {19, 0xedf874ae}, {4, 0x838017f4}, {19, 0xdb85e253},
    {5, 0xdacb144f}, {21, 0x011219fe}, {27, 0x707dd29c}, {9, 0x249212fe},
    {29, 0x9218bbcd}, {19, 0x19532352},
};

// NOTE: These tests rely on CreateNewBlock doing its own self-validation!
void Test_CreateNewBlock_validity(TestingSetup& testingSetup)
{
    CScript scriptPubKeyPadding = CScript() << 0x11de784a << OP_DROP;

    // Note that by default, these tests run with size accounting enabled.
    CScript scriptPubKey = CScript() << OP_DUP
                                     << OP_HASH160
                                     << ParseHex("0567b5f0544536d023fbb123b830f626d9c80389")
                                     << OP_EQUALVERIFY
                                     << OP_CHECKSIG;

    std::unique_ptr<CBlockTemplate> pblocktemplate;
    CMutableTransaction tx;
    CScript script;
    uint256 hash;
    TestMemPoolEntryHelper entry;
    entry.nFee = Amount(11);
    entry.nHeight = 11;

    LOCK(cs_main);
    fCheckpointsEnabled = false;

    // Simple block creation, nothing special yet:
    CBlockIndex* pindexPrev {nullptr};
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));

    // We can't make transactions until we have inputs. Therefore, load 110 blocks :)
    static_assert(sizeof(blockinfo) / sizeof(*blockinfo) == 110, "Should have 110 blocks to import");
    int32_t baseheight = 0;
    std::vector<CTransactionRef> txFirst;
    for (size_t i = 0; i < sizeof(blockinfo) / sizeof(*blockinfo); ++i) {
        // pointer for convenience.
        CBlockRef blockRef = pblocktemplate->GetBlockRef();
        CBlock *pblock = blockRef.get();
        pblock->nVersion = VERSIONBITS_TOP_BITS;
        pblock->nTime = chainActive.Tip()->GetMedianTimePast() + 1;
        pblock->nBits = GetNextWorkRequired(chainActive.Tip(), pblock, GlobalConfig::GetConfig());
        CMutableTransaction txCoinbase(*pblock->vtx[0]);
        txCoinbase.nVersion = 1;
        txCoinbase.vin[0].scriptSig = CScript() << chainActive.Height() + 1 << 0x11de784a << blockinfo[i].extranonce;
        txCoinbase.vout.resize(1);
        txCoinbase.vout[0].scriptPubKey = CScript();

        pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
        if (txFirst.size() == 0) baseheight = chainActive.Height();
        if (txFirst.size() < 4) txFirst.push_back(pblock->vtx[0]);
        pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
        pblock->nNonce = blockinfo[i].nonce;

        std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
        BOOST_CHECK(ProcessNewBlock(testingSetup.testConfig, shared_pblock, true, nullptr));

        pblock->hashPrevBlock = pblock->GetHash();
    }

    // Just to make sure we can still make simple blocks.
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));

    const Amount BLOCKSUBSIDY = 2000000 * COIN;
    const Amount LOWFEE = 10*CENT;
    const Amount HIGHFEE = COIN;
    const Amount HIGHERFEE = 8 * COIN;

    tx.nVersion = 1;
    // block sigops > limit: 1000 CHECKMULTISIG + 1
    tx.vin.resize(1);
    // NOTE: OP_NOP is used to force 20 SigOps for the CHECKMULTISIG
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_0 << OP_0 << OP_NOP
                                    << OP_CHECKMULTISIG << OP_1;
    tx.vin[0].prevout = COutPoint(txFirst[0]->GetId(), 0);
    tx.vout.resize(1);
    tx.vout[0].nValue = BLOCKSUBSIDY;
    for (unsigned int i = 0; i < 1001; ++i) {
        tx.vout[0].nValue -= LOWFEE;
        hash = tx.GetId();
        // Only first tx spends coinbase.
        bool spendsCoinbase = (i == 0) ? true : false;
        // If we don't set the # of sig ops in the CTxMemPoolEntry, template
        // creation fails when validating.
        mempool.AddUnchecked(hash,
                             entry.Fee(LOWFEE)
                                 .Time(GetTime())
                                 .SpendsCoinbase(spendsCoinbase)
                                 .FromTx(tx),
                             TxStorage::memory,
                             nullChangeSet);
        tx.vin[0].prevout = COutPoint(hash, 0);
    }

    testingSetup.testConfig.SetTestBlockCandidateValidity(false);
    BOOST_CHECK_NO_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    testingSetup.testConfig.SetTestBlockCandidateValidity(true);
    BOOST_CHECK_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev), std::runtime_error);

    mempool.Clear();

    // block size > limit
    tx.vout[0].scriptPubKey = CScript();
    // 18 * (520char + DROP) + OP_1 = 9433 bytes
    std::vector<uint8_t> vchData(520);
    for (unsigned int i = 0; i < 18; ++i) {
        tx.vout[0].scriptPubKey << vchData << OP_DROP;
    }

    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout = COutPoint(txFirst[0]->GetId(), 0);
    tx.vout[0].nValue = BLOCKSUBSIDY;
    // tx.vout[0].scriptPubKey = scriptPubKeyPadding;
    for (unsigned int i = 0; i < 128; ++i) {
        tx.vout[0].nValue -= HIGHERFEE;
        hash = tx.GetId();
        // Only first tx spends coinbase.
        bool spendsCoinbase = (i == 0) ? true : false;
        mempool.AddUnchecked(hash,
                             entry.Fee(HIGHERFEE)
                                 .Time(GetTime())
                                 .SpendsCoinbase(spendsCoinbase)
                                 .FromTx(tx),
                             TxStorage::memory,
                             nullChangeSet);
        tx.vin[0].prevout = COutPoint(hash, 0);
    }
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    mempool.Clear();

    // Orphan in mempool, template creation fails.
    hash = tx.GetId();
    mempool.AddUnchecked(hash, entry.Fee(HIGHERFEE).Time(GetTime()).FromTx(tx), TxStorage::memory, nullChangeSet);
    testingSetup.testConfig.SetTestBlockCandidateValidity(false);
    BOOST_CHECK_NO_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    testingSetup.testConfig.SetTestBlockCandidateValidity(true);
    BOOST_CHECK_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev), std::runtime_error);

    mempool.Clear();

    // Child with higher priority than parent.
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout = COutPoint(txFirst[1]->GetId(), 0);
    tx.vout[0].nValue = BLOCKSUBSIDY - HIGHFEE;
    tx.vout[0].scriptPubKey = scriptPubKeyPadding;
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    tx.vin[0].prevout = COutPoint(hash, 0);
    tx.vin.resize(2);
    tx.vin[1].scriptSig = CScript() << OP_1;
    tx.vin[1].prevout = COutPoint(txFirst[0]->GetId(), 0);
    // First txn output + fresh coinbase - new txn fee.
    tx.vout[0].nValue = tx.vout[0].nValue + BLOCKSUBSIDY - HIGHERFEE;
    tx.vout[0].scriptPubKey = scriptPubKeyPadding;
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(HIGHERFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    mempool.Clear();

    // Coinbase in mempool, template creation fails.
    tx.vin.resize(1);
    tx.vin[0].prevout = COutPoint();
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_1;
    tx.vout[0].nValue = Amount(0);
    tx.vout[0].scriptPubKey = scriptPubKeyPadding;
    hash = tx.GetId();
    // Give it a fee so it'll get mined.
    mempool.AddUnchecked(
        hash,
        entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    testingSetup.testConfig.SetTestBlockCandidateValidity(false);
    BOOST_CHECK_NO_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    testingSetup.testConfig.SetTestBlockCandidateValidity(true);
    BOOST_CHECK_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev), std::runtime_error);

    mempool.Clear();

    // Invalid (pre-p2sh) txn in mempool, template creation fails.
    std::array<int64_t, CBlockIndex::nMedianTimeSpan> times;
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Trick the MedianTimePast.
        times[i] = chainActive.Tip()
                       ->GetAncestor(chainActive.Tip()->GetHeight() - i)
                       ->GetBlockTime();
    }

    tx.vin[0].prevout = COutPoint(txFirst[0]->GetId(), 0);
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = BLOCKSUBSIDY - LOWFEE;
    script = CScript() << OP_0;
    tx.vout[0].scriptPubKey = GetScriptForDestination(CScriptID(script));
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    tx.vin[0].prevout = COutPoint(hash, 0);
    tx.vin[0].scriptSig = CScript()
                          << std::vector<uint8_t>(script.begin(), script.end());
    tx.vout[0].nValue -= LOWFEE;
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    testingSetup.testConfig.SetTestBlockCandidateValidity(false);
    BOOST_CHECK_NO_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    testingSetup.testConfig.SetTestBlockCandidateValidity(true);
    BOOST_CHECK_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev), std::runtime_error);

    mempool.Clear();
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Restore the MedianTimePast.
        TestAccessCBlockIndex::SetTime(
            *chainActive.Tip()->GetAncestor(chainActive.Tip()->GetHeight() - i),
            times[i]);
    }

    // Double spend txn pair in mempool, template creation fails.
    tx.vin[0].prevout = COutPoint(txFirst[0]->GetId(), 0);
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = BLOCKSUBSIDY - HIGHFEE;
    tx.vout[0].scriptPubKey = CScript() << 0x11de784a << OP_DROP << OP_1;
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    tx.vout[0].scriptPubKey = CScript() << 0x11de784a << OP_DROP << OP_2;
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx),
        TxStorage::memory, nullChangeSet);
    testingSetup.testConfig.SetTestBlockCandidateValidity(false);
    BOOST_CHECK_NO_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    testingSetup.testConfig.SetTestBlockCandidateValidity(true);
    BOOST_CHECK_THROW(mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev), std::runtime_error);
    mempool.Clear();

    {
        // Subsidy changing.
        auto tipMarker = chainActive.Tip();
        // Create an actual 209999-long block chain (without valid blocks).
        while (chainActive.Tip()->GetHeight() < 209999) {
            CBlockHeader header;
            header.nTime = GetTime();
            header.hashPrevBlock = chainActive.Tip()->GetBlockHash();
            header.nBits = chainActive.Tip()->GetBits();
            CBlockIndex* next = mapBlockIndex.Insert( header );
            TestAccessCoinsDB::SetBestBlock(*pcoinsTip, next->GetBlockHash());
            chainActive.SetTip(next);
        }
        BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
        // Extend to a 210000-long block chain.
        while (chainActive.Tip()->GetHeight() < 210000) {
            CBlockHeader header;
            header.nTime = GetTime();
            header.hashPrevBlock = chainActive.Tip()->GetBlockHash();
            header.nBits = chainActive.Tip()->GetBits();
            CBlockIndex* next = mapBlockIndex.Insert( header );
            TestAccessCoinsDB::SetBestBlock(*pcoinsTip, next->GetBlockHash());
            chainActive.SetTip(next);
        }
        BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));

        if (testingSetup.testConfig.GetMiningCandidateBuilder() == mining::CMiningFactory::BlockAssemblerType::JOURNALING)
        {
            mining::g_miningFactory.reset();
        }

        // Remove dummy blocks that were created in this scope from the active chain.
        chainActive.SetTip( tipMarker );
        TestAccessCoinsDB::SetBestBlock(*pcoinsTip, tipMarker->GetBlockHash());
    }

    if (testingSetup.testConfig.GetMiningCandidateBuilder() == mining::CMiningFactory::BlockAssemblerType::JOURNALING)
    {
        mining::g_miningFactory = std::make_unique<mining::CMiningFactory>(testingSetup.testConfig);
    }

    // non-final txs in mempool
    SetMockTime(chainActive.Tip()->GetMedianTimePast() + 1);
    // height map
    std::vector<int32_t> prevheights;

    // Relative height locked.
    tx.nVersion = 1;
    tx.vin.resize(1);
    prevheights.resize(1);
    // Only 1 transaction.
    tx.vin[0].prevout = COutPoint(txFirst[0]->GetId(), 0);
    tx.vin[0].scriptSig = CScript() << OP_1;
    // txFirst[0] is the 2nd block
    tx.vin[0].nSequence = chainActive.Tip()->GetHeight() + 1;
    prevheights[0] = baseheight + 1;
    tx.vout.resize(1);
    tx.vout[0].nValue = BLOCKSUBSIDY - HIGHFEE;
    tx.vout[0].scriptPubKey = scriptPubKeyPadding;
    tx.nLockTime = 0;
    hash = tx.GetId();
    mempool.AddUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx),
        TxStorage::memory, nullChangeSet);

    {
        // Locktime passes.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransactionForCurrentBlock(
                        config,
                        CTransaction(tx),
                        chainActive.Height(),
                        chainActive.Tip()->GetMedianTimePast(),
                        state));
    }

    // Sequence locks fail.

    // Relative time locked.
    tx.vin[0].prevout = COutPoint(txFirst[1]->GetId(), 0);
    // txFirst[1] is the 3rd block.
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG |
                          (((chainActive.Tip()->GetMedianTimePast() + 1 -
                             chainActive[1]->GetMedianTimePast()) >>
                            CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) +
                           1);
    prevheights[0] = baseheight + 2;
    hash = tx.GetId();
    mempool.AddUnchecked(hash, entry.Time(GetTime()).FromTx(tx), TxStorage::memory, nullChangeSet);

    {
        // Locktime passes.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransactionForCurrentBlock(
                        config,
                        CTransaction(tx),
                        chainActive.Height(),
                        chainActive.Tip()->GetMedianTimePast(),
                        state));
    }

    // Sequence locks fail.

    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Trick the MedianTimePast.
        TestAccessCBlockIndex::AddTime(
            *chainActive.Tip()->GetAncestor(chainActive.Tip()->GetHeight() - i),
            512);
    }

    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Undo tricked MTP.
        TestAccessCBlockIndex::SubTime(
            *chainActive.Tip()->GetAncestor(chainActive.Tip()->GetHeight() - i),
            512);
    }

    // Absolute height locked.
    tx.vin[0].prevout = COutPoint(txFirst[2]->GetId(), 0);
    tx.vin[0].nSequence = CTxIn::SEQUENCE_FINAL - 1;
    prevheights[0] = baseheight + 3;
    tx.nLockTime = chainActive.Tip()->GetHeight() + 1;
    hash = tx.GetId();
    mempool.AddUnchecked(hash, entry.Time(GetTime()).FromTx(tx), TxStorage::memory, nullChangeSet);

    {
        // Locktime fails.
        CValidationState state;
        BOOST_CHECK(!ContextualCheckTransaction(
                        testingSetup.testConfig,
                        CTransaction(tx), state,
                        chainActive.Height()+1,
                        chainActive.Tip()->GetMedianTimePast(),
                        true));
        BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-nonfinal");
    }

    // Sequence locks pass.

    {
        // Locktime passes on 2nd block.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransaction(
            config, CTransaction(tx), state, chainActive.Tip()->GetHeight() + 2,
            chainActive.Tip()->GetMedianTimePast(), false));
    }

    // Absolute time locked.
    tx.vin[0].prevout = COutPoint(txFirst[3]->GetId(), 0);
    tx.nLockTime = chainActive.Tip()->GetMedianTimePast();
    prevheights.resize(1);
    prevheights[0] = baseheight + 4;
    hash = tx.GetId();
    mempool.AddUnchecked(hash, entry.Time(GetTime()).FromTx(tx), TxStorage::memory, nullChangeSet);

    {
        // Locktime fails.
        CValidationState state;
        BOOST_CHECK(!ContextualCheckTransaction(
                        testingSetup.testConfig,
                        CTransaction(tx), state,
                        chainActive.Height()+1,
                        chainActive.Tip()->GetMedianTimePast(),
                        true));
        BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-nonfinal");
    }

    // Sequence locks pass.

    {
        // Locktime passes 1 second later.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransaction(
            config, CTransaction(tx), state, chainActive.Tip()->GetHeight() + 1,
            chainActive.Tip()->GetMedianTimePast() + 1, false));
    }

    // mempool-dependent transactions (not added)
    tx.vin[0].prevout = COutPoint(hash, 0);
    prevheights[0] = chainActive.Tip()->GetHeight() + 1;
    tx.nLockTime = 0;
    tx.vin[0].nSequence = 0;

    {
        // Locktime passes.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransactionForCurrentBlock(
                        config,
                        CTransaction(tx),
                        chainActive.Height(),
                        chainActive.Tip()->GetMedianTimePast(),
                        state));
    }


    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));

    // None of the of the absolute height/time locked tx should have made it
    // into the template because we still check IsFinalTx in CreateNewBlock, but
    // relative locked txs will if inconsistently added to mempool. For now
    // these will still generate a valid template until BIP68 soft fork.
    BOOST_CHECK_EQUAL(pblocktemplate->GetBlockRef()->vtx.size(), 3UL);
    // However if we advance height by 1 and time by 512, all of them should be
    // mined.
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Trick the MedianTimePast.
        TestAccessCBlockIndex::AddTime(
            *chainActive.Tip()->GetAncestor(chainActive.Tip()->GetHeight() - i),
            512);
    }
    TestAccessCBlockIndex::SetHeight( *chainActive.Tip(), chainActive.Tip()->GetHeight() + 1 );
    SetMockTime(chainActive.Tip()->GetMedianTimePast() + 1);

    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    BOOST_CHECK_EQUAL(pblocktemplate->GetBlockRef()->vtx.size(), 5UL);

    TestAccessCBlockIndex::SetHeight( *chainActive.Tip(), chainActive.Tip()->GetHeight() - 1 );
    SetMockTime(0);
    mempool.Clear();

    fCheckpointsEnabled = true;
}

void CheckBlockMaxSize(TestingSetup& testingSetup, uint64_t size, uint64_t expected)
{
    BOOST_REQUIRE(mining::g_miningFactory.get() == nullptr);
    testingSetup.testConfig.SetMaxGeneratedBlockSize(size);
    mining::CMiningFactory miningFactory { testingSetup.testConfig };
    BOOST_CHECK_EQUAL(miningFactory.GetAssembler()->GetMaxGeneratedBlockSize(), expected);
}

void Test_BlockAssembler_construction(TestingSetup& testingSetup)
{
    // we need to delete global mining factory because we want to
    // create new mining factory for testing and JBA does not behave ok when
    // there are multiple instances of it
    mining::g_miningFactory.reset();

    uint64_t nDefaultMaxGeneratedBlockSize = testingSetup.testConfig.GetMaxGeneratedBlockSize();
    uint64_t nDefaultMaxBlockSize = testingSetup.testConfig.GetMaxBlockSize();

    // We are working on a fake chain and need to protect ourselves.
    LOCK(cs_main);

    // Test around historical 1MB (plus one byte because that's mandatory)
    BOOST_REQUIRE(testingSetup.testConfig.SetMaxBlockSize(ONE_MEGABYTE + 1));
    CheckBlockMaxSize(testingSetup, 0, 1000);
    CheckBlockMaxSize(testingSetup, 1000, 1000);
    CheckBlockMaxSize(testingSetup, 1001, 1001);
    CheckBlockMaxSize(testingSetup, 12345, 12345);

    CheckBlockMaxSize(testingSetup, ONE_MEGABYTE - 1001, ONE_MEGABYTE - 1001);
    CheckBlockMaxSize(testingSetup, ONE_MEGABYTE - 1000, ONE_MEGABYTE - 1000);
    CheckBlockMaxSize(testingSetup, ONE_MEGABYTE - 999, ONE_MEGABYTE - 999);
    CheckBlockMaxSize(testingSetup, ONE_MEGABYTE, ONE_MEGABYTE - 999);

    // Test around default cap
    BOOST_REQUIRE(testingSetup.testConfig.SetMaxBlockSize(nDefaultMaxBlockSize));

    // Now we can use the default max block size.
    CheckBlockMaxSize(testingSetup, nDefaultMaxBlockSize - 1001, nDefaultMaxBlockSize - 1001);
    CheckBlockMaxSize(testingSetup, nDefaultMaxBlockSize - 1000, nDefaultMaxBlockSize - 1000);
    CheckBlockMaxSize(testingSetup, nDefaultMaxBlockSize - 999, nDefaultMaxBlockSize - 1000);
    CheckBlockMaxSize(testingSetup, nDefaultMaxBlockSize, nDefaultMaxBlockSize - 1000);

    // If the parameter is not specified, we use
    // max(1K, min(DEFAULT_MAX_BLOCK_SIZE - 1K, DEFAULT_MAX_GENERATED_BLOCK_SIZE))
    {
        const auto expected { std::max(ONE_KILOBYTE,
                                std::min(nDefaultMaxBlockSize - ONE_KILOBYTE,
                                    nDefaultMaxGeneratedBlockSize)) };

        // Set generated max size to default
        CheckBlockMaxSize(testingSetup, nDefaultMaxGeneratedBlockSize, expected);
    }
}

void CheckBlockMaxSizeForTime(TestingSetup& testingSetup, uint64_t medianPastTime, uint64_t expectedSize)
{
    BlockIndexStore blockIndexStore;

    {
        LOCK(cs_main);

        // Construct chain  with desired median time. Set time of each block to
        // the same value to get desired median past time.
        int32_t height = 0;
        uint256 prevHash;
        do
        {
            CBlockHeader header;
            header.nTime = medianPastTime;
            header.hashPrevBlock = prevHash;
            header.nBits = GetNextWorkRequired(chainActive.Tip(), &header, GlobalConfig::GetConfig());
            CBlockIndex* next = blockIndexStore.Insert( header );

            prevHash = next->GetBlockHash();

            // chainActive is used by BlockAssembler to get median past time, which is used to select default block size
            chainActive.SetTip( next );
        }
        while(++height < 11);
    }

    // Make sure that we got correct median past time.
    BOOST_REQUIRE_EQUAL(chainActive.Tip()->GetMedianTimePast(), medianPastTime);


    BOOST_REQUIRE(mining::g_miningFactory.get() == nullptr);
    mining::CMiningFactory miningFactory { testingSetup.testConfig };
    BOOST_CHECK_EQUAL(miningFactory.GetAssembler()->GetMaxGeneratedBlockSize(), expectedSize);

    {
        LOCK(cs_main);

        chainActive.SetTip(nullptr); // cleanup
    }
}

void Test_BlockAssembler_construction_activate_new_blocksize(TestingSetup& testingSetup)
{
    // we need to delete global mining factory because we want to
    // create new mining factory for testing and JBA does not behave ok when
    // there are multiple instances of it
    mining::g_miningFactory.reset();

    DefaultBlockSizeParams defaultParams{
        // max block size
        6000,
        // max generated block size
        4000
    };

    testingSetup.testConfig.SetDefaultBlockSizeParams(defaultParams);

    CheckBlockMaxSizeForTime(testingSetup, 1000, 4000);

    // When explicitly set, defaults values must not be used
    testingSetup.testConfig.SetMaxGeneratedBlockSize(3333);
    CheckBlockMaxSizeForTime(testingSetup, 10001, 3333);
}


void Test_JournalingBlockAssembler_Construction(TestingSetup& testingSetup)
{
    CScript scriptPubKey = CScript() << OP_DUP
                                     << OP_HASH160
                                     << ParseHex("0567b5f0544536d023fbb123b830f626d9c80389")
                                     << OP_EQUALVERIFY
                                     << OP_CHECKSIG;
    CBlockIndex* pindexPrev {nullptr};

    std::unique_ptr<CBlockTemplate> bt { mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev) };
    BOOST_REQUIRE(bt);
    BOOST_REQUIRE(bt->GetBlockRef());
    BOOST_CHECK_EQUAL(bt->GetBlockRef()->vtx.size(), 1);
}

void Test_CreateNewBlock_JBA_Config(TestingSetup& testingSetup)
{
    CScript scriptPubKeyPadding = CScript() << 0x11de784a << OP_DROP;
    CScript scriptPubKey = CScript() << OP_DUP
                                     << OP_HASH160
                                     << ParseHex("0567b5f0544536d023fbb123b830f626d9c80389")
                                     << OP_EQUALVERIFY
                                     << OP_CHECKSIG;
    TestMemPoolEntryHelper entry {};
    entry.nFee = Amount(11);
    entry.nHeight = 11;

    gArgs.ForceSetArg("-jbamaxtxnbatch", "1");
    gArgs.ForceSetArg("-jbafillafternewblock", "0");
    auto* jbaPtr = dynamic_cast<JournalingBlockAssembler*>(mining::g_miningFactory->GetAssembler().get());
    BOOST_REQUIRE(jbaPtr != nullptr);
    jbaPtr->ReadConfigParameters();

    LOCK(cs_main);
    fCheckpointsEnabled = false;

    // Simple block creation, nothing special yet:
    CBlockIndex* pindexPrev {nullptr};
    std::unique_ptr<CBlockTemplate> pblocktemplate {};
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));

    // We can't make transactions until we have inputs. Therefore, load 100 blocks
    std::vector<CTransactionRef> txFirst;
    for (size_t i = 0; i < sizeof(blockinfo) / sizeof(*blockinfo); ++i) {
        // pointer for convenience.
        CBlockRef blockRef = pblocktemplate->GetBlockRef();
        CBlock *pblock = blockRef.get();
        pblock->nVersion = VERSIONBITS_TOP_BITS;
        pblock->nTime = chainActive.Tip()->GetMedianTimePast() + 1;
        pblock->nBits = GetNextWorkRequired(chainActive.Tip(), pblock, GlobalConfig::GetConfig());
        CMutableTransaction txCoinbase(*pblock->vtx[0]);
        txCoinbase.nVersion = 1;
        txCoinbase.vin[0].scriptSig = CScript() << chainActive.Height() + 1 << 0x11de784a << blockinfo[i].extranonce;
        txCoinbase.vout.resize(1);
        txCoinbase.vout[0].scriptPubKey = CScript();
        pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
        if (txFirst.size() < 4)
            txFirst.push_back(pblock->vtx[0]);
        pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
        pblock->nNonce = blockinfo[i].nonce;
        std::shared_ptr<const CBlock> shared_pblock = std::make_shared<const CBlock>(*pblock);
        BOOST_CHECK(ProcessNewBlock(testingSetup.testConfig, shared_pblock, true, nullptr));
        pblock->hashPrevBlock = pblock->GetHash();
    }

    const Amount BLOCKSUBSIDY { 2000000 * COIN };
    const Amount LOWFEE {10*CENT};
    constexpr unsigned NUM_TXNS {1000};

    CMutableTransaction tx {};
    tx.nVersion = 1;
    tx.vin.resize(1);
    tx.vin[0].prevout = COutPoint(txFirst[0]->GetId(), 0);
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout.resize(1);
    tx.vout[0].nValue = BLOCKSUBSIDY;
    tx.vout[0].scriptPubKey = scriptPubKeyPadding;

    mempool.Clear();
    for (unsigned int i = 0; i < NUM_TXNS; ++i) {
        tx.vout[0].nValue -= LOWFEE;
        uint256 hash = tx.GetId();
        // Only first tx spends coinbase.
        bool spendsCoinbase = (i == 0) ? true : false;
        mempool.AddUnchecked(hash,
                             entry.Fee(LOWFEE)
                                 .Time(GetTime())
                                 .SpendsCoinbase(spendsCoinbase)
                                 .FromTx(tx),
                             TxStorage::memory,
                             nullChangeSet);
        tx.vin[0].prevout = COutPoint(hash, 0);
    }

    // CreateNewBlock will only include what we have processed so far from the journal
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    BOOST_CHECK(pblocktemplate->GetBlockRef()->vtx.size() < NUM_TXNS);

    gArgs.ForceSetArg("-jbamaxtxnbatch", "1");
    gArgs.ForceSetArg("-jbafillafternewblock", "1");
    jbaPtr->ReadConfigParameters();
    // CreateNewBlock will finish processing and including everything in the journal
    BOOST_CHECK(pblocktemplate = mining::g_miningFactory->GetAssembler()->CreateNewBlock(scriptPubKey, pindexPrev));
    BOOST_CHECK_EQUAL(pblocktemplate->GetBlockRef()->vtx.size(), NUM_TXNS + 1);
}


BOOST_FIXTURE_TEST_SUITE(miner_tests_journal, JournalingTestingSetup)
BOOST_AUTO_TEST_CASE(CreateNewBlock_validity)
{
    Test_CreateNewBlock_validity(*this);
}
BOOST_AUTO_TEST_CASE(BlockAssembler_construction)
{
    Test_BlockAssembler_construction(*this);
}
BOOST_AUTO_TEST_CASE(BlockAssembler_construction_activate_new_blocksize)
{
    Test_BlockAssembler_construction_activate_new_blocksize(*this);
}
BOOST_AUTO_TEST_CASE(JournalingBlockAssembler_Construction)
{
    Test_JournalingBlockAssembler_Construction(*this);
}
BOOST_AUTO_TEST_CASE(CreateNewBlock_JBA_Config)
{
    Test_CreateNewBlock_JBA_Config(*this);
}
BOOST_AUTO_TEST_SUITE_END()
