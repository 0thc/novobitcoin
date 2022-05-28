// Copyright (c) 2017-2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "config.h"
#include "chainparams.h"
#include "consensus/consensus.h"
#include "validation.h"
#include "util.h"
#include "merkletree.h"

#include <boost/algorithm/string.hpp>
#include <limits>

namespace
{
    bool LessThan(
        int64_t argValue,
        std::string* err,
        const std::string& errorMessage,
        int64_t minValue)
    {
        if (argValue < minValue)
        {
            if (err)
            {
                *err = errorMessage;
            }
            return true;
        }
        return false;
    }

    bool LessThanZero(
        int64_t argValue,
        std::string* err,
        const std::string& errorMessage)
    {
        return LessThan( argValue, err, errorMessage, 0 );
    }
}

GlobalConfig::GlobalConfig() {
    Reset();
}

void GlobalConfig::Reset()
{
    feePerKB = CFeeRate {};
    dustLimitFactor = DEFAULT_DUST_LIMIT_FACTOR;
    blockMinFeePerKB = CFeeRate{DEFAULT_BLOCK_MIN_TX_FEE};
    preferredBlockFileSize = DEFAULT_PREFERRED_BLOCKFILE_SIZE;
    factorMaxSendQueuesBytes = DEFAULT_FACTOR_MAX_SEND_QUEUES_BYTES;

    setDefaultBlockSizeParamsCalled = false;

    maxBlockSize = 0;
    defaultBlockSize = 0;
    maxGeneratedBlockSize = 0;
    maxTxSizePolicy = DEFAULT_MAX_TX_SIZE_POLICY;
    minConsolidationFactor = DEFAULT_MIN_CONSOLIDATION_FACTOR;
    maxConsolidationInputScriptSize = DEFAULT_MAX_CONSOLIDATION_INPUT_SCRIPT_SIZE;
    minConfConsolidationInput = DEFAULT_MIN_CONF_CONSOLIDATION_INPUT;
    acceptNonStdConsolidationInput = DEFAULT_ACCEPT_NON_STD_CONSOLIDATION_INPUT;

    dataCarrierSize = DEFAULT_DATA_CARRIER_SIZE;
    limitAncestorCount = DEFAULT_ANCESTOR_LIMIT;
    limitSecondaryMempoolAncestorCount = DEFAULT_SECONDARY_MEMPOOL_ANCESTOR_LIMIT;

    testBlockCandidateValidity = false;
    blockAssemblerType = mining::DEFAULT_BLOCK_ASSEMBLER_TYPE;

    mMaxConcurrentAsyncTasksPerNode = DEFAULT_NODE_ASYNC_TASKS_LIMIT;

    mMaxParallelBlocks = DEFAULT_SCRIPT_CHECK_POOL_SIZE;
    mPerBlockScriptValidatorThreadsCount = DEFAULT_SCRIPTCHECK_THREADS;
    mPerBlockScriptValidationMaxBatchSize = DEFAULT_SCRIPT_CHECK_MAX_BATCH_SIZE;
    maxOpsPerScriptPolicy = DEFAULT_OPS_PER_SCRIPT_POLICY;
    maxTxSigOpsCountPolicy = DEFAULT_TX_SIGOPS_COUNT_POLICY;
    maxPubKeysPerMultiSig = DEFAULT_PUBKEYS_PER_MULTISIG_POLICY;

    mMaxStdTxnValidationDuration = DEFAULT_MAX_STD_TXN_VALIDATION_DURATION;
    mMaxNonStdTxnValidationDuration = DEFAULT_MAX_NON_STD_TXN_VALIDATION_DURATION;

    maxStackMemoryUsagePolicy = DEFAULT_STACK_MEMORY_USAGE_POLICY;
    maxStackMemoryUsageConsensus = DEFAULT_STACK_MEMORY_USAGE_CONSENSUS;
    maxScriptSizePolicy = DEFAULT_MAX_SCRIPT_SIZE_POLICY;

    maxScriptNumLengthPolicy = DEFAULT_SCRIPT_NUM_LENGTH_POLICY;

    mMaxCoinsViewCacheSize = 0;
    mMaxCoinsProviderCacheSize = DEFAULT_COINS_PROVIDER_CACHE_SIZE;

    maxProtocolRecvPayloadLength = DEFAULT_MAX_PROTOCOL_RECV_PAYLOAD_LENGTH;
    maxProtocolSendPayloadLength = DEFAULT_MAX_PROTOCOL_RECV_PAYLOAD_LENGTH * MAX_PROTOCOL_SEND_PAYLOAD_FACTOR;

    recvInvQueueFactor = DEFAULT_RECV_INV_QUEUE_FACTOR;

    mMaxMempool = DEFAULT_MAX_MEMPOOL_SIZE * ONE_MEGABYTE;
    mMaxMempoolSizeDisk = mMaxMempool * DEFAULT_MAX_MEMPOOL_SIZE_DISK_FACTOR;
    mMempoolMaxPercentCPFP = DEFAULT_MEMPOOL_MAX_PERCENT_CPFP;
    mMemPoolExpiry = DEFAULT_MEMPOOL_EXPIRY * SECONDS_IN_ONE_HOUR;
    mMaxOrphanTxSize = COrphanTxns::DEFAULT_MAX_ORPHAN_TRANSACTIONS_SIZE;
    mMaxPercentageOfOrphansInMaxBatchSize = COrphanTxns::DEFAULT_MAX_PERCENTAGE_OF_ORPHANS_IN_BATCH;
    mMaxInputsForSecondLayerOrphan = COrphanTxns::DEFAULT_MAX_INPUTS_OUTPUTS_PER_TRANSACTION;
    mStopAtHeight = DEFAULT_STOPATHEIGHT;

    invalidTxFileSinkSize = CInvalidTxnPublisher::DEFAULT_FILE_SINK_DISK_USAGE;
    invalidTxFileSinkEvictionPolicy = CInvalidTxnPublisher::DEFAULT_FILE_SINK_EVICTION_POLICY;

    // Block download
    blockStallingMinDownloadSpeed = DEFAULT_MIN_BLOCK_STALLING_RATE;
    blockStallingTimeout = DEFAULT_BLOCK_STALLING_TIMEOUT;
    blockDownloadWindow = DEFAULT_BLOCK_DOWNLOAD_WINDOW;
    blockDownloadSlowFetchTimeout = DEFAULT_BLOCK_DOWNLOAD_SLOW_FETCH_TIMEOUT;
    blockDownloadMaxParallelFetch = DEFAULT_MAX_BLOCK_PARALLEL_FETCH;

    // P2P parameters
    p2pHandshakeTimeout = DEFAULT_P2P_HANDSHAKE_TIMEOUT_INTERVAL;
    streamSendRateLimit = Stream::DEFAULT_SEND_RATE_LIMIT;
    banScoreThreshold = DEFAULT_BANSCORE_THRESHOLD;

#if ENABLE_ZMQ
    invalidTxZMQMaxMessageSize = CInvalidTxnPublisher::DEFAULT_ZMQ_SINK_MAX_MESSAGE_SIZE;
#endif

    maxMerkleTreeDiskSpace = MIN_DISK_SPACE_FOR_MERKLETREE_FILES;
    preferredMerkleTreeFileSize = DEFAULT_PREFERRED_MERKLETREE_FILE_SIZE;
    maxMerkleTreeMemoryCacheSize = DEFAULT_MAX_MERKLETREE_MEMORY_CACHE_SIZE;

}

