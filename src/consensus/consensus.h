// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2018-2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_CONSENSUS_CONSENSUS_H
#define BITCOIN_CONSENSUS_CONSENSUS_H

#include <cstdint>

/** 1KB */
static const uint64_t ONE_KILOBYTE = 1000;
/** 1MB */
static const uint64_t ONE_MEGABYTE = ONE_KILOBYTE * 1000;
/** 1GB */
static const uint64_t ONE_GIGABYTE = ONE_MEGABYTE * 1000;
/** 1KiB = 1024 */
static const uint64_t ONE_KIBIBYTE = 1024;
/** 1MiB = 1024 * 1024 */
static const uint64_t ONE_MEBIBYTE = ONE_KIBIBYTE * 1024;
/** 1GiB = 1024 * 1024 * 1024 */
static const uint64_t ONE_GIBIBYTE = ONE_MEBIBYTE * 1024;
/** 1 hour in seconds */
static constexpr unsigned SECONDS_IN_ONE_HOUR { 60 * 60 };
/** The maximum allowed size for a transaction, in bytes */
static const uint64_t MAX_TX_SIZE_CONSENSUS = 8 * ONE_MEGABYTE;
/** The minimum allowed size for a transaction, in bytes, longer than 2 sha256 */
static const uint64_t MIN_TX_SIZE_CONSENSUS = 32 + 32 + 1;
/** The maximum allowed size for a block */
static const uint64_t LEGACY_MAX_BLOCK_SIZE = 1 * ONE_MEGABYTE;

/**
 * The maximum allowed number of signature check operations per MB in a block
 * (network rule).
 */
static const uint64_t MAX_BLOCK_SIGOPS_PER_MB = 75000;

/** Allowed number of signature check operations per transaction */
static const uint64_t MAX_TX_SIGOPS_COUNT = MAX_BLOCK_SIGOPS_PER_MB/5;

// Maximum number of non-push operations per script
static const uint64_t MAX_OPS_PER_SCRIPT = UINT32_MAX;

// Maximum number of public keys per multisig
// Actual maximum number of public keys that can be stored in script of
// maximum length is actually smaller (since each compressed pub key takes up 33 bytes).
static const uint64_t MAX_PUBKEYS_PER_MULTISIG = UINT32_MAX;

// Maximum script length in bytes
static const uint64_t MAX_SCRIPT_SIZE = 8*ONE_MEGABYTE;

// Minimum script number length
static const uint64_t MIN_SCRIPT_NUM_LENGTH = 4;
// Maximum script number length
static const uint64_t MAX_SCRIPT_NUM_LENGTH = 750 * ONE_KILOBYTE;

// Maximum coinbase scriptSig size
static const uint64_t MAX_COINBASE_SCRIPTSIG_SIZE = 100;

/**
 * Coinbase transaction outputs can only be spent after this number of new
 * blocks (network rule).
 */
static const int COINBASE_MATURITY = 100;

/** Maximum stack memory usage (used instead of MAX_SCRIPT_ELEMENT_SIZE and MAX_STACK_ELEMENTS). **/
static const uint64_t DEFAULT_STACK_MEMORY_USAGE_CONSENSUS = INT64_MAX;

#endif // BITCOIN_CONSENSUS_CONSENSUS_H
