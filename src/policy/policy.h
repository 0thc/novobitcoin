// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_POLICY_POLICY_H
#define BITCOIN_POLICY_POLICY_H

#include "consensus/consensus.h"
#include "script/interpreter.h"
#include "script/standard.h"

#include <optional>
#include <string>

class Config;
class CCoinsViewCache;

namespace task{class CCancellationToken;}

/** Defaults for -excessiveblocksize and -blockmaxsize. The changed when we reach blocksize activation time.
 *
 * DEFAULT_MAX_GENERATED_BLOCK_SIZE_* represents default for -blockmaxsize,
 * which controls the maximum size of block the mining code will create
 *
 * DEFAULT_MAX_BLOCK_SIZE_* represents default for -excessiveblocksize, which specifies the
 * maximum allowed size for a block, in bytes. This is actually a consenus rule - if a node sets
 * this to a value lower than  -blockmaxsize of another node, it will start rejecting
 * big another node.
 *
 * Values bellow are also parsed by cdefs.py.
 */


/** Default max block size parameters
 */
static const uint64_t MAIN_DEFAULT_MAX_BLOCK_SIZE = 8 * ONE_MEGABYTE;
static const uint64_t REGTEST_DEFAULT_MAX_BLOCK_SIZE = 32 * ONE_MEGABYTE;
static const uint64_t TESTNET_DEFAULT_MAX_BLOCK_SIZE = 32 * ONE_MEGABYTE;
static const uint64_t STN_DEFAULT_MAX_BLOCK_SIZE = 32 * ONE_MEGABYTE;


/** Default max generated block size parameters.
 */
static const uint64_t MAIN_DEFAULT_MAX_GENERATED_BLOCK_SIZE = 8 * ONE_MEGABYTE;
static const uint64_t REGTEST_DEFAULT_MAX_GENERATED_BLOCK_SIZE = 32 * ONE_MEGABYTE;
static const uint64_t TESTNET_DEFAULT_MAX_GENERATED_BLOCK_SIZE = 32 * ONE_MEGABYTE;
static const uint64_t STN_DEFAULT_MAX_GENERATED_BLOCK_SIZE = 32 * ONE_MEGABYTE;


/** Default for -blockmintxfee, which sets the minimum feerate for a transaction
 * in blocks created by mining code **/
static const Amount DEFAULT_BLOCK_MIN_TX_FEE(8000);
/** The default size for transactions we're willing to relay/mine */
static const uint64_t DEFAULT_MAX_TX_SIZE_POLICY = 1 * ONE_MEGABYTE;
/** The default minimum input (previous output) ScriptPubKey size to output ScriptPubKey size ratio to qualify for consolidation transaction */
static const uint64_t DEFAULT_MIN_CONSOLIDATION_FACTOR = 20;
/** The default maximum size for input scriptSig in a consolidation transaction */
static const uint64_t DEFAULT_MAX_CONSOLIDATION_INPUT_SCRIPT_SIZE = 150;
/** The default minimum number of confirmations to be eligible as an input in consolidation transaction */
static const uint64_t DEFAULT_MIN_CONF_CONSOLIDATION_INPUT = 6;
/** consolidation transaction with non standard inputs */
static const bool DEFAULT_ACCEPT_NON_STD_CONSOLIDATION_INPUT = false;

/** The maximum number of sigops we're willing to relay/mine in a single tx */
static const unsigned int MAX_TX_SIGOPS_COUNT_POLICY = MAX_TX_SIGOPS_COUNT / 5;
/** Default policy value for -maxtxsigopscountspolicy, maximum number of sigops we're willing to relay/mine in a single tx */
static const unsigned int DEFAULT_TX_SIGOPS_COUNT_POLICY = MAX_TX_SIGOPS_COUNT_POLICY;

/** Default for -maxmempool, maximum megabytes of mempool memory usage */
static const unsigned int DEFAULT_MAX_MEMPOOL_SIZE = 1000;
/** Default for -maxnonfinalmempool, maximum megabytes of non-final mempool memory usage */
static const unsigned int DEFAULT_MAX_NONFINAL_MEMPOOL_SIZE = 50;
/** Minimum feerate increase for mempool limiting **/
static const CFeeRate MEMPOOL_FULL_FEE_INCREMENT(Amount(1000));
/** Default for -maxscriptsizepolicy **/
static const unsigned int DEFAULT_MAX_SCRIPT_SIZE_POLICY = 10000;
/** Default -maxmempoolsizedisk factor, maximum megabytes of total mempool disk usage as scaled -maxmempool */
static const unsigned int DEFAULT_MAX_MEMPOOL_SIZE_DISK_FACTOR = 0;
/** Default percentage of total mempool size (ram+disk) to use as max limit for CPFP transactions */
static const unsigned int DEFAULT_MEMPOOL_MAX_PERCENT_CPFP = 10;

