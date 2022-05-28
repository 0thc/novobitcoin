// Copyright (c) 2017 Amaury SÉCHET
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_CONFIG_H
#define BITCOIN_CONFIG_H

static_assert(sizeof(void*) >= 8, "32 bit systems are not supported");

#include "amount.h"
#include "consensus/consensus.h"
#include "mining/factory.h"
#include "net/net.h"
#include "policy/policy.h"
#include "rpc/client_config.h"
#include "script/standard.h"
#include "txn_validation_config.h"
#include "validation.h"
#include "script_config.h"
#include "invalid_txn_publisher.h"

#include <boost/noncopyable.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <orphan_txns.h>
#include <shared_mutex>

class CChainParams;
struct DefaultBlockSizeParams;

class Config : public boost::noncopyable, public CScriptConfig {
public:
    virtual uint64_t GetMaxBlockSize() const = 0;
    virtual bool SetMaxBlockSize(uint64_t maxBlockSize, std::string* err = nullptr) = 0;
    virtual uint64_t GetMaxGeneratedBlockSize() const = 0;
    virtual bool SetMaxGeneratedBlockSize(uint64_t maxGeneratedBlockSize, std::string* err = nullptr) = 0;
    virtual const CChainParams &GetChainParams() const = 0;
    virtual uint64_t GetMaxTxSize(bool isConsensus) const = 0;
    virtual uint64_t GetMinConsolidationFactor() const = 0;
    virtual uint64_t GetMaxConsolidationInputScriptSize() const = 0;
    virtual uint64_t GetMinConfConsolidationInput() const = 0;
    virtual bool GetAcceptNonStdConsolidationInput() const = 0;
    virtual CFeeRate GetMinFeePerKB() const = 0;
    virtual int64_t GetDustLimitFactor() const = 0;
    virtual CFeeRate GetBlockMinFeePerKB() const = 0;
    virtual uint64_t GetPreferredBlockFileSize() const = 0;
    virtual uint64_t GetDataCarrierSize() const = 0;
    virtual uint64_t GetLimitAncestorCount() const = 0;
    virtual uint64_t GetLimitSecondaryMempoolAncestorCount() const = 0;
    virtual bool GetTestBlockCandidateValidity() const = 0;
    virtual uint64_t GetFactorMaxSendQueuesBytes() const = 0;
    virtual uint64_t GetMaxSendQueuesBytes() const = 0; // calculated based on factorMaxSendQueuesBytes
    virtual mining::CMiningFactory::BlockAssemblerType GetMiningCandidateBuilder() const = 0;
    virtual int GetMaxConcurrentAsyncTasksPerNode() const = 0;
    virtual int GetMaxParallelBlocks() const = 0;
    virtual int GetPerBlockScriptValidatorThreadsCount() const = 0;
    virtual int GetPerBlockScriptValidationMaxBatchSize() const = 0;
    virtual uint64_t GetMaxTxSigOpsCountPolicy() const = 0;
    virtual uint64_t GetMaxBlockSigOpsConsensus(uint64_t blockSize) const = 0;
    virtual std::chrono::milliseconds GetMaxStdTxnValidationDuration() const = 0;
    virtual std::chrono::milliseconds GetMaxNonStdTxnValidationDuration() const = 0;
    virtual bool GetValidationClockCPU() const = 0;
    virtual std::chrono::milliseconds GetMaxTxnChainValidationBudget() const = 0;
    virtual uint64_t GetMaxCoinsViewCacheSize() const = 0;
    virtual uint64_t GetMaxCoinsProviderCacheSize() const = 0;
    virtual const std::set<uint256>& GetInvalidBlocks() const = 0;
    virtual bool IsBlockInvalidated(const uint256& hash) const = 0;
    virtual bool IsClientUABanned(const std::string uaClient) const = 0;
    virtual uint64_t GetMaxMerkleTreeDiskSpace() const = 0;
    virtual uint64_t GetPreferredMerkleTreeFileSize() const = 0;
    virtual uint64_t GetMaxMerkleTreeMemoryCacheSize() const = 0;
    virtual uint64_t GetMaxMempool() const = 0;
    virtual uint64_t GetMemPoolExpiry() const = 0;
    virtual uint64_t GetMaxOrphanTxSize() const = 0;
    virtual uint64_t GetMaxOrphansInBatchPercentage() const = 0;
    virtual uint64_t GetMaxInputsForSecondLayerOrphan() const = 0;
    virtual int32_t GetStopAtHeight() const = 0;
    virtual std::set<std::string> GetInvalidTxSinks() const = 0;
    virtual std::set<std::string> GetAvailableInvalidTxSinks() const = 0;
    virtual int64_t GetInvalidTxFileSinkMaxDiskUsage() const = 0;
    virtual InvalidTxEvictionPolicy GetInvalidTxFileSinkEvictionPolicy() const = 0;

    // Block download
    virtual uint64_t GetBlockStallingMinDownloadSpeed() const = 0;
    virtual int64_t GetBlockStallingTimeout() const = 0;
    virtual int64_t GetBlockDownloadWindow() const = 0;
    virtual int64_t GetBlockDownloadSlowFetchTimeout() const = 0;
    virtual uint64_t GetBlockDownloadMaxParallelFetch() const = 0;

    // P2P parameters
    virtual int64_t GetP2PHandshakeTimeout() const = 0;
    virtual int64_t GetStreamSendRateLimit() const = 0;
    virtual unsigned int GetBanScoreThreshold() const = 0;

#if ENABLE_ZMQ
    virtual int64_t GetInvalidTxZMQMaxMessageSize() const = 0;
#endif