void GlobalConfig::SetPreferredBlockFileSize(uint64_t preferredSize) {
    preferredBlockFileSize = preferredSize;
}

uint64_t GlobalConfig::GetPreferredBlockFileSize() const {
    return preferredBlockFileSize;
}

void GlobalConfig::SetDefaultBlockSizeParams(const DefaultBlockSizeParams &params) {
    maxBlockSize = params.maxBlockSize;
    defaultBlockSize = maxBlockSize;
    maxGeneratedBlockSize = params.maxGeneratedBlockSize;
    setDefaultBlockSizeParamsCalled = true;
}

void GlobalConfig::CheckSetDefaultCalled() const
{
    if (!setDefaultBlockSizeParamsCalled)
    {
        // If you hit this we created new instance of GlobalConfig without
        // setting defaults
        throw std::runtime_error(
            "GlobalConfig::SetDefaultBlockSizeParams must be called before accessing block size related parameters");
    }
}

bool GlobalConfig::SetMaxBlockSize(uint64_t maxSize, std::string* err) {
    std::scoped_lock<std::shared_mutex> lock{configMtx};
    // Do not allow maxBlockSize to be set below historic 1MB limit
    // It cannot be equal either because of the "must be big" UAHF rule.
    if (maxSize && maxSize <= LEGACY_MAX_BLOCK_SIZE) {
        if (err)
            *err = _("Excessive block size (excessiveblocksize) must be larger than ") + std::to_string(LEGACY_MAX_BLOCK_SIZE);
        return false;
    }

    // Unlimited value depends on each definition of CChainParams
    maxBlockSize = maxSize ? maxSize : defaultBlockSize;

    return true;
}

uint64_t GlobalConfig::GetMaxBlockSize() const {
    std::shared_lock<std::shared_mutex> lock{configMtx};
    CheckSetDefaultCalled();
    return maxBlockSize;
}

void GlobalConfig::SetFactorMaxSendQueuesBytes(uint64_t factorMaxSendQueuesBytesIn) {
    factorMaxSendQueuesBytes = factorMaxSendQueuesBytesIn;
}

uint64_t GlobalConfig::GetFactorMaxSendQueuesBytes() const {
    return factorMaxSendQueuesBytes;
}

uint64_t GlobalConfig::GetMaxSendQueuesBytes() const {

    // Use the "after upgrade" excessive block size to determine the maximum size of
    // block related messages that we are prepared to queue
    uint64_t maxBlockSize = GetMaxBlockSize();
    if (factorMaxSendQueuesBytes > UINT64_MAX / maxBlockSize)
    {
        return UINT64_MAX;
    }
    return factorMaxSendQueuesBytes * maxBlockSize;
}

bool GlobalConfig::SetMaxGeneratedBlockSize(uint64_t maxSize, std::string* err) {
    std::scoped_lock<std::shared_mutex> lock{configMtx};
    maxGeneratedBlockSize = maxSize;
    return true;
}

uint64_t GlobalConfig::GetMaxGeneratedBlockSize() const {
    std::shared_lock<std::shared_mutex> lock{configMtx};
    CheckSetDefaultCalled();
    return maxGeneratedBlockSize;
}

bool GlobalConfig::SetMaxTxSizePolicy(int64_t maxTxSizePolicyIn, std::string* err)
{
    if (LessThanZero(maxTxSizePolicyIn, err, "Policy value for max tx size must not be less than 0"))
    {
        return false;
    }
    if (maxTxSizePolicyIn == 0)
    {
        maxTxSizePolicy = MAX_TX_SIZE_CONSENSUS;
        return true;
    }
    uint64_t maxTxSizePolicyInUnsigned = static_cast<uint64_t>(maxTxSizePolicyIn);
    if (maxTxSizePolicyInUnsigned > MAX_TX_SIZE_CONSENSUS)
    {
        if (err)
        {
            *err = "Policy value for max tx size must not exceed consensus limit of " + std::to_string(MAX_TX_SIZE_CONSENSUS);
        }
        return false;
    }
    else if (maxTxSizePolicyInUnsigned < DEFAULT_MAX_TX_SIZE_POLICY)
    {
        if (err)
        {
            *err = "Policy value for max tx size must not be less than " + std::to_string(DEFAULT_MAX_TX_SIZE_POLICY);
        }
        return false;
    }

    maxTxSizePolicy = maxTxSizePolicyInUnsigned;
    return true;
}

uint64_t GlobalConfig::GetMaxTxSize(bool isConsensus) const
{
    if (isConsensus)
    {
        return MAX_TX_SIZE_CONSENSUS;
    }
    return maxTxSizePolicy;
}

bool GlobalConfig::SetMinConsolidationFactor(int64_t minConsolidationFactorIn, std::string* err)
{
    if (LessThanZero(minConsolidationFactorIn, err, "Minimum consolidation factor cannot be less than zero."))
    {
        return false;
    }
    minConsolidationFactor = static_cast<uint64_t>(minConsolidationFactorIn);
    return true;
}

