// Copyright (c) 2017 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_RPC_MINING_H
#define BITCOIN_RPC_MINING_H

#include <univalue.h>
#include <functional>
#include <memory>

#include "consensus/params.h"

class Config;
class CBlock;
class CReserveScript;
class CBlockIndex;

/** Generate blocks (mine) */
UniValue generateBlocks(const Config& config,
                        std::shared_ptr<CReserveScript> coinbaseScript,
                        int nGenerate, uint64_t nMaxTries, bool keepScript);

UniValue processBlock(const Config& config,
	const std::shared_ptr<CBlock>& block,
	std::function<bool(const Config&, const std::shared_ptr<CBlock>&)> performBlockOperation);

/** Modify the extranonce in a block */
void IncrementExtraNonce(CBlock *pblock,
                         const CBlockIndex *pindexPrev,
                         unsigned int &nExtraNonce);
#endif