    virtual unsigned int GetMaxProtocolRecvPayloadLength() const = 0;
    virtual unsigned int GetMaxProtocolSendPayloadLength() const = 0;
    virtual unsigned int GetRecvInvQueueFactor() const = 0;
    virtual uint64_t GetMaxCoinsDbOpenFiles() const = 0;
    virtual uint64_t GetMaxMempoolSizeDisk() const = 0;
    virtual uint64_t GetMempoolMaxPercentCPFP() const = 0;

protected:
    ~Config() = default;
};

class ConfigInit : public Config {
public:
    // used to specify default block size related parameters
    virtual void SetDefaultBlockSizeParams(const DefaultBlockSizeParams& params) = 0;
    virtual bool SetMaxTxSizePolicy(int64_t value, std::string* err = nullptr) = 0;
    virtual bool SetMinConsolidationFactor(int64_t value, std::string* err = nullptr) = 0;
    virtual bool SetMaxConsolidationInputScriptSize(int64_t value, std::string* err = nullptr) = 0;
    virtual bool SetMinConfConsolidationInput(int64_t value, std::string* err = nullptr) = 0;
    virtual bool SetAcceptNonStdConsolidationInput(bool flagValue, std::string* err = nullptr) = 0;
    virtual void SetMinFeePerKB(CFeeRate amt) = 0;
    virtual bool SetDustLimitFactor(int64_t factor, std::string* err = nullptr) = 0;
    virtual void SetBlockMinFeePerKB(CFeeRate amt) = 0;
    virtual void SetPreferredBlockFileSize(uint64_t preferredBlockFileSize) = 0;
    virtual void SetDataCarrierSize(uint64_t dataCarrierSize) = 0;
    virtual bool SetLimitAncestorCount(int64_t limitAncestorCount, std::string* err = nullptr) = 0;
    virtual void SetTestBlockCandidateValidity(bool test) = 0;
    virtual void SetFactorMaxSendQueuesBytes(uint64_t factorMaxSendQueuesBytes) = 0;
    virtual void SetMiningCandidateBuilder(mining::CMiningFactory::BlockAssemblerType type) = 0;
    virtual bool SetMaxConcurrentAsyncTasksPerNode(
        int maxConcurrentAsyncTasksPerNode,
        std::string* error = nullptr) = 0;
    virtual bool SetBlockScriptValidatorsParams(
        int maxParallelBlocks,
        int perValidatorThreadsCount,
        int perValidatorThreadMaxBatchSize,
        std::string* error = nullptr) = 0;
    virtual bool SetMaxOpsPerScriptPolicy(int64_t maxOpsPerScriptPolicyIn, std::string* error) = 0;
    /** Sets the maximum policy number of sigops we're willing to relay/mine in a single tx */
    virtual bool SetMaxTxSigOpsCountPolicy(int64_t maxTxSigOpsCountIn, std::string* err = nullptr) = 0;
    virtual bool SetMaxPubKeysPerMultiSigPolicy(int64_t maxPubKeysPerMultiSigIn, std::string* err = nullptr) = 0;
    virtual bool SetMaxStdTxnValidationDuration(int ms, std::string* err = nullptr) = 0;
    virtual bool SetMaxNonStdTxnValidationDuration(int ms, std::string* err = nullptr) = 0;
    virtual bool SetMaxTxnChainValidationBudget(int ms, std::string* err = nullptr) = 0;
    virtual void SetValidationClockCPU(bool enable) = 0;
    virtual bool SetMaxStackMemoryUsage(int64_t maxStackMemoryUsageConsensusIn, int64_t maxStackMemoryUsagePolicyIn, std::string* err = nullptr) = 0;
    virtual bool SetMaxScriptSizePolicy(int64_t maxScriptSizePolicyIn, std::string* err = nullptr) = 0;
    virtual bool SetMaxScriptNumLengthPolicy(int64_t maxScriptNumLengthIn, std::string* err = nullptr) = 0;
    virtual bool SetMaxCoinsViewCacheSize(int64_t max, std::string* err) = 0;
    virtual bool SetMaxCoinsProviderCacheSize(int64_t max, std::string* err) = 0;
    virtual bool SetMaxCoinsDbOpenFiles(int64_t max, std::string* err) = 0;
    virtual void SetInvalidBlocks(const std::set<uint256>& hashes) = 0;
    virtual void SetBanClientUA(const std::set<std::string> uaClients) = 0;
    virtual bool SetMaxMerkleTreeDiskSpace(int64_t maxDiskSpace, std::string* err = nullptr) = 0;
    virtual bool SetPreferredMerkleTreeFileSize(int64_t preferredFileSize, std::string* err = nullptr) = 0;
    virtual bool SetMaxMerkleTreeMemoryCacheSize(int64_t maxMemoryCacheSize, std::string* err = nullptr) = 0;
    virtual bool SetMaxMempool(int64_t maxMempool, std::string* err) = 0;
    virtual bool SetMaxMempoolSizeDisk(int64_t maxMempoolSizeDisk, std::string* err) = 0;
    virtual bool SetMempoolMaxPercentCPFP(int64_t mempoolMaxPercentCPFP, std::string* err) = 0;
    virtual bool SetMemPoolExpiry(int64_t memPoolExpiry, std::string* err) = 0;
    virtual bool SetMaxOrphanTxSize(int64_t maxOrphanTxSize, std::string* err) = 0;
    virtual bool SetMaxOrphansInBatchPercentage(uint64_t percent, std::string* err) = 0;
    virtual bool SetMaxInputsForSecondLayerOrphan(uint64_t maxInputs, std::string* err) = 0;
    virtual bool SetStopAtHeight(int32_t StopAtHeight, std::string* err) = 0;
    virtual bool AddInvalidTxSink(const std::string& sink, std::string* err = nullptr) = 0;
    virtual bool SetInvalidTxFileSinkMaxDiskUsage(int64_t max, std::string* err = nullptr) = 0;
    virtual bool SetInvalidTxFileSinkEvictionPolicy(std::string policy, std::string* err = nullptr) = 0;