uint64_t GlobalConfig::GetMinConsolidationFactor() const
{
    return minConsolidationFactor;
}

bool GlobalConfig::SetMaxConsolidationInputScriptSize(int64_t maxConsolidationInputScriptSizeIn, std::string* err)
{
    if (LessThanZero(maxConsolidationInputScriptSizeIn, err, "Maximum length for a scriptSig input in a consolidation txn cannot be less than zero."))
    {
        return false;
    }
    else if (maxConsolidationInputScriptSizeIn == 0) {
        maxConsolidationInputScriptSize = DEFAULT_MAX_CONSOLIDATION_INPUT_SCRIPT_SIZE;
    } else {
        maxConsolidationInputScriptSize = static_cast<uint64_t>(maxConsolidationInputScriptSizeIn);
    }
    return true;
}

uint64_t GlobalConfig::GetMaxConsolidationInputScriptSize() const
{
    return maxConsolidationInputScriptSize;
}

bool GlobalConfig::SetMinConfConsolidationInput(int64_t minconfIn, std::string* err)
{
    if (LessThanZero(minconfIn, err, "Minimum number of confirmations of inputs spent by consolidation transactions cannot be less than 0"))
    {
        return false;
    }
    if (minconfIn == 0)
    {
        minConfConsolidationInput = DEFAULT_MIN_CONF_CONSOLIDATION_INPUT;
    }
    else
    {
        minConfConsolidationInput = static_cast<uint64_t>(minconfIn);
    }
    return true;
}

uint64_t GlobalConfig::GetMinConfConsolidationInput() const
{
    return minConfConsolidationInput;
}

bool GlobalConfig::SetAcceptNonStdConsolidationInput(bool flagValue, std::string* err)
{
    acceptNonStdConsolidationInput = flagValue;
    return true;
}

bool GlobalConfig::GetAcceptNonStdConsolidationInput() const
{
    return acceptNonStdConsolidationInput;
}

void GlobalConfig::SetDataCarrierSize(uint64_t dataCarrierSizeIn) {
    dataCarrierSize = dataCarrierSizeIn;
}

uint64_t GlobalConfig::GetDataCarrierSize() const {
    return dataCarrierSize;
}

bool  GlobalConfig::SetLimitAncestorCount(int64_t limitAncestorCountIn, std::string* err) {
    if (limitAncestorCountIn <= 0)
    {
        if (err)
        {
            *err = "The maximal number of the in-mempool ancestors must be greater than 0.";
        }
        return false;
    }
    limitAncestorCount = static_cast<uint64_t>(limitAncestorCountIn);
    return true;
}

uint64_t GlobalConfig::GetLimitAncestorCount() const {
    return limitAncestorCount;
}

bool GlobalConfig::SetLimitSecondaryMempoolAncestorCount(int64_t limitSecondaryMempoolAncestorCountIn, std::string* err) {
    if (limitSecondaryMempoolAncestorCountIn <= 1)
    {
        if (err)
        {
            *err = "The maximal number of the CPFP group members must be greater than 1.";
        }
        return false;
    }
    limitSecondaryMempoolAncestorCount = static_cast<uint64_t>(limitSecondaryMempoolAncestorCountIn);
    return true;
}

uint64_t GlobalConfig::GetLimitSecondaryMempoolAncestorCount()const {
    return limitSecondaryMempoolAncestorCount;
}

const CChainParams &GlobalConfig::GetChainParams() const {
    return Params();
}

bool GlobalConfig::SetMaxPubKeysPerMultiSigPolicy(int64_t maxPubKeysPerMultiSigIn, std::string* err)
{
    if (LessThanZero(maxPubKeysPerMultiSigIn, err, "Policy value for maximum public keys per multisig must not be less than zero"))
    {
        return false;
    }

    uint64_t maxPubKeysPerMultiSigUnsigned = static_cast<uint64_t>(maxPubKeysPerMultiSigIn);
    if (maxPubKeysPerMultiSigUnsigned > MAX_PUBKEYS_PER_MULTISIG)
    {
        if (err)
        {
            *err = "Policy value for maximum public keys per multisig must not exceed consensus limit of " + std::to_string(MAX_PUBKEYS_PER_MULTISIG) + ".";
        }
        return false;
    }
    else if (maxPubKeysPerMultiSigUnsigned == 0)
    {
        maxPubKeysPerMultiSig = MAX_PUBKEYS_PER_MULTISIG;
    }
    else
    {
        maxPubKeysPerMultiSig = maxPubKeysPerMultiSigUnsigned;
    }

    return true;
}

uint64_t GlobalConfig::GetMaxPubKeysPerMultiSig(bool consensus) const
{
    if (consensus)
    {
        return MAX_PUBKEYS_PER_MULTISIG;
    }

    return maxPubKeysPerMultiSig;
}

Config& GlobalConfig::GetConfig()
{
    static GlobalConfig config {};
    return config;
}

ConfigInit& GlobalConfig::GetModifiableGlobalConfig()
{
    static Config& config = GlobalConfig::GetConfig();
    return static_cast<ConfigInit&>(config);
}

void GlobalConfig::SetTestBlockCandidateValidity(bool test) {
    testBlockCandidateValidity = test;
}

bool GlobalConfig::GetTestBlockCandidateValidity() const {
    return testBlockCandidateValidity;
}

void GlobalConfig::SetMiningCandidateBuilder(mining::CMiningFactory::BlockAssemblerType type) {
    blockAssemblerType = type;
}

mining::CMiningFactory::BlockAssemblerType GlobalConfig::GetMiningCandidateBuilder() const {
    return blockAssemblerType;
}

bool GlobalConfig::SetMaxConcurrentAsyncTasksPerNode(
    int maxConcurrentAsyncTasksPerNode,
    std::string* error)
{
    if (maxConcurrentAsyncTasksPerNode < 1
        || maxConcurrentAsyncTasksPerNode > mMaxParallelBlocks)
    {
        if(error)
        {
            *error =
                strprintf(
                _("Max parallel tasks per node count must be at least 1 and at most"
                    " maxParallelBlocks"));
        }

        return false;
    }

    mMaxConcurrentAsyncTasksPerNode = maxConcurrentAsyncTasksPerNode;

    return true;
}

