// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

#include <cstdint>

/**
 * Configuration interface that contains limits used when evaluating scripts.
 * Class must be defined outside config.h as it is used by a dynamic library
 * (libbitcoinconsensus) which is not connected to the rest of bitcoin code.
 */
class CScriptConfig
{
public:
    virtual uint64_t GetMaxOpsPerScript(bool isConsensus) const = 0;
    virtual uint64_t GetMaxScriptNumLength(bool isConsensus) const = 0;
    virtual uint64_t GetMaxScriptSize(bool isConsensus) const = 0;
    virtual uint64_t GetMaxPubKeysPerMultiSig(bool isConsensus) const = 0;
    virtual uint64_t GetMaxStackMemoryUsage(bool isConsensus) const = 0;

protected:
    ~CScriptConfig() = default;
};