    // Block download
    virtual bool SetBlockStallingMinDownloadSpeed(int64_t min, std::string* err = nullptr) = 0;
    virtual bool SetBlockStallingTimeout(int64_t timeout, std::string* err = nullptr) = 0;
    virtual bool SetBlockDownloadWindow(int64_t window, std::string* err = nullptr) = 0;
    virtual bool SetBlockDownloadSlowFetchTimeout(int64_t timeout, std::string* err = nullptr) = 0;
    virtual bool SetBlockDownloadMaxParallelFetch(int64_t max, std::string* err = nullptr) = 0;

    // P2P parameters
    virtual bool SetP2PHandshakeTimeout(int64_t timeout, std::string* err = nullptr) = 0;
    virtual bool SetStreamSendRateLimit(int64_t limit, std::string* err = nullptr) = 0;
    virtual bool SetBanScoreThreshold(int64_t threshold, std::string* err = nullptr) = 0;

#if ENABLE_ZMQ
    virtual bool SetInvalidTxZMQMaxMessageSize(int64_t max, std::string* err = nullptr) = 0;
#endif

    virtual bool SetMaxProtocolRecvPayloadLength(uint64_t value, std::string* err) = 0;
    virtual bool SetRecvInvQueueFactor(uint64_t value, std::string* err) = 0;
    virtual bool SetLimitSecondaryMempoolAncestorCount(int64_t limitSecondaryMempoolAncestorCountIn, std::string* err = nullptr) = 0;

    // Reset state of this object to match a newly constructed one.
    // Used in constructor and for unit testing to always start with a clean
    // state
    virtual void Reset() = 0;

protected:
    ~ConfigInit() = default;
};

class GlobalConfig final : public ConfigInit {
public:
    GlobalConfig();

    // Set block size related default. This must be called after constructing GlobalConfig
    void SetDefaultBlockSizeParams(const DefaultBlockSizeParams &params) override;

    bool SetMaxBlockSize(uint64_t maxBlockSize, std::string* err = nullptr) override;
    uint64_t GetMaxBlockSize() const override;

    bool SetMaxGeneratedBlockSize(uint64_t maxGeneratedBlockSize, std::string* err = nullptr) override;
    uint64_t GetMaxGeneratedBlockSize() const override;

    const CChainParams &GetChainParams() const override;

    bool SetMaxTxSizePolicy(int64_t value, std::string* err = nullptr) override;
    uint64_t GetMaxTxSize(bool isConsensus) const  override;

    bool SetMinConsolidationFactor(int64_t value, std::string* err = nullptr) override;
    uint64_t GetMinConsolidationFactor() const  override;

    bool SetMaxConsolidationInputScriptSize(int64_t value, std::string* err = nullptr) override;
    uint64_t GetMaxConsolidationInputScriptSize() const  override;

    bool SetMinConfConsolidationInput(int64_t value, std::string* err = nullptr) override;
    uint64_t GetMinConfConsolidationInput() const override;

    bool SetAcceptNonStdConsolidationInput(bool flagValue, std::string* err = nullptr) override;
    bool GetAcceptNonStdConsolidationInput() const  override;

    void SetMinFeePerKB(CFeeRate amt) override;
    CFeeRate GetMinFeePerKB() const override;

    bool SetDustLimitFactor(int64_t factor, std::string* err = nullptr) override;
    int64_t GetDustLimitFactor() const override;

    void SetBlockMinFeePerKB(CFeeRate amt) override;
    CFeeRate GetBlockMinFeePerKB() const override;

    void SetPreferredBlockFileSize(uint64_t preferredBlockFileSize) override;
    uint64_t GetPreferredBlockFileSize() const override;

    void SetDataCarrierSize(uint64_t dataCarrierSize) override;
    uint64_t GetDataCarrierSize() const override;

    bool SetLimitAncestorCount(int64_t limitAncestorCount, std::string* err = nullptr) override;
    uint64_t GetLimitAncestorCount() const override;

    bool SetLimitSecondaryMempoolAncestorCount(int64_t limitSecondaryMempoolAncestorCountIn, std::string* err = nullptr) override;
    uint64_t GetLimitSecondaryMempoolAncestorCount() const override;

    void SetTestBlockCandidateValidity(bool test) override;
    bool GetTestBlockCandidateValidity() const override;

    void SetFactorMaxSendQueuesBytes(uint64_t factorMaxSendQueuesBytes) override;
    uint64_t GetFactorMaxSendQueuesBytes() const override;
    uint64_t GetMaxSendQueuesBytes() const override;

    void SetMiningCandidateBuilder(mining::CMiningFactory::BlockAssemblerType type) override;
    mining::CMiningFactory::BlockAssemblerType GetMiningCandidateBuilder() const override;

    bool SetMaxConcurrentAsyncTasksPerNode(
        int maxConcurrentAsyncTasksPerNode,
        std::string* error = nullptr) override;
    int GetMaxConcurrentAsyncTasksPerNode() const override;

    bool SetBlockScriptValidatorsParams(
        int maxParallelBlocks,
        int perValidatorThreadsCount,
        int perValidatorThreadMaxBatchSize,
        std::string* error = nullptr) override;
    int GetMaxParallelBlocks() const override;
    int GetPerBlockScriptValidatorThreadsCount() const override;
    int GetPerBlockScriptValidationMaxBatchSize() const override;

    bool SetMaxOpsPerScriptPolicy(int64_t maxOpsPerScriptPolicyIn, std::string* error) override;
    uint64_t GetMaxOpsPerScript(bool consensus) const override;

    bool SetMaxTxSigOpsCountPolicy(int64_t maxTxSigOpsCountIn, std::string* err = nullptr) override;
    uint64_t GetMaxTxSigOpsCountPolicy() const override;