int GlobalConfig::GetMaxConcurrentAsyncTasksPerNode() const
{
    return mMaxConcurrentAsyncTasksPerNode;
}

bool GlobalConfig::SetBlockScriptValidatorsParams(
    int maxParallelBlocks,
    int perValidatorThreadsCount,
    int perValidatorThreadMaxBatchSize,
    std::string* error)
{
    {
        constexpr int max = 100;
        if (maxParallelBlocks < 1 || maxParallelBlocks > max)
        {
            if(error)
            {
                *error =
                    strprintf(
                    _("Max parallel blocks count must be at least 1 and at most %d"),
                    max);
            }

            return false;
        }

        mMaxParallelBlocks = maxParallelBlocks;

        // limit dependent variable
        mMaxConcurrentAsyncTasksPerNode =
            std::min(mMaxConcurrentAsyncTasksPerNode, mMaxParallelBlocks);
    }

    {
        // perValidatorThreadsCount==0 means autodetect,
        // but nScriptCheckThreads==0 means no concurrency
        if (perValidatorThreadsCount == 0)
        {
            perValidatorThreadsCount =
                std::clamp(GetNumCores(), 0, MAX_SCRIPTCHECK_THREADS);
        }
        else if (perValidatorThreadsCount < 0
            || perValidatorThreadsCount > MAX_SCRIPTCHECK_THREADS)
        {
            if(error)
            {
                *error =
                    strprintf(
                        _("Per block script validation threads count must be at "
                          "least 0 and at most %d"), MAX_SCRIPTCHECK_THREADS);
            }

            return false;
        }

        mPerBlockScriptValidatorThreadsCount = perValidatorThreadsCount;
    }

    {
        if (perValidatorThreadMaxBatchSize < 1
            || perValidatorThreadMaxBatchSize > std::numeric_limits<uint8_t>::max())
        {
            if(error)
            {
                *error =
                    strprintf(
                        _("Per block script validation max batch size must be at "
                          "least 1 and at most %d"),
                        std::numeric_limits<uint8_t>::max());
            }

            return false;
        }
        mPerBlockScriptValidationMaxBatchSize = perValidatorThreadMaxBatchSize;
    }

    return true;
}

int GlobalConfig::GetMaxParallelBlocks() const
{
    return mMaxParallelBlocks;
}

int GlobalConfig::GetPerBlockScriptValidatorThreadsCount() const
{
    return mPerBlockScriptValidatorThreadsCount;
}

int GlobalConfig::GetPerBlockScriptValidationMaxBatchSize() const
{
    return mPerBlockScriptValidationMaxBatchSize;
}

bool GlobalConfig::SetMaxOpsPerScriptPolicy(int64_t maxOpsPerScriptPolicyIn, std::string* error)
{
    if (LessThanZero(maxOpsPerScriptPolicyIn, error, "Policy value for MaxOpsPerScript cannot be less than zero."))
    {
        return false;
    }
    uint64_t maxOpsPerScriptPolicyInUnsigned = static_cast<uint64_t>(maxOpsPerScriptPolicyIn);

    if (maxOpsPerScriptPolicyInUnsigned > MAX_OPS_PER_SCRIPT)
    {
        if (error)
        {
            *error = "Policy value for MaxOpsPerScript must not exceed consensus limit of " + std::to_string(MAX_OPS_PER_SCRIPT) + ".";
        }
        return false;
    }
    else if (maxOpsPerScriptPolicyInUnsigned == 0)
    {
        maxOpsPerScriptPolicy = MAX_OPS_PER_SCRIPT;
    }
    else
    {
        maxOpsPerScriptPolicy = maxOpsPerScriptPolicyInUnsigned;
    }

    return true;
}

uint64_t GlobalConfig::GetMaxOpsPerScript(bool consensus) const
{
    if (consensus)
    {
        return MAX_OPS_PER_SCRIPT;
    }
    return maxOpsPerScriptPolicy;
}

bool GlobalConfig::SetMaxStdTxnValidationDuration(int ms, std::string* err)
{
    if(ms < 1)
    {
        if(err)
        {
            *err =
                strprintf(
                    _("Per transaction max validation duration must be at least 1ms"));
        }

        return false;
    }

    mMaxStdTxnValidationDuration = std::chrono::milliseconds{ms};

    return true;
}

std::chrono::milliseconds GlobalConfig::GetMaxStdTxnValidationDuration() const
{
    return mMaxStdTxnValidationDuration;
}

bool GlobalConfig::SetMaxNonStdTxnValidationDuration(int ms, std::string* err)
{
    if(ms < 10)
    {
        if(err)
        {
            *err =
                strprintf(
                    _("Per transaction max validation duration must be at least 10ms"));
        }

        return false;
    }

    mMaxNonStdTxnValidationDuration = std::chrono::milliseconds{ms};

    return true;
}

std::chrono::milliseconds GlobalConfig::GetMaxNonStdTxnValidationDuration() const
{
    return mMaxNonStdTxnValidationDuration;
}

bool GlobalConfig::SetMaxTxnChainValidationBudget(int ms, std::string* err)
{
    if(LessThanZero(ms, err, "Per chain max validation duration budget must be non-negative"))
    {

        return false;
    }

    mMaxTxnChainValidationBudget = std::chrono::milliseconds{ms};

    return true;
}

std::chrono::milliseconds GlobalConfig::GetMaxTxnChainValidationBudget() const {
    return mMaxTxnChainValidationBudget;
}

void GlobalConfig::SetValidationClockCPU(bool enable) {
    mValidationClockCPU = enable;
}

bool GlobalConfig::GetValidationClockCPU() const {
    return mValidationClockCPU;
}


/**
 * Compute the maximum number of sigops operations that can be contained in a block
 * given the block size as parameter. It is computed by multiplying the upper sigops limit
 * MAX_BLOCK_SIGOPS_PER_MB by the size of the block in MB rounded up to the
 * closest integer.
 */

uint64_t GlobalConfig::GetMaxBlockSigOpsConsensus(uint64_t blockSize) const
{
    auto nMbRoundedUp = 1 + ((blockSize - 1) / ONE_MEGABYTE);
    return nMbRoundedUp * MAX_BLOCK_SIGOPS_PER_MB;
}

