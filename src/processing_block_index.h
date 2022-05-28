// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_PROCESSINGBLOCKINDEX_H
#define BITCOIN_PROCESSINGBLOCKINDEX_H

#include "primitives/block.h"
#include "undo.h"

class ProcessingBlockIndex
{
public:
    template<typename T> struct UnitTestAccess;

    ProcessingBlockIndex( CBlockIndex& index ) : mIndex(index) {}

    DisconnectResult DisconnectBlock(
        const CBlock& block,
        CCoinsViewCache& view,
        const task::CCancellationToken& shutdownToken) const;

private:

    DisconnectResult ApplyBlockUndo(
        const CBlockUndo& blockUndo,
        const CBlock& block,
        CCoinsViewCache& view,
        const task::CCancellationToken& shutdownToken) const;

    CBlockIndex& mIndex;
};

#endif