    uint64_t GetMaxBlockSigOpsConsensus(uint64_t blockSize) const override;

    bool SetMaxPubKeysPerMultiSigPolicy(int64_t maxPubKeysPerMultiSigIn, std::string* error = nullptr) override;
    uint64_t GetMaxPubKeysPerMultiSig(bool consensus) const override;

    bool SetMaxStdTxnValidationDuration(int ms, std::string* err = nullptr) override;
    std::chrono::milliseconds GetMaxStdTxnValidationDuration() const override;

    bool SetMaxNonStdTxnValidationDuration(int ms, std::string* err = nullptr) override;
    std::chrono::milliseconds GetMaxNonStdTxnValidationDuration() const override;

    bool SetMaxTxnChainValidationBudget(int ms, std::string* err = nullptr) override;
    std::chrono::milliseconds GetMaxTxnChainValidationBudget() const override;

    void SetValidationClockCPU(bool enable) override;
    bool GetValidationClockCPU() const override;

    bool SetMaxStackMemoryUsage(int64_t maxStackMemoryUsageConsensusIn, int64_t maxStackMemoryUsagePolicyIn, std::string* err = nullptr) override;
    uint64_t GetMaxStackMemoryUsage(bool consensus) const override;


    bool SetMaxScriptSizePolicy(int64_t maxScriptSizePolicyIn, std::string* err = nullptr) override;
    uint64_t GetMaxScriptSize(bool isConsensus) const override;

    bool SetMaxScriptNumLengthPolicy(int64_t maxScriptNumLengthIn, std::string* err = nullptr) override;
    uint64_t GetMaxScriptNumLength(bool isConsensus) const override;

    bool SetMaxCoinsViewCacheSize(int64_t max, std::string* err) override;
    uint64_t GetMaxCoinsViewCacheSize() const override {return mMaxCoinsViewCacheSize;}

    bool SetMaxCoinsProviderCacheSize(int64_t max, std::string* err) override;
    uint64_t GetMaxCoinsProviderCacheSize() const override {return mMaxCoinsProviderCacheSize;}

    bool SetMaxCoinsDbOpenFiles(int64_t max, std::string* err) override;
    uint64_t GetMaxCoinsDbOpenFiles() const override {return mMaxCoinsDbOpenFiles; }

    void SetInvalidBlocks(const std::set<uint256>& hashes) override;
    const std::set<uint256>& GetInvalidBlocks() const override;
    bool IsBlockInvalidated(const uint256& hash) const override;

    void SetBanClientUA(const std::set<std::string> uaClients) override;
    bool IsClientUABanned(const std::string uaClient) const override;
    bool SetMaxMerkleTreeDiskSpace(int64_t maxDiskSpace, std::string* err = nullptr) override;
    uint64_t GetMaxMerkleTreeDiskSpace() const override;
    bool SetPreferredMerkleTreeFileSize(int64_t preferredFileSize, std::string* err = nullptr) override;
    uint64_t GetPreferredMerkleTreeFileSize() const override;
    bool SetMaxMerkleTreeMemoryCacheSize(int64_t maxMemoryCacheSize, std::string* err = nullptr) override;
    uint64_t GetMaxMerkleTreeMemoryCacheSize() const override;

    bool SetMaxMempool(int64_t maxMempool, std::string* err) override;
    uint64_t GetMaxMempool() const override;

    bool SetMaxMempoolSizeDisk(int64_t maxMempoolSizeDisk, std::string* err) override;
    uint64_t GetMaxMempoolSizeDisk() const override;

    bool SetMempoolMaxPercentCPFP(int64_t mempoolMaxPercentCPFP, std::string* err) override;
    uint64_t GetMempoolMaxPercentCPFP() const override;

    bool SetMemPoolExpiry(int64_t memPoolExpiry, std::string* err) override;
    uint64_t GetMemPoolExpiry() const override;

    bool SetMaxOrphanTxSize(int64_t maxOrphanTxSize, std::string* err) override;
    uint64_t GetMaxOrphanTxSize() const override;

    bool SetMaxOrphansInBatchPercentage(uint64_t percent, std::string* err) override;
    uint64_t GetMaxOrphansInBatchPercentage() const override;

    bool SetMaxInputsForSecondLayerOrphan(uint64_t maxInputs, std::string* err) override;
    uint64_t GetMaxInputsForSecondLayerOrphan() const override;

    bool SetStopAtHeight(int32_t stopAtHeight, std::string* err) override;
    int32_t GetStopAtHeight() const override;

    bool AddInvalidTxSink(const std::string& sink, std::string* err) override;
    std::set<std::string> GetInvalidTxSinks() const override;
    std::set<std::string> GetAvailableInvalidTxSinks() const override;

    bool SetInvalidTxFileSinkMaxDiskUsage(int64_t max, std::string* err) override;
    int64_t GetInvalidTxFileSinkMaxDiskUsage() const override;

    bool SetInvalidTxFileSinkEvictionPolicy(std::string policy, std::string* err = nullptr) override;
    InvalidTxEvictionPolicy GetInvalidTxFileSinkEvictionPolicy() const override;

    // Block download
    bool SetBlockStallingMinDownloadSpeed(int64_t min, std::string* err = nullptr) override;
    uint64_t GetBlockStallingMinDownloadSpeed() const override;
    bool SetBlockStallingTimeout(int64_t timeout, std::string* err = nullptr) override;
    int64_t GetBlockStallingTimeout() const override;
    bool SetBlockDownloadWindow(int64_t window, std::string* err = nullptr) override;
    int64_t GetBlockDownloadWindow() const override;
    bool SetBlockDownloadSlowFetchTimeout(int64_t timeout, std::string* err = nullptr) override;
    int64_t GetBlockDownloadSlowFetchTimeout() const override;
    bool SetBlockDownloadMaxParallelFetch(int64_t max, std::string* err = nullptr) override;
    uint64_t GetBlockDownloadMaxParallelFetch() const override;