bool GlobalConfig::SetMaxStackMemoryUsage(int64_t maxStackMemoryUsageConsensusIn, int64_t maxStackMemoryUsagePolicyIn, std::string* err)
{
    if (maxStackMemoryUsageConsensusIn < 0 || maxStackMemoryUsagePolicyIn < 0)
    {
        if (err)
        {
            *err = "Policy and consensus value for max stack memory usage must not be less than 0.";
        }
        return false;
    }

    if (maxStackMemoryUsageConsensusIn == 0)
    {
        maxStackMemoryUsageConsensus = DEFAULT_STACK_MEMORY_USAGE_CONSENSUS;
    }
    else
    {
        maxStackMemoryUsageConsensus = static_cast<uint64_t>(maxStackMemoryUsageConsensusIn);
    }

    if (maxStackMemoryUsagePolicyIn == 0)
    {
        maxStackMemoryUsagePolicy = DEFAULT_STACK_MEMORY_USAGE_CONSENSUS;
    }
    else
    {
        maxStackMemoryUsagePolicy = static_cast<uint64_t>(maxStackMemoryUsagePolicyIn);
    }

    if (maxStackMemoryUsagePolicy > maxStackMemoryUsageConsensus)
    {
        if (err)
        {
            *err = _("Policy value of max stack memory usage must not exceed consensus limit of ") + std::to_string(maxStackMemoryUsageConsensus);
        }
        return false;
    }

    return true;
}

uint64_t GlobalConfig::GetMaxStackMemoryUsage(bool consensus) const
{
    if (consensus)
    {
        return maxStackMemoryUsageConsensus;
    }

    return maxStackMemoryUsagePolicy;
}

bool GlobalConfig::SetMaxScriptNumLengthPolicy(int64_t maxScriptNumLengthIn, std::string* err)
{
    if (LessThanZero(maxScriptNumLengthIn, err, "Policy value for maximum script number length must not be less than 0."))
    {
        return false;
    }

    uint64_t maxScriptNumLengthUnsigned = static_cast<uint64_t>(maxScriptNumLengthIn);
    if (maxScriptNumLengthUnsigned > MAX_SCRIPT_NUM_LENGTH)
    {
        if (err)
        {
            *err = "Policy value for maximum script number length must not exceed consensus limit of " + std::to_string(MAX_SCRIPT_NUM_LENGTH) + ".";
        }
        return false;
    }
    else if (maxScriptNumLengthUnsigned == 0)
    {
        maxScriptNumLengthPolicy = MAX_SCRIPT_NUM_LENGTH;
    }
    else if (maxScriptNumLengthUnsigned < MIN_SCRIPT_NUM_LENGTH)
    {
        if (err)
        {
            *err = "Policy value for maximum script number length must not be less than " + std::to_string(MIN_SCRIPT_NUM_LENGTH) + ".";
        }
        return false;
    }
    else
    {
        maxScriptNumLengthPolicy = maxScriptNumLengthUnsigned;
    }

    return true;
}

uint64_t GlobalConfig::GetMaxScriptNumLength(bool isConsensus) const
{
    if (isConsensus)
    {
        return MAX_SCRIPT_NUM_LENGTH;
    }
    return maxScriptNumLengthPolicy;
}

bool GlobalConfig::SetMaxCoinsViewCacheSize(int64_t max, std::string* err)
{
    if (LessThanZero(max, err, "Policy value for maximum coins view cache size must not be less than 0."))
    {
        return false;
    }

    mMaxCoinsViewCacheSize = static_cast<uint64_t>(max);

    return true;
}

bool GlobalConfig::SetMaxCoinsProviderCacheSize(int64_t max, std::string* err)
{
    static_assert( MIN_COINS_PROVIDER_CACHE_SIZE <= std::numeric_limits<uint64_t>::max() );

    if (LessThan(
            max,
            err,
            "Policy value for maximum coins provider cache size must not be less than "
                + std::to_string(MIN_COINS_PROVIDER_CACHE_SIZE),
            MIN_COINS_PROVIDER_CACHE_SIZE))
    {
        return false;
    }

    mMaxCoinsProviderCacheSize = static_cast<uint64_t>(max);

    return true;
}

bool GlobalConfig::SetMaxCoinsDbOpenFiles(int64_t max, std::string* err)
{
    if (LessThanZero(max - 1, err, "Minimum value for max number of leveldb open files for coinsdb size must not be less than 1."))
    {
        return false;
    }

    mMaxCoinsDbOpenFiles = static_cast<uint64_t>(max);

    return true;
}

void GlobalConfig::SetInvalidBlocks(const std::set<uint256>& hashes)
{
    mInvalidBlocks = hashes;
}

const std::set<uint256>& GlobalConfig::GetInvalidBlocks() const
{
    return mInvalidBlocks;
}

bool GlobalConfig::IsBlockInvalidated(const uint256& hash) const
{
    return mInvalidBlocks.find(hash) != mInvalidBlocks.end();
}

void GlobalConfig::SetBanClientUA(const std::set<std::string> uaClients)
{
    mBannedUAClients = uaClients;
}

bool GlobalConfig::IsClientUABanned(const std::string uaClient) const
{
    for (std::string invUAClient : mBannedUAClients)
    {
        if (boost::icontains(uaClient, invUAClient))
        {
            return true;
        }
    }
    return false;
}

bool GlobalConfig::SetMaxMerkleTreeDiskSpace(int64_t maxDiskSpace, std::string* err)
{
    if (LessThanZero(maxDiskSpace, err, "Maximum disk space taken by merkle tree files cannot be configured with a negative value."))
    {
        return false;
    }
    uint64_t setMaxDiskSpace = static_cast<uint64_t>(maxDiskSpace);
    if (setMaxDiskSpace < MIN_DISK_SPACE_FOR_MERKLETREE_FILES)
    {
        if (err)
        {
            *err = _("Maximum disk space used by merkle tree files cannot be below the minimum of ") +
                std::to_string(MIN_DISK_SPACE_FOR_MERKLETREE_FILES / ONE_MEBIBYTE) + _(" MiB.");
        }
        return false;
    }
    maxMerkleTreeDiskSpace = setMaxDiskSpace;
    return true;
}