/**
 * Min feerate for defining dust. Historically this has been the same as the
 * minRelayTxFee, however changing the dust limit changes which transactions are
 * standard and should be done with care and ideally rarely. It makes sense to
 * only increase the dust limit after prior releases were already not creating
 * outputs below the new threshold.
 * We will statically assert this to be the same value as DEFAULT_MIN_RELAY_TX_FEE
 * because of CORE-647
 */
static constexpr Amount DUST_RELAY_TX_FEE(8000);

/**
 * The threshold below which a transaction is considered dust.
 * The dust limit factor expresses this value as a multiple (in percent) of the dust relay fee
 * applied to a transaction output.
 */
static constexpr int64_t DEFAULT_DUST_LIMIT_FACTOR{300};

// Default policy value for maximum number of non-push operations per script
static const uint64_t DEFAULT_OPS_PER_SCRIPT_POLICY = UINT32_MAX;

// Default policy value for maximum number of public keys per multisig
static const uint64_t DEFAULT_PUBKEYS_PER_MULTISIG_POLICY = UINT32_MAX;

/** Maximum stack memory usage (used instead of MAX_SCRIPT_ELEMENT_SIZE and MAX_STACK_ELEMENTS). **/
static const uint64_t DEFAULT_STACK_MEMORY_USAGE_POLICY = 100 * ONE_MEGABYTE;

// Default policy value for script number length
static const uint64_t DEFAULT_SCRIPT_NUM_LENGTH_POLICY = 250 * ONE_KILOBYTE;

// Default policy value for coins cache size threshold before coins are no longer
// loaded into cache but instead returned directly to the caller.
static const uint64_t MIN_COINS_PROVIDER_CACHE_SIZE = ONE_MEGABYTE;
static const uint64_t DEFAULT_COINS_PROVIDER_CACHE_SIZE = ONE_GIGABYTE;

/**
 * Standard script verification flags that standard transactions will comply
 * with. However scripts violating these flags may still be present in valid
 * blocks and we must accept those blocks.
 */
static const unsigned int STANDARD_SCRIPT_VERIFY_FLAGS =
    MANDATORY_SCRIPT_VERIFY_FLAGS | SCRIPT_VERIFY_DERSIG |
    SCRIPT_VERIFY_MINIMALDATA | SCRIPT_VERIFY_NULLDUMMY |
    SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS | SCRIPT_VERIFY_CLEANSTACK;

/** For convenience, standard but not mandatory verify flags. */
static const unsigned int STANDARD_NOT_MANDATORY_VERIFY_FLAGS =
    STANDARD_SCRIPT_VERIFY_FLAGS & ~MANDATORY_SCRIPT_VERIFY_FLAGS;

/** returns flags for "standard" script*/
inline unsigned int StandardScriptVerifyFlags() {
    unsigned int scriptFlags = STANDARD_SCRIPT_VERIFY_FLAGS;

    scriptFlags |= SCRIPT_VERIFY_SIGPUSHONLY;
    return scriptFlags;
}

/** Consolidation transactions are free */
bool IsDustReturnTxn (const CTransaction &tx);
bool IsConsolidationTxn(const Config &config, const CTransaction &tx, const CCoinsViewCache &inputs, int32_t tipHeight);

bool IsStandard(const Config &config, const CScript &scriptPubKey, txnouttype &whichType);

/**
 * Check for standard transaction types
 * @param[in] nHeight represents the height that transactions was mined or the height that
 * we expect transcation will be mined in (in case transcation is being added to mempool)
 * @return True if all outputs (scriptPubKeys) use only standard transaction
 * forms
 */
bool IsStandardTx(const Config &config, const CTransaction &tx, std::string &reason);

/**
 * Check for standard transaction types
 * @param[in] mapInputs    Map of previous transactions that have outputs we're
 * spending
 * @return True if all inputs (scriptSigs) use only standard transaction forms
 */
std::optional<bool> AreInputsStandard(
    const task::CCancellationToken& token,
    const Config& config,
    const CTransaction& tx,
    const CCoinsViewCache &mapInputs,
    const int32_t mempoolHeight);

extern CFeeRate dustRelayFee;

#endif // BITCOIN_POLICY_POLICY_H