    // P2P parameters
    bool SetP2PHandshakeTimeout(int64_t timeout, std::string* err = nullptr) override;
    int64_t GetP2PHandshakeTimeout() const override { return p2pHandshakeTimeout; }
    bool SetStreamSendRateLimit(int64_t limit, std::string* err = nullptr) override;
    int64_t GetStreamSendRateLimit() const override;
    bool SetBanScoreThreshold(int64_t threshold, std::string* err = nullptr) override;
    unsigned int GetBanScoreThreshold() const override;

#if ENABLE_ZMQ
    bool SetInvalidTxZMQMaxMessageSize(int64_t max, std::string* err = nullptr) override;
    int64_t GetInvalidTxZMQMaxMessageSize() const override;
#endif

    bool SetMaxProtocolRecvPayloadLength(uint64_t value, std::string* err) override;
    bool SetRecvInvQueueFactor(uint64_t value, std::string* err) override;
    unsigned int GetMaxProtocolRecvPayloadLength() const override;
    unsigned int GetMaxProtocolSendPayloadLength() const override;
    unsigned int GetRecvInvQueueFactor() const override;

    // Reset state of this object to match a newly constructed one.
    // Used in constructor and for unit testing to always start with a clean state
    void Reset() override;

    // GetConfig() is used where read-only access to global configuration is needed.
    static Config& GetConfig();
    // GetModifiableGlobalConfig() should only be used in initialization and unit tests.
    static ConfigInit& GetModifiableGlobalConfig();

private:
    // All fileds are initialized in Reset()
    CFeeRate feePerKB;
    int64_t dustLimitFactor;
    CFeeRate blockMinFeePerKB;
    uint64_t preferredBlockFileSize;
    uint64_t factorMaxSendQueuesBytes;

    // Block size limits
    // SetDefaultBlockSizeParams must be called before reading any of those
    bool  setDefaultBlockSizeParamsCalled;
    void  CheckSetDefaultCalled() const;

    uint64_t maxBlockSize;
    // Used when SetMaxBlockSize is called with value 0
    uint64_t defaultBlockSize;
    uint64_t maxGeneratedBlockSize;

    uint64_t maxTxSizePolicy;
    uint64_t minConsolidationFactor;
    uint64_t maxConsolidationInputScriptSize;
    uint64_t minConfConsolidationInput;
    bool acceptNonStdConsolidationInput;
    uint64_t dataCarrierSize;
    uint64_t limitAncestorCount;
    uint64_t limitSecondaryMempoolAncestorCount;

    bool testBlockCandidateValidity;
    mining::CMiningFactory::BlockAssemblerType blockAssemblerType;

    int mMaxConcurrentAsyncTasksPerNode;

    int mMaxParallelBlocks;
    int mPerBlockScriptValidatorThreadsCount;
    int mPerBlockScriptValidationMaxBatchSize;

    uint64_t maxOpsPerScriptPolicy;

    uint64_t maxTxSigOpsCountPolicy;
    uint64_t maxPubKeysPerMultiSig;

    std::chrono::milliseconds mMaxStdTxnValidationDuration;
    std::chrono::milliseconds mMaxNonStdTxnValidationDuration;
    std::chrono::milliseconds mMaxTxnChainValidationBudget;

    bool mValidationClockCPU;

    uint64_t maxStackMemoryUsagePolicy;
    uint64_t maxStackMemoryUsageConsensus;

    uint64_t maxScriptSizePolicy;

    uint64_t maxScriptNumLengthPolicy;

    uint64_t mMaxCoinsViewCacheSize;
    uint64_t mMaxCoinsProviderCacheSize;

    uint64_t mMaxCoinsDbOpenFiles;

    uint64_t mMaxMempool;
    uint64_t mMaxMempoolSizeDisk;
    uint64_t mMempoolMaxPercentCPFP;
    uint64_t mMemPoolExpiry;
    uint64_t mMaxOrphanTxSize;
    uint64_t mMaxPercentageOfOrphansInMaxBatchSize;
    uint64_t mMaxInputsForSecondLayerOrphan;
    int32_t mStopAtHeight;

    std::set<uint256> mInvalidBlocks;

    std::set<std::string> mBannedUAClients;
    uint64_t maxMerkleTreeDiskSpace;
    uint64_t preferredMerkleTreeFileSize;
    uint64_t maxMerkleTreeMemoryCacheSize;

    std::set<std::string> invalidTxSinks;
    int64_t invalidTxFileSinkSize;
    InvalidTxEvictionPolicy invalidTxFileSinkEvictionPolicy;

    // Block download
    uint64_t blockStallingMinDownloadSpeed;
    int64_t blockStallingTimeout;
    int64_t blockDownloadWindow;
    int64_t blockDownloadSlowFetchTimeout;
    uint64_t blockDownloadMaxParallelFetch;

    // P2P parameters
    int64_t p2pHandshakeTimeout;
    int64_t streamSendRateLimit;
    unsigned int maxProtocolRecvPayloadLength;
    unsigned int maxProtocolSendPayloadLength;
    unsigned int recvInvQueueFactor;
    unsigned int banScoreThreshold;

#if ENABLE_ZMQ
    int64_t invalidTxZMQMaxMessageSize;
#endif

    // Only for values that can change in runtime
    mutable std::shared_mutex configMtx{};

};

// Dummy for subclassing in unittests
class DummyConfig : public ConfigInit {
public:
    DummyConfig();
    DummyConfig(std::string net);

    void SetDefaultBlockSizeParams(const DefaultBlockSizeParams &params) override {  }