uint64_t GlobalConfig::GetMaxMerkleTreeDiskSpace() const
{
    return maxMerkleTreeDiskSpace;
}

bool GlobalConfig::SetPreferredMerkleTreeFileSize(int64_t preferredFileSize, std::string* err)
{
    if (LessThanZero(preferredFileSize, err, "Merkle tree file size cannot be configured with a negative value."))
    {
        return false;
    }
    preferredMerkleTreeFileSize = static_cast<uint64_t>(preferredFileSize);
    return true;
}

uint64_t GlobalConfig::GetPreferredMerkleTreeFileSize() const
{
    return preferredMerkleTreeFileSize;
}

bool GlobalConfig::AddInvalidTxSink(const std::string& sink, std::string* err)
{
    auto availableSinks = GetAvailableInvalidTxSinks();
    if (availableSinks.find(sink) == availableSinks.end())
    {
        if (err)
        {
            *err =sink + " is not valid transaction sink. Valid transactions sinks are: ";
            *err += StringJoin(", ", availableSinks.begin(), availableSinks.end());
        }
        return false;
    }
    invalidTxSinks.insert(sink);
    return true;
}

std::set<std::string> GlobalConfig::GetInvalidTxSinks() const
{
    return invalidTxSinks;
}

std::set<std::string> GlobalConfig::GetAvailableInvalidTxSinks() const
{
#if ENABLE_ZMQ
    return {"FILE", "ZMQ"};
#else
    return {"FILE"};
#endif
}

bool GlobalConfig::SetInvalidTxFileSinkMaxDiskUsage(int64_t max, std::string* err)
{
    if (LessThanZero(max, err, "Invalid transaction file usage can not be negative."))
    {
        return false;
    }

    invalidTxFileSinkSize = (max == 0 ? std::numeric_limits<int64_t>::max() : max);
    return true;
}

int64_t GlobalConfig::GetInvalidTxFileSinkMaxDiskUsage() const
{
    return invalidTxFileSinkSize;
}

bool GlobalConfig::SetInvalidTxFileSinkEvictionPolicy(std::string policy, std::string* err)
{
    if(policy == "IGNORE_NEW")
    {
        invalidTxFileSinkEvictionPolicy = InvalidTxEvictionPolicy::IGNORE_NEW;
        return true;
    }
    else if (policy == "DELETE_OLD")
    {
        invalidTxFileSinkEvictionPolicy = InvalidTxEvictionPolicy::DELETE_OLD;
        return true;
    }

    if (err)
    {
        *err = "Invalid value for invalid transactions eviction policy. Available policies are IGNORE_NEW and DELETE_OLD. Got " + policy;
    }

    return false;
}

InvalidTxEvictionPolicy GlobalConfig::GetInvalidTxFileSinkEvictionPolicy() const
{
    return invalidTxFileSinkEvictionPolicy;
}

// Block download
bool GlobalConfig::SetBlockStallingMinDownloadSpeed(int64_t min, std::string* err)
{
    if(min < 0)
    {
        if(err)
        {
            *err = "Block stalling minimum download speed must be >= 0";
        }
        return false;
    }

    blockStallingMinDownloadSpeed = min;
    return true;
}
uint64_t GlobalConfig::GetBlockStallingMinDownloadSpeed() const
{
    return blockStallingMinDownloadSpeed;
}

bool GlobalConfig::SetBlockStallingTimeout(int64_t timeout, std::string* err)
{
    if(timeout <= 0)
    {
        if(err)
        {
            *err = "Block stalling timeout must be greater than 0.";
        }
        return false;
    }

    blockStallingTimeout = timeout;
    return true;
}
int64_t GlobalConfig::GetBlockStallingTimeout() const
{
    return blockStallingTimeout;
}

bool GlobalConfig::SetBlockDownloadWindow(int64_t window, std::string* err)
{
    if(window <= 0)
    {
        if(err)
        {
            *err = "Block download window must be greater than 0.";
        }
        return false;
    }

    blockDownloadWindow = window;
    return true;
}
int64_t GlobalConfig::GetBlockDownloadWindow() const
{
    return blockDownloadWindow;
}

bool GlobalConfig::SetBlockDownloadSlowFetchTimeout(int64_t timeout, std::string* err)
{
    if(timeout <= 0)
    {
        if(err)
        {
            *err = "Block download slow fetch timeout must be greater than 0.";
        }
        return false;
    }

    blockDownloadSlowFetchTimeout = timeout;
    return true;
}
int64_t GlobalConfig::GetBlockDownloadSlowFetchTimeout() const
{
    return blockDownloadSlowFetchTimeout;
}

bool GlobalConfig::SetBlockDownloadMaxParallelFetch(int64_t max, std::string* err)
{
    if(max <= 0)
    {
        if(err)
        {
            *err = "Block download maximum parallel fetch must be greater than 0.";
        }
        return false;
    }

    blockDownloadMaxParallelFetch = max;
    return true;
}
uint64_t GlobalConfig::GetBlockDownloadMaxParallelFetch() const
{
    return blockDownloadMaxParallelFetch;
}

// P2P Parameters
bool GlobalConfig::SetP2PHandshakeTimeout(int64_t timeout, std::string* err)
{
    if(timeout <= 0)
    {
        if(err)
        {
            *err = "P2P handshake timeout must be greater than 0.";
        }
        return false;
    }

    p2pHandshakeTimeout = timeout;
    return true;
}

bool GlobalConfig::SetStreamSendRateLimit(int64_t limit, std::string* err)
{
    streamSendRateLimit = limit;
    return true;
}
int64_t GlobalConfig::GetStreamSendRateLimit() const
{
    return streamSendRateLimit;
}

bool GlobalConfig::SetBanScoreThreshold(int64_t threshold, std::string* err)
{
    auto maxThreshold { std::numeric_limits<decltype(banScoreThreshold)>::max() };
    if(threshold <= 0 || threshold > maxThreshold)
    {
        if(err)
        {
            *err = "Ban score threshold must be greater than 0 and less then " + std::to_string(maxThreshold);
        }
        return false;
    }

    banScoreThreshold = static_cast<decltype(banScoreThreshold)>(threshold);
    return true;
}

unsigned int GlobalConfig::GetBanScoreThreshold() const
{
    return banScoreThreshold;
}

