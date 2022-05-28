// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "chainparams.h"
#include "consensus/merkle.h"

#include "policy/policy.h"
#include "script/script_num.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include "core_io.h"

#include <cassert>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char *pszTimestamp,
                                 const CScript &genesisOutputScript,
                                 uint32_t nTime, uint32_t nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const Amount genesisReward) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig =
        CScript() << 0x11de784a
                  << std::vector<uint8_t>((const uint8_t *)pszTimestamp,
                                          (const uint8_t *)pszTimestamp +
                                              strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation transaction
 * cannot be spent since it did not originally exist in the database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000,
 * hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d7fffff, nNonce=2083236893,
 * vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase
 * 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=2000000.0000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const Amount genesisReward) {
    const char *pszTimestamp =
        "The Times 02/Dec/2021 Fourth jab to fight variants";
    const CScript genesisOutputScript =
      CScript() << OP_DUP
                << OP_HASH160
                << ParseHex("0567b5f0544536d023fbb123b830f626d9c80389")
                << OP_EQUALVERIFY
                << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce,
                              nBits, nVersion, genesisReward);
}


/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000;

        // Note: Because BIP34Height is less than 17, clients will face an unusual corner case with BIP34 encoding.
        // The "correct" encoding for BIP34 blocks at height <= 16 uses OP_1 (0x81) through OP_16 (0x90) as a single
        // byte (i.e. "[shortest possible] encoded CScript format"), not a single byte with length followed by the
        // little-endian encoded version of the height as mentioned in BIP34. The BIP34 spec document itself ought to
        // be updated to reflect this.
        // https://github.com/bitcoin/bitcoin/pull/14633
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("00000000df5c5164b4516916ac7a520df6039e8cac3d4ac9235e15eace81acd2");
        consensus.BIP66Height = 1;
        consensus.powLimit = uint256S(
            "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 150;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;

        // The half life for the ASERT DAA. For every (nASERTHalfLife) seconds behind schedule the blockchain gets,
        // difficulty is cut in half. Doubled if blocks are ahead of schedule.
        consensus.nUnsteadyASERTHalfLife = UNSTEADY_ASERT_HALF_LIFE;
        consensus.nSteadyASERTHalfLife = STEADY_ASERT_HALF_LIFE;
        consensus.SteadyASERTHeight = 100000;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0000000000000000000000000000000000000000000000000000d9e4a0215757");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid = uint256S("00000000be8113904edd472404e861e0492f980af7e5345fdd87ce0e41b072ba");

        consensus.asertAnchorParams = Consensus::Params::ASERTAnchor{
          1,            // anchor block height
          0x1d00ffff,   // anchor block nBits
          1638457291,   // anchor block previous block timestamp
        };

        /**
         * The message start string is designed to be unlikely to occur in
         * normal data. The characters are rarely used upper ASCII, not valid as
         * UTF-8, and produce a large 32-bit integer with any alignment.
         */
        diskMagic[0] = 0xe0;        // mainnet - diskMagic - `babebee0` in little-endian
        diskMagic[1] = 0xbe;
        diskMagic[2] = 0xbe;
        diskMagic[3] = 0xba;
        netMagic[0] = 0xe0;         // mainnet - netMagic - `cafefee0` in little-endian
        netMagic[1] = 0xfe;
        netMagic[2] = 0xfe;
        netMagic[3] = 0xca;
        nDefaultPort = 8666;        // [MainNet] original port: 8333
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(1638457291, 0x7823b7d4, 0x1d00ffff, 1, 2000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0000000000b3de1ef5bd7c20708dbafc3df0441877fa4a59cda22b4c2d4f39ce"));
        assert(genesis.hashMerkleRoot ==
               uint256S("cbdb156beade97595e5d6ff8b0ee609033030bec41851576e30c4f5a68e2cbeb"));

        // Note that of those with the service bits flag, most only support a
        // subset of possible options.
        vSeeds.push_back(CDNSSeedData("novobitcoin.org", "seed.novobitcoin.org", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 0);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 5);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        vFixedSeeds = std::vector<SeedSpec6>(
            pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = { {
                {0, uint256S("0000000000b3de1ef5bd7c20708dbafc3df0441877fa4a59cda22b4c2d4f39ce")},
                {11111, uint256S("00000000e5ab5f4cc6ae918f997fe188d906690957e1f6a30c3e28c4cf4e561f")},
                {33333, uint256S("00000000335152fea863a7e2b6320ec12e5b9d6b0bba9c4f6a9970ab6c1aa1e2")},
                {55555, uint256S("00000000224682e5cb41eb91b04c3a872f11e3216ef354a79b48aa2c4e6717aa")},
                {66666, uint256S("0000000000a56eaa524bd157ef8649e5427af2c36e902dc96a4025de25f0f110")},
            }};

        // Data as of block
        // 00000000976a33f40852fe0d843ae5dfeae7941ad8270d77fb74b4be72e6792f
        // (height 29979).
        chainTxData = ChainTxData{
            // UNIX timestamp of last known number of transactions.
            1642953295,
            // Total number of transactions between genesis and that timestamp
            // (the tx=... number in the SetBestChain debug.log lines)
            29980,
            // Estimated number of transactions per second after that timestamp.
            3};

        defaultBlockSizeParams = DefaultBlockSizeParams{
            // max block size
            MAIN_DEFAULT_MAX_BLOCK_SIZE,
            // max generated block size
            MAIN_DEFAULT_MAX_GENERATED_BLOCK_SIZE
        };

        fTestBlockCandidateValidity = false;
    }
};

/**
 * Scaling test network
 */
class CStnParams : public CChainParams
{
public:
    CStnParams()
    {
        strNetworkID = "stn";

        genesis = CreateGenesisBlock(1638457291, 0x7823b7d4, 0x1d00ffff, 1, 2000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0000000000b3de1ef5bd7c20708dbafc3df0441877fa4a59cda22b4c2d4f39ce"));

        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 150;
        // Do not allow min difficulty blocks after some time has elapsed
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        consensus.asertAnchorParams = Consensus::Params::ASERTAnchor{
          1,            // anchor block height
          0x1d00ffff,   // anchor block nBits
          1638457291,   // anchor block previous block timestamp
        };

        /**
         * The message start string is designed to be unlikely to occur in
         * normal data. The characters are rarely used upper ASCII, not valid as
         * UTF-8, and produce a large 32-bit integer with any alignment.
         */
        diskMagic[0] = 0xe3;        // stn - diskMagic - `babebee3` in little-endian
        diskMagic[1] = 0xbe;
        diskMagic[2] = 0xbe;
        diskMagic[3] = 0xba;
        netMagic[0] = 0xe3;         // stn - netMagic - `cafefee3` in little-endian
        netMagic[1] = 0xfe;
        netMagic[2] = 0xfe;
        netMagic[3] = 0xca;
        nDefaultPort = 9666;         // [STN] original port: 9333
        nPruneAfterHeight = 1000;

        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("stn.novobitcoin.org", "stn-dnsseed.novobitcoin.org", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        vFixedSeeds = std::vector<SeedSpec6>();

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = {  {
                {0, uint256S("000000000933ea01ad0ee984209779baaec3ced90fa3f408719526f8d77f4943")},
                {1, uint256S("00000000e23f9436cc8a6d6aaaa515a7b84e7a1720fc9f92805c0007c77420c4")},
                {2, uint256S("0000000040f8f40b5111d037b8b7ff69130de676327bcbd76ca0e0498a06c44a")},
                {4, uint256S("00000000d33661d5a6906f84e3c64ea6101d144ec83760bcb4ba81edcb15e68d")},
                {5, uint256S("00000000e9222ebe623bf53f6ec774619703c113242327bdc24ac830787873d6")}
        }};

        defaultBlockSizeParams = DefaultBlockSizeParams{
            // max block size
            STN_DEFAULT_MAX_BLOCK_SIZE,
            // max generated block size
            STN_DEFAULT_MAX_GENERATED_BLOCK_SIZE
        };

        fTestBlockCandidateValidity = false;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("");
        consensus.BIP66Height = 1;
        consensus.powLimit = uint256S(
            "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 150;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;

        // The half life for the ASERT DAA. For every (nASERTHalfLife) seconds behind schedule the blockchain gets,
        // difficulty is cut in half. Doubled if blocks are ahead of schedule.
        consensus.nUnsteadyASERTHalfLife = UNSTEADY_ASERT_HALF_LIFE;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        consensus.asertAnchorParams = Consensus::Params::ASERTAnchor{
          1,            // anchor block height
          0x1d00ffff,   // anchor block nBits
          1638457834,   // anchor block previous block timestamp
        };

        /**
         * The message start string is designed to be unlikely to occur in
         * normal data. The characters are rarely used upper ASCII, not valid as
         * UTF-8, and produce a large 32-bit integer with any alignment.
         */
        diskMagic[0] = 0xec;        // testnet - diskMagic - `beafbaec` in little-endian
        diskMagic[1] = 0xba;
        diskMagic[2] = 0xaf;
        diskMagic[3] = 0xbe;
        netMagic[0] = 0xeb;         // testnet - netMagic - `ceabfaeb` in little-endian
        netMagic[1] = 0xfa;
        netMagic[2] = 0xab;
        netMagic[3] = 0xce;
        nDefaultPort = 18666;       // [TestNet] original port: 18333
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1638457834, 0xaadc772a, 0x1d00ffff, 1, 2000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock ==
               uint256S("0000000000867f82407320d0939e3e618e5579156a4c0f21c067ea31edd39f49"));
        assert(genesis.hashMerkleRoot ==
               uint256S("cbdb156beade97595e5d6ff8b0ee609033030bec41851576e30c4f5a68e2cbeb"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        vSeeds.push_back(CDNSSeedData("testnet.novobitcoin.org", "testnet-dnsseed.novobitcoin.org", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
        vFixedSeeds = std::vector<SeedSpec6>(
            pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = { {
                // {546, uint256S("000000002a936ca763904c3c35fce2f3556c559c0214345"
                //                "d31b1bcebf76acb70")},
                // // UAHF fork block.
                // {1155875, uint256S("00000000f17c850672894b9a75b63a1e72830bbd5f4"
                //                    "c8889b5c1a80e7faef138")},
                // // Nov, 13. DAA activation block.
                // {1188697, uint256S("0000000000170ed0918077bde7b4d36cc4c91be69fa"
                //                    "09211f748240dabe047fb")}
            }};

        // Data as of block
        // 000000000005b07ecf85563034d13efd81c1a29e47e22b20f4fc6919d5b09cd6
        // (height 1223263)
        chainTxData = ChainTxData{1522608381, 15052068, 0.15};

        defaultBlockSizeParams = DefaultBlockSizeParams{
            // max block size
            TESTNET_DEFAULT_MAX_BLOCK_SIZE,
            // max generated block size
            TESTNET_DEFAULT_MAX_GENERATED_BLOCK_SIZE
        };

        fTestBlockCandidateValidity = false;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        // BIP34 has not activated on regtest (far in the future so block v1 are
        // not rejected in tests)
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256();
        consensus.BIP66Height = 1;
        consensus.powLimit = uint256S(
            "7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetSpacing = 150;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        /**
         * The message start string is designed to be unlikely to occur in
         * normal data. The characters are rarely used upper ASCII, not valid as
         * UTF-8, and produce a large 32-bit integer with any alignment.
         */
        diskMagic[0] = 0xe2;        // regtest - diskMagic - `babebee2` in little-endian
        diskMagic[1] = 0xbe;
        diskMagic[2] = 0xbe;
        diskMagic[3] = 0xba;
        netMagic[0] = 0xe2;         // regtest - netMagic - `cafefee2` in little-endian
        netMagic[1] = 0xfe;
        netMagic[2] = 0xfe;
        netMagic[3] = 0xca;
        nDefaultPort = 18999;       // [RegTest] original port: 18444
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1638457291, 2, 0x207fffff, 1, 2000000 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock ==
               uint256S("0693faff1ff2efb098f89871433dcc9d631929a8616fc55415268d6339f909d5"));
        assert(genesis.hashMerkleRoot ==
               uint256S("cbdb156beade97595e5d6ff8b0ee609033030bec41851576e30c4f5a68e2cbeb"));

        //!< Regtest mode doesn't have any fixed seeds.
        vFixedSeeds.clear();
        //!< Regtest mode doesn't have any DNS seeds.
        vSeeds.clear();

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = { {
            {0, uint256S("0693faff1ff2efb098f89871433dcc9d631929a8616fc55415268d6339f909d5")}
          }};

        chainTxData = ChainTxData{0, 0, 0};

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        defaultBlockSizeParams = DefaultBlockSizeParams{
            // max block size
            REGTEST_DEFAULT_MAX_BLOCK_SIZE,
            // max generated block size
            REGTEST_DEFAULT_MAX_GENERATED_BLOCK_SIZE
        };

        fTestBlockCandidateValidity = true;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

void ResetNetMagic(CChainParams& chainParam, const std::string& hexcode)
{
    if(!HexToArray(hexcode, chainParam.netMagic))
        throw std::runtime_error(strprintf("%s: Bad hex code %s.", __func__, hexcode));
}


bool HexToArray(const std::string& hexstring, CMessageHeader::MessageMagic& array){
    if(!IsHexNumber(hexstring))
        return false;

    const std::vector<uint8_t> hexVect = ParseHex(hexstring);

    if(hexVect.size()!= array.size())
        return false;

    std::copy(hexVect.begin(),hexVect.end(),array.begin());

    return true;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string &chain) {
    if (chain == CBaseChainParams::MAIN) {
        return std::unique_ptr<CChainParams>(new CMainParams());
    }

    if (chain == CBaseChainParams::TESTNET) {
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    }

    if (chain == CBaseChainParams::REGTEST) {
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    }

    if (chain == CBaseChainParams::STN) {
        return std::unique_ptr<CChainParams>(new CStnParams());
    }

    throw std::runtime_error(
        strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string &network) {
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);

    // If not mainnet, allow to set the parameter magicbytes (for testing propose)
    const bool isMagicBytesSet = gArgs.IsArgSet("-magicbytes");
    if(network != CBaseChainParams::MAIN && isMagicBytesSet){
        const std::string magicbytesStr = gArgs.GetArg("-magicbytes", "0f0f0f0f");
        LogPrintf("Manually set magicbytes [%s].\n",magicbytesStr);
        ResetNetMagic(*globalChainParams,magicbytesStr);
    }
}