    bool SetMaxBlockSize(uint64_t maxBlockSize, std::string* err = nullptr) override {
        SetErrorMsg(err);
        return false;
    }
    uint64_t GetMaxBlockSize() const override { return 0; }

    bool SetMaxGeneratedBlockSize(uint64_t maxGeneratedBlockSize, std::string* err = nullptr) override {
        SetErrorMsg(err);
        return false;
    }
    uint64_t GetMaxGeneratedBlockSize() const override { return 0; };

    bool SetMaxTxSizePolicy(int64_t value, std::string* err = nullptr) override
    {
        SetErrorMsg(err);
        maxTxSizePolicy = value;
        return false;
    }
    uint64_t GetMaxTxSize(bool isConsensus) const override { return maxTxSizePolicy; }

    bool SetMinConsolidationFactor(int64_t value, std::string* err = nullptr) override
    {
        SetErrorMsg(err);
        minConsolidationFactor = value;
        return false;
    }
    uint64_t GetMinConsolidationFactor() const override { return minConsolidationFactor; }

    bool SetMaxConsolidationInputScriptSize(int64_t value, std::string* err = nullptr) override
    {
        SetErrorMsg(err);
        maxConsolidationInputScriptSize = value;
        return false;
    }
    uint64_t GetMaxConsolidationInputScriptSize() const override { return maxConsolidationInputScriptSize; }

    bool SetMinConfConsolidationInput(int64_t value, std::string* err = nullptr) override
    {
        SetErrorMsg(err);
        minConfConsolidationInput = value;
        return false;
    }
    uint64_t GetMinConfConsolidationInput() const override { return minConfConsolidationInput; }

    bool SetAcceptNonStdConsolidationInput(bool flagValue, std::string* err = nullptr) override
    {
        SetErrorMsg(err);
        acceptNonStdConsolidationInput = flagValue;
        return false;
    }
    bool GetAcceptNonStdConsolidationInput() const override { return acceptNonStdConsolidationInput; }

    void SetChainParams(std::string net);
    const CChainParams &GetChainParams() const override { return *chainParams; }

    void SetMinFeePerKB(CFeeRate amt) override{};
    CFeeRate GetMinFeePerKB() const override { return CFeeRate(Amount(0)); }

    bool SetDustLimitFactor(int64_t factor, std::string* err = nullptr) override{return true;};
    int64_t GetDustLimitFactor() const override { return 0; }

    void SetBlockMinFeePerKB(CFeeRate amt) override{};
    CFeeRate GetBlockMinFeePerKB() const override { return CFeeRate(Amount(0)); }

    void SetPreferredBlockFileSize(uint64_t preferredBlockFileSize) override {}
    uint64_t GetPreferredBlockFileSize() const override { return 0; }

    uint64_t GetDataCarrierSize() const override { return dataCarrierSize; }
    void SetDataCarrierSize(uint64_t dataCarrierSizeIn) override { dataCarrierSize = dataCarrierSizeIn; }

    bool SetLimitAncestorCount(int64_t limitAncestorCount, std::string* err = nullptr) override {return true;}
    uint64_t GetLimitAncestorCount() const override { return 0; }

    bool SetLimitSecondaryMempoolAncestorCount(int64_t limitSecondaryMempoolAncestorCountIn, std::string* err = nullptr) override {return true;}
    uint64_t GetLimitSecondaryMempoolAncestorCount() const override { return 0; }

    void SetTestBlockCandidateValidity(bool skip) override {}
    bool GetTestBlockCandidateValidity() const override { return false; }

    void SetFactorMaxSendQueuesBytes(uint64_t factorMaxSendQueuesBytes) override {}
    uint64_t GetFactorMaxSendQueuesBytes() const override { return 0;}
    uint64_t GetMaxSendQueuesBytes() const override { return 0; }

    void SetMiningCandidateBuilder(mining::CMiningFactory::BlockAssemblerType type) override {}
    mining::CMiningFactory::BlockAssemblerType GetMiningCandidateBuilder() const override {
        return mining::CMiningFactory::BlockAssemblerType::JOURNALING;
    }

    bool SetMaxConcurrentAsyncTasksPerNode(
        int maxConcurrentAsyncTasksPerNode,
        std::string* error = nullptr) override
    {
        SetErrorMsg(error);

        return false;
    }
    int GetMaxConcurrentAsyncTasksPerNode() const override;

    bool SetBlockScriptValidatorsParams(
        int maxParallelBlocks,
        int perValidatorThreadsCount,
        int perValidatorThreadMaxBatchSize,
        std::string* error = nullptr) override
    {
        SetErrorMsg(error);

        return false;
    }
    int GetMaxParallelBlocks() const override;
    int GetPerBlockScriptValidatorThreadsCount() const override;
    int GetPerBlockScriptValidationMaxBatchSize() const override;
    bool SetMaxStackMemoryUsage(int64_t maxStackMemoryUsageConsensusIn, int64_t maxStackMemoryUsagePolicyIn, std::string* err = nullptr)  override { return true; }
    uint64_t GetMaxStackMemoryUsage(bool consensus) const override { return UINT32_MAX; }

    bool SetMaxOpsPerScriptPolicy(int64_t maxOpsPerScriptPolicyIn, std::string* error) override { return true;  }
    uint64_t GetMaxOpsPerScript(bool consensus) const override
    {
        return MAX_OPS_PER_SCRIPT;
    }
    bool SetMaxTxSigOpsCountPolicy(int64_t maxTxSigOpsCountIn, std::string* err = nullptr) override { return true; }
    uint64_t GetMaxTxSigOpsCountPolicy() const override { return MAX_TX_SIGOPS_COUNT_POLICY; }