#if ENABLE_ZMQ
bool GlobalConfig::SetInvalidTxZMQMaxMessageSize(int64_t max, std::string* err)
{
    if (LessThanZero(max, err, "Invalid transaction ZMQ max message size can not be negative."))
    {
        return false;
    }

    invalidTxZMQMaxMessageSize = (max == 0 ? std::numeric_limits<int64_t>::max() : max);
    return true;
}

int64_t GlobalConfig::GetInvalidTxZMQMaxMessageSize() const
{
    return invalidTxZMQMaxMessageSize;
}
#endif

bool GlobalConfig::SetMaxMerkleTreeMemoryCacheSize(int64_t maxMemoryCacheSize, std::string* err)
{
    if (LessThanZero(maxMemoryCacheSize, err, "Maximum merkle tree memory cache size cannot be configured with a negative value."))
    {
        return false;
    }

    maxMerkleTreeMemoryCacheSize = static_cast<uint64_t>(maxMemoryCacheSize);
    return true;
}

uint64_t GlobalConfig::GetMaxMerkleTreeMemoryCacheSize() const
{
    return maxMerkleTreeMemoryCacheSize;
}

bool GlobalConfig::SetMaxProtocolRecvPayloadLength(uint64_t value, std::string* err)
{
    // sending maxRecvPayloadLength less than LEGACY_MAX_PROTOCOL_PAYLOAD_LENGTH is considered protocol violation
    if (value < LEGACY_MAX_PROTOCOL_PAYLOAD_LENGTH)
    {
        if (err)
        {
            *err = "MaxProtocolRecvPayloadLength should be at least: " + std::to_string(LEGACY_MAX_PROTOCOL_PAYLOAD_LENGTH) + ".";
        }
        return false;
    }

    if (value > MAX_PROTOCOL_RECV_PAYLOAD_LENGTH )
    {
        if (err)
        {
            *err = "MaxProtocolRecvPayloadLength should be less than: " + std::to_string(MAX_PROTOCOL_RECV_PAYLOAD_LENGTH ) + ".";
        }
        return false;
    }

    maxProtocolRecvPayloadLength = value;

    // Since value is between LEGACY_MAX_PROTOCOL_PAYLOAD_LENGTH and ONE_GIGABYTE and MAX_PROTOCOL_SEND_PAYLOAD_FACTOR is set to 4
    // this cannot overflow unsigned int
    maxProtocolSendPayloadLength = static_cast<unsigned int>(value * MAX_PROTOCOL_SEND_PAYLOAD_FACTOR);

    return true;
}

bool GlobalConfig::SetRecvInvQueueFactor(uint64_t value, std::string* err)
{
    if(value < MIN_RECV_INV_QUEUE_FACTOR || value > MAX_RECV_INV_QUEUE_FACTOR)
    {
        if(err)
        {
            *err = "RecvInvQueueFactor should be between: " + std::to_string(MIN_RECV_INV_QUEUE_FACTOR) + " and " +
                   std::to_string(MAX_RECV_INV_QUEUE_FACTOR) + ".";
        }
        return false;
    }
    recvInvQueueFactor = value;
    return true;
}

unsigned int GlobalConfig::GetMaxProtocolRecvPayloadLength() const
{
  return maxProtocolRecvPayloadLength;
}

unsigned int GlobalConfig::GetMaxProtocolSendPayloadLength() const
{
  return maxProtocolSendPayloadLength;
}

unsigned int GlobalConfig::GetRecvInvQueueFactor() const
{
  return recvInvQueueFactor;
}

DummyConfig::DummyConfig()
    : chainParams(CreateChainParams(CBaseChainParams::REGTEST)) {}

DummyConfig::DummyConfig(std::string net)
    : chainParams(CreateChainParams(net)) {}

void DummyConfig::SetChainParams(std::string net) {
    chainParams = CreateChainParams(net);
}

int DummyConfig::GetMaxConcurrentAsyncTasksPerNode() const
{
    return DEFAULT_NODE_ASYNC_TASKS_LIMIT;
}

int DummyConfig::GetMaxParallelBlocks() const
{
    return DEFAULT_SCRIPT_CHECK_POOL_SIZE;
}

int DummyConfig::GetPerBlockScriptValidatorThreadsCount() const
{
    return DEFAULT_SCRIPTCHECK_THREADS;
}

int DummyConfig::GetPerBlockScriptValidationMaxBatchSize() const
{
    return DEFAULT_SCRIPT_CHECK_MAX_BATCH_SIZE;
}

void DummyConfig::Reset() {}

void GlobalConfig::SetMinFeePerKB(CFeeRate fee) {
    feePerKB = fee;
}

CFeeRate GlobalConfig::GetMinFeePerKB() const {
    return feePerKB;
}

bool GlobalConfig::SetDustLimitFactor(int64_t factor, std::string* err) {
    if (factor < 0 || factor > DEFAULT_DUST_LIMIT_FACTOR)
    {
        if (err)
        {
            *err = _("The dust limit factor must be between 0% and ") + std::to_string(DEFAULT_DUST_LIMIT_FACTOR) + "%";
        }
        return false;
    }
    dustLimitFactor = factor;
    return true;
}

int64_t GlobalConfig::GetDustLimitFactor() const {
    return dustLimitFactor;
}

void GlobalConfig::SetBlockMinFeePerKB(CFeeRate fee) {
    blockMinFeePerKB = fee;
}

CFeeRate GlobalConfig::GetBlockMinFeePerKB() const {
    return blockMinFeePerKB;
}

bool GlobalConfig::SetMaxTxSigOpsCountPolicy(int64_t maxTxSigOpsCountIn, std::string* err)
{
    if (LessThanZero(maxTxSigOpsCountIn, err, "Policy value for maximum allowed number of signature operations per transaction cannot be less than 0"))
    {
        return false;
    }
    uint64_t maxTxSigOpsCountInUnsigned = static_cast<uint64_t>(maxTxSigOpsCountIn);
    if (maxTxSigOpsCountInUnsigned > MAX_TX_SIGOPS_COUNT_POLICY)
    {
        if (err)
        {
            *err = _("Policy value for maximum allowed number of signature operations per transaction must not exceed limit of ") + std::to_string(MAX_TX_SIGOPS_COUNT_POLICY);
        }
        return false;
    }
    if (maxTxSigOpsCountInUnsigned == 0)
    {
        maxTxSigOpsCountPolicy = MAX_TX_SIGOPS_COUNT_POLICY;
    }
    else
    {
        maxTxSigOpsCountPolicy = maxTxSigOpsCountInUnsigned;
    }
    return true;
}

uint64_t GlobalConfig::GetMaxTxSigOpsCountPolicy() const
{
    return maxTxSigOpsCountPolicy;
}

bool GlobalConfig::SetMaxScriptSizePolicy(int64_t maxScriptSizePolicyIn, std::string* err) {
    if (LessThanZero(maxScriptSizePolicyIn, err, "Policy value for max script size must not be less than 0"))
    {
        return false;
    }
    uint64_t maxScriptSizePolicyInUnsigned = static_cast<uint64_t>(maxScriptSizePolicyIn);
    if (maxScriptSizePolicyInUnsigned > MAX_SCRIPT_SIZE)
    {
        if (err)
        {
            *err = "Policy value for max script size must not exceed consensus limit of " + std::to_string(MAX_SCRIPT_SIZE);
        }
        return false;
    }
    else if (maxScriptSizePolicyInUnsigned == 0 ) {
        maxScriptSizePolicy = MAX_SCRIPT_SIZE;
    }
    else
    {
        maxScriptSizePolicy = maxScriptSizePolicyInUnsigned;
    }
    return true;
}

uint64_t GlobalConfig::GetMaxScriptSize(bool isConsensus) const {
    if (isConsensus)
    {
        return MAX_SCRIPT_SIZE;
    }
    return maxScriptSizePolicy;
}

bool GlobalConfig::SetMaxMempool(int64_t maxMempool, std::string* err) {
    if (LessThanZero(maxMempool, err, "Policy value for maximum resident memory pool must not be less than 0."))
    {
        return false;
    }
    if (maxMempool > 0 && maxMempool < DEFAULT_MAX_MEMPOOL_SIZE * ONE_MEGABYTE * 0.3)
    {
        if (err)
        {
            *err = strprintf(_("Policy value for maximum resident memory pool must be at least %d MB"), std::ceil(DEFAULT_MAX_MEMPOOL_SIZE * 0.3));
        }
        return false;
    }

    mMaxMempool = static_cast<uint64_t>(maxMempool);

    return true;
}

uint64_t GlobalConfig::GetMaxMempool() const {
    return mMaxMempool;
}

bool GlobalConfig::SetMaxMempoolSizeDisk(int64_t maxMempoolSizeDisk, std::string* err) {
    if (LessThanZero(maxMempoolSizeDisk, err, "Policy value for maximum on-disk memory pool must not be less than 0."))
    {
        return false;
    }

    mMaxMempoolSizeDisk = static_cast<uint64_t>(maxMempoolSizeDisk);

    return true;
}

uint64_t GlobalConfig::GetMaxMempoolSizeDisk() const {
    return mMaxMempoolSizeDisk;
}

bool GlobalConfig::SetMempoolMaxPercentCPFP(int64_t mempoolMaxPercentCPFP, std::string* err) {
    if (LessThanZero(mempoolMaxPercentCPFP, err, "Policy value for percentage of memory for low paying transactions must not be less than 0."))
    {
        return false;
    }

    if (mempoolMaxPercentCPFP > 100)
    {
        if (err)
        {
            *err = "Policy value for percentage of memory for low paying transactions must not be greater than 100";
        }
        return false;
    }

    mMempoolMaxPercentCPFP = static_cast<uint64_t>(mempoolMaxPercentCPFP);

    return true;
}

uint64_t GlobalConfig::GetMempoolMaxPercentCPFP() const {
    return mMempoolMaxPercentCPFP;
}

bool GlobalConfig::SetMemPoolExpiry(int64_t memPoolExpiry, std::string* err) {
    if (LessThanZero(memPoolExpiry, err, "Policy value for memory pool expiry must not be less than 0."))
    {
        return false;
    }

    mMemPoolExpiry = static_cast<uint64_t>(memPoolExpiry);

    return true;
}

uint64_t GlobalConfig::GetMemPoolExpiry() const {
    return mMemPoolExpiry;
}

bool GlobalConfig::SetMaxOrphanTxSize(int64_t maxOrphanTxSize, std::string* err) {
    if (LessThanZero(maxOrphanTxSize, err, "Policy value for maximum orphan transaction size must not be less than 0."))
    {
        return false;
    }

    mMaxOrphanTxSize = static_cast<uint64_t>(maxOrphanTxSize);

    return true;
}

uint64_t GlobalConfig::GetMaxOrphanTxSize() const {
    return mMaxOrphanTxSize;
}


bool GlobalConfig::SetMaxOrphansInBatchPercentage(uint64_t percent, std::string* err) {
    if (percent < 1 || percent > 100)
    {
        if (err)
        {
            *err = "Max percentage of orphans as percentage of maximal batch size must be between 1 and 100.";
        }
        return false;
    }

    mMaxPercentageOfOrphansInMaxBatchSize = percent;
    return true;
}

uint64_t GlobalConfig::GetMaxOrphansInBatchPercentage() const {
    return mMaxPercentageOfOrphansInMaxBatchSize;
}

bool GlobalConfig::SetMaxInputsForSecondLayerOrphan(uint64_t maxInputs, std::string* err) {
    if (LessThanZero(maxInputs, err, "Max inputs for out of first layer orphan txs must not be less than 0."))
    {
        return false;
    }

    mMaxInputsForSecondLayerOrphan = maxInputs;
    return true;
}

uint64_t GlobalConfig::GetMaxInputsForSecondLayerOrphan() const {
    return mMaxInputsForSecondLayerOrphan;
}

bool GlobalConfig::SetStopAtHeight(int32_t stopAtHeight, std::string* err) {
    if (LessThanZero(stopAtHeight, err, "Policy value for stop at height in the main chain must not be less than 0."))
    {
        return false;
    }

    mStopAtHeight = stopAtHeight;
    return true;
}

int32_t GlobalConfig::GetStopAtHeight() const {
    return mStopAtHeight;
}