    uint64_t GetMaxBlockSigOpsConsensus(uint64_t blockSize) const override { throw std::runtime_error("DummyCofig::GetMaxBlockSigOps not implemented"); }

    bool SetMaxPubKeysPerMultiSigPolicy(int64_t maxPubKeysPerMultiSigIn, std::string* err = nullptr) override { return true; }
    uint64_t GetMaxPubKeysPerMultiSig(bool consensus) const override
    {
        return MAX_PUBKEYS_PER_MULTISIG;
    }

    bool SetMaxStdTxnValidationDuration(int ms, std::string* err = nullptr) override
    {
        SetErrorMsg(err);

        return false;
    }
    std::chrono::milliseconds GetMaxStdTxnValidationDuration() const override
    {
        return DEFAULT_MAX_STD_TXN_VALIDATION_DURATION;
    }

    bool SetMaxNonStdTxnValidationDuration(int ms, std::string* err = nullptr) override
    {
        SetErrorMsg(err);

        return false;
    }
    std::chrono::milliseconds GetMaxNonStdTxnValidationDuration() const override
    {
        return DEFAULT_MAX_NON_STD_TXN_VALIDATION_DURATION;
    }

    bool SetMaxTxnChainValidationBudget(int ms, std::string* err = nullptr) override
    {
        SetErrorMsg(err);

        return false;
    }
    std::chrono::milliseconds GetMaxTxnChainValidationBudget() const override
    {
        return DEFAULT_MAX_TXN_CHAIN_VALIDATION_BUDGET;
    }

    void SetValidationClockCPU(bool enable) override {}
    bool GetValidationClockCPU() const override { return DEFAULT_VALIDATION_CLOCK_CPU; }

    bool SetMaxScriptSizePolicy(int64_t maxScriptSizePolicyIn, std::string* err = nullptr) override
    {
        SetErrorMsg(err);
        maxScriptSizePolicy = static_cast<uint64_t>(maxScriptSizePolicyIn);
        return true;
    };
    uint64_t GetMaxScriptSize(bool isConsensus) const override { return maxScriptSizePolicy; };

    bool SetMaxScriptNumLengthPolicy(int64_t maxScriptNumLengthIn, std::string* err = nullptr) override { return true;  }
    uint64_t GetMaxScriptNumLength(bool isConsensus) const override
    {
        return MAX_SCRIPT_NUM_LENGTH;
    }

    bool SetMaxCoinsViewCacheSize(int64_t max, std::string* err) override
    {
        SetErrorMsg(err);

        return false;
    }
    uint64_t GetMaxCoinsViewCacheSize() const override {return 0; /* unlimited */}

    bool SetMaxCoinsProviderCacheSize(int64_t max, std::string* err) override
    {
        SetErrorMsg(err);

        return false;
    }
    uint64_t GetMaxCoinsProviderCacheSize() const override {return 0; /* unlimited */}

    bool SetMaxCoinsDbOpenFiles(int64_t max, std::string* err)  override
    {
        SetErrorMsg(err);

        return false;
    }
    uint64_t GetMaxCoinsDbOpenFiles() const override {return 64; /* old default */}

    bool SetMaxMempool(int64_t maxMempool, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    uint64_t GetMaxMempool() const override { return DEFAULT_MAX_MEMPOOL_SIZE * ONE_MEGABYTE; }

    bool SetMaxMempoolSizeDisk(int64_t maxMempoolSizeDisk, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    uint64_t GetMaxMempoolSizeDisk() const override {
        return DEFAULT_MAX_MEMPOOL_SIZE * DEFAULT_MAX_MEMPOOL_SIZE_DISK_FACTOR * ONE_MEGABYTE;
    }

    bool SetMempoolMaxPercentCPFP(int64_t mempoolMaxPercentCPFP, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    uint64_t GetMempoolMaxPercentCPFP() const override { return DEFAULT_MEMPOOL_MAX_PERCENT_CPFP; }

    bool SetMemPoolExpiry(int64_t memPoolExpiry, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    uint64_t GetMemPoolExpiry() const override { return DEFAULT_MEMPOOL_EXPIRY * SECONDS_IN_ONE_HOUR; }

    bool SetMaxOrphanTxSize(int64_t maxOrphanTxSize, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    uint64_t GetMaxOrphanTxSize() const override { return COrphanTxns::DEFAULT_MAX_ORPHAN_TRANSACTIONS_SIZE; }

    bool SetMaxOrphansInBatchPercentage(uint64_t percent, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }

    uint64_t GetMaxOrphansInBatchPercentage() const override { return COrphanTxns::DEFAULT_MAX_PERCENTAGE_OF_ORPHANS_IN_BATCH; };

    bool SetMaxInputsForSecondLayerOrphan(uint64_t maxInputs, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    uint64_t GetMaxInputsForSecondLayerOrphan() const override { return COrphanTxns::DEFAULT_MAX_INPUTS_OUTPUTS_PER_TRANSACTION; };

    bool SetStopAtHeight(int32_t stopAtHeight, std::string* err) override
    {
        SetErrorMsg(err);

        return true;
    }
    int32_t GetStopAtHeight() const override { return DEFAULT_STOPATHEIGHT; }

    void SetInvalidBlocks(const std::set<uint256>& hashes) override
    {
        mInvalidBlocks = hashes;
    };

    const std::set<uint256>& GetInvalidBlocks() const override
    {
        return mInvalidBlocks;
    };

    bool IsBlockInvalidated(const uint256& hash) const override
    {
        return mInvalidBlocks.find(hash) != mInvalidBlocks.end();
    };

    void SetBanClientUA(const std::set<std::string> uaClients) override
    {
        mBannedUAClients = uaClients;
    }

    bool IsClientUABanned(const std::string uaClient) const override
    {
        return mBannedUAClients.find(uaClient) != mBannedUAClients.end();
    }

    bool SetMaxProtocolRecvPayloadLength(uint64_t value, std::string* err) override { return true; }
    bool SetRecvInvQueueFactor(uint64_t value, std::string* err) override { return true; }
    unsigned int GetMaxProtocolRecvPayloadLength() const override { return DEFAULT_MAX_PROTOCOL_RECV_PAYLOAD_LENGTH; }
    unsigned int GetMaxProtocolSendPayloadLength() const override { return DEFAULT_MAX_PROTOCOL_RECV_PAYLOAD_LENGTH*MAX_PROTOCOL_SEND_PAYLOAD_FACTOR; }
    unsigned int GetRecvInvQueueFactor() const override { return DEFAULT_RECV_INV_QUEUE_FACTOR; }

    bool AddInvalidTxSink(const std::string& sink, std::string* err = nullptr) override { return true; };
    std::set<std::string> GetInvalidTxSinks() const override { return {"NONE"}; };
    std::set<std::string> GetAvailableInvalidTxSinks() const override { return {"NONE"}; };

    bool SetInvalidTxFileSinkMaxDiskUsage(int64_t max, std::string* err = nullptr) override { return true; };
    int64_t GetInvalidTxFileSinkMaxDiskUsage() const override { return 300* ONE_MEGABYTE; };

    bool SetInvalidTxFileSinkEvictionPolicy(std::string policy, std::string* err = nullptr) override { return true; };
    InvalidTxEvictionPolicy GetInvalidTxFileSinkEvictionPolicy() const override { return InvalidTxEvictionPolicy::IGNORE_NEW; };

    // Block download
    bool SetBlockStallingMinDownloadSpeed(int64_t min, std::string* err = nullptr) override { return true; }
    uint64_t GetBlockStallingMinDownloadSpeed() const override { return DEFAULT_MIN_BLOCK_STALLING_RATE; }
    bool SetBlockStallingTimeout(int64_t timeout, std::string* err = nullptr) override { return true; }
    int64_t GetBlockStallingTimeout() const override { return DEFAULT_BLOCK_STALLING_TIMEOUT; }
    bool SetBlockDownloadWindow(int64_t window, std::string* err = nullptr) override { return true; }
    int64_t GetBlockDownloadWindow() const override { return DEFAULT_BLOCK_DOWNLOAD_WINDOW; }
    bool SetBlockDownloadSlowFetchTimeout(int64_t timeout, std::string* err = nullptr) override { return true; }
    int64_t GetBlockDownloadSlowFetchTimeout() const override { return DEFAULT_BLOCK_DOWNLOAD_SLOW_FETCH_TIMEOUT; }
    bool SetBlockDownloadMaxParallelFetch(int64_t max, std::string* err = nullptr) override { return true; }
    uint64_t GetBlockDownloadMaxParallelFetch() const override { return DEFAULT_MAX_BLOCK_PARALLEL_FETCH; }

    // P2P parameters
    bool SetP2PHandshakeTimeout(int64_t timeout, std::string* err = nullptr) override { return true; }
    int64_t GetP2PHandshakeTimeout() const override { return DEFAULT_P2P_HANDSHAKE_TIMEOUT_INTERVAL; }
    bool SetStreamSendRateLimit(int64_t limit, std::string* err = nullptr) override { return true; }
    int64_t GetStreamSendRateLimit() const override { return Stream::DEFAULT_SEND_RATE_LIMIT; }
    bool SetBanScoreThreshold(int64_t threshold, std::string* err = nullptr) override { return true; }
    unsigned int GetBanScoreThreshold() const override { return DEFAULT_BANSCORE_THRESHOLD; }

#if ENABLE_ZMQ
    bool SetInvalidTxZMQMaxMessageSize(int64_t max, std::string* err = nullptr) override { return true; };
    int64_t GetInvalidTxZMQMaxMessageSize() const override { return 10 * ONE_MEGABYTE; };
#endif

    bool SetMaxMerkleTreeDiskSpace(int64_t maxDiskSpace, std::string* err = nullptr) override
    {
        return true;
    }

    uint64_t GetMaxMerkleTreeDiskSpace() const override
    {
        return 0;
    }

    bool SetPreferredMerkleTreeFileSize(int64_t preferredFileSize, std::string* err = nullptr) override
    {
        return true;
    }

    uint64_t GetPreferredMerkleTreeFileSize() const override
    {
        return 0;
    }

    bool SetMaxMerkleTreeMemoryCacheSize(int64_t maxMemoryCacheSize, std::string* err = nullptr) override
    {
        return true;
    }

    uint64_t GetMaxMerkleTreeMemoryCacheSize() const override
    {
        return 0;
    }

    void Reset() override;

private:
    std::unique_ptr<CChainParams> chainParams;
    uint64_t dataCarrierSize { DEFAULT_DATA_CARRIER_SIZE };
    uint64_t maxTxSizePolicy{ DEFAULT_MAX_TX_SIZE_POLICY };
    uint64_t minConsolidationFactor{ DEFAULT_MIN_CONSOLIDATION_FACTOR };
    uint64_t maxConsolidationInputScriptSize{DEFAULT_MAX_CONSOLIDATION_INPUT_SCRIPT_SIZE };
    uint64_t minConfConsolidationInput { DEFAULT_MIN_CONF_CONSOLIDATION_INPUT };
    uint64_t acceptNonStdConsolidationInput { DEFAULT_ACCEPT_NON_STD_CONSOLIDATION_INPUT };
    uint64_t maxScriptSizePolicy { DEFAULT_MAX_SCRIPT_SIZE_POLICY };
    std::set<uint256> mInvalidBlocks;
    std::set<std::string> mBannedUAClients;

    void SetErrorMsg(std::string* err)
    {
        if (err)
        {
            *err = "This is dummy config";
        }
    }
};

#endif