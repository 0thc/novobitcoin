// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "protocol.h"
#include "config.h"
#include "util.h"
#include "utilstrencodings.h"

#ifndef WIN32
#include <arpa/inet.h>
#endif

namespace NetMsgType {
const char *VERSION = "version";
const char *VERACK = "verack";
const char *ADDR = "addr";
const char *INV = "inv";
const char *GETDATA = "getdata";
const char *MERKLEBLOCK = "merkleblock";
const char *GETBLOCKS = "getblocks";
const char *GETHEADERS = "getheaders";
const char *TX = "tx";
const char *HEADERS = "headers";
const char *BLOCK = "block";
const char *GETADDR = "getaddr";
const char *MEMPOOL = "mempool";
const char *PING = "ping";
const char *PONG = "pong";
const char *NOTFOUND = "notfound";
const char *FILTERLOAD = "filterload";
const char *FILTERADD = "filteradd";
const char *FILTERCLEAR = "filterclear";
const char *REJECT = "reject";
const char *SENDHEADERS = "sendheaders";
const char *FEEFILTER = "feefilter";
const char *SENDCMPCT = "sendcmpct";
const char *CMPCTBLOCK = "cmpctblock";
const char *GETBLOCKTXN = "getblocktxn";
const char *BLOCKTXN = "blocktxn";
const char *PROTOCONF = "protoconf";
const char *CREATESTREAM = "createstrm";
const char *STREAMACK = "streamack";

bool IsBlockLike(const std::string &strCommand) {
    return strCommand == NetMsgType::BLOCK ||
           strCommand == NetMsgType::CMPCTBLOCK ||
           strCommand == NetMsgType::BLOCKTXN;
}

uint64_t GetMaxMessageLength(const std::string& command, const Config& config, uint64_t maxBlockSize)
{
    if (command == NetMsgType::PROTOCONF)
    {
        // If the message is PROTOCONF, it is limited to LEGACY_MAX_PROTOCOL_PAYLOAD_LENGTH.
        return LEGACY_MAX_PROTOCOL_PAYLOAD_LENGTH;
    }
    else if (command == NetMsgType::TX)
    {
        // If the message is TX, it is limited to max consensus tx size
        // can not use policy limit because of banning rules.
        return config.GetMaxTxSize(true);
    }
    else if (command == NetMsgType::GETBLOCKTXN)
    {
        // Minimum realistic transaction size in bytes
        static constexpr unsigned MIN_TX_SIZE { 215 };
        // Short TXID size in bytes
        static constexpr unsigned SHORT_TXID_SIZE { 6 };

        // If the message is GETBLOCKTXN, it is limited to an estimate of the maximum number of
        // short TXIDs the message could contain.
        return (maxBlockSize / MIN_TX_SIZE * SHORT_TXID_SIZE) + CMessageHeader::HEADER_SIZE;
    }
    else if (!NetMsgType::IsBlockLike(command))
    {
        // If the message doesn't not contain a block content,
        // it is limited to MAX_PROTOCOL_RECV_PAYLOAD_LENGTH.
        return config.GetMaxProtocolRecvPayloadLength();
    }
    else
    {
        // Maximum accepted block type message size
        return maxBlockSize;
    }
}

} // namespace NetMsgType

/**
 * All known message types. Keep this in the same order as the list of messages
 * above and in protocol.h.
 */
static const std::string allNetMessageTypes[] = {
    NetMsgType::VERSION,      NetMsgType::VERACK,     NetMsgType::ADDR,
    NetMsgType::INV,          NetMsgType::GETDATA,    NetMsgType::MERKLEBLOCK,
    NetMsgType::GETBLOCKS,    NetMsgType::GETHEADERS, NetMsgType::TX,
    NetMsgType::HEADERS,      NetMsgType::BLOCK,      NetMsgType::GETADDR,
    NetMsgType::MEMPOOL,      NetMsgType::PING,       NetMsgType::PONG,
    NetMsgType::NOTFOUND,     NetMsgType::FILTERLOAD, NetMsgType::FILTERADD,
    NetMsgType::FILTERCLEAR,  NetMsgType::REJECT,     NetMsgType::SENDHEADERS,
    NetMsgType::FEEFILTER,    NetMsgType::SENDCMPCT,  NetMsgType::CMPCTBLOCK,
    NetMsgType::GETBLOCKTXN,  NetMsgType::BLOCKTXN,   NetMsgType::PROTOCONF,
    NetMsgType::CREATESTREAM, NetMsgType::STREAMACK,
};
static const std::vector<std::string>
    allNetMessageTypesVec(allNetMessageTypes,
                          allNetMessageTypes + ARRAYLEN(allNetMessageTypes));

CMessageHeader::CMessageHeader(const MessageMagic &pchMessageStartIn) {
    memcpy(pchMessageStart.data(), pchMessageStartIn.data(),
           MESSAGE_START_SIZE);
    memset(pchCommand, 0, sizeof(pchCommand));
    nPayloadLength = -1;
    memset(pchChecksum, 0, CHECKSUM_SIZE);
}

CMessageHeader::CMessageHeader(const MessageMagic &pchMessageStartIn,
                               const char *pszCommand,
                               unsigned int nPayloadLengthIn) {
    memcpy(pchMessageStart.data(), pchMessageStartIn.data(),
           MESSAGE_START_SIZE);
    memset(pchCommand, 0, sizeof(pchCommand));
    GCC_WARNINGS_PUSH;
    #if defined __GNUC__ && (__GNUC__ >= 8)
        // -Wstringop-truncation was introduced in GCC 8
        GCC_WARNINGS_IGNORE(-Wstringop-truncation);
    #endif
    // length of pszCommand is always smaller than COMMAND_SIZE
    strncpy(pchCommand, pszCommand, COMMAND_SIZE);
    GCC_WARNINGS_POP;

    nPayloadLength = nPayloadLengthIn;
    memset(pchChecksum, 0, CHECKSUM_SIZE);
}

std::string CMessageHeader::GetCommand() const {
    return std::string(pchCommand,
                       pchCommand + strnlen(pchCommand, COMMAND_SIZE));
}

static bool
CheckHeaderMagicAndCommand(const CMessageHeader &header,
                           const CMessageHeader::MessageMagic &magic) {
    // Check start string
    if (memcmp(header.pchMessageStart.data(), magic.data(),
               CMessageHeader::MESSAGE_START_SIZE) != 0) {
        return false;
    }

    // Check the command string for errors
    for (const char *p1 = header.pchCommand;
         p1 < header.pchCommand + CMessageHeader::COMMAND_SIZE; p1++) {
        if (*p1 == 0) {
            // Must be all zeros after the first zero
            for (; p1 < header.pchCommand + CMessageHeader::COMMAND_SIZE;
                 p1++) {
                if (*p1 != 0) {
                    return false;
                }
            }
        } else if (*p1 < ' ' || *p1 > 0x7E) {
            return false;
        }
    }

    return true;
}

bool CMessageHeader::IsValid(const Config &config) const {
    // Check start string
    if (!CheckHeaderMagicAndCommand(*this,
                                    config.GetChainParams().NetMagic())) {
        return false;
    }

    // Message size
    uint64_t maxBlockSize = config.GetMaxBlockSize();
    if (IsOversized(config, maxBlockSize)) {
        LogPrintf("CMessageHeader::IsValid(): (%s, %u bytes) is oversized\n",
                  GetCommand(), nPayloadLength);
        return false;
    }

    return true;
}

bool CMessageHeader::IsOversized(const Config &config, uint64_t maxBlockSize) const
{
    return nPayloadLength > NetMsgType::GetMaxMessageLength(GetCommand(), config, maxBlockSize);
}

CAddress::CAddress(CService ipIn, ServiceFlags nServicesIn) : CService{ipIn}, nServices{nServicesIn} {
}

std::string CInv::GetCommand() const {
    std::string cmd;
    switch (GetKind()) {
        case MSG_TX:
            return cmd.append(NetMsgType::TX);
        case MSG_BLOCK:
            return cmd.append(NetMsgType::BLOCK);
        case MSG_FILTERED_BLOCK:
            return cmd.append(NetMsgType::MERKLEBLOCK);
        case MSG_CMPCT_BLOCK:
            return cmd.append(NetMsgType::CMPCTBLOCK);
        default:
            throw std::out_of_range(
                strprintf("CInv::GetCommand(): type=%d unknown type", type));
    }
}

std::string CInv::ToString() const {
    try {
        return strprintf("%s %s", GetCommand(), hash.ToString());
    } catch (const std::out_of_range &) {
        return strprintf("0x%08x %s", type, hash.ToString());
    }
}

const std::vector<std::string> &getAllNetMessageTypes() {
    return allNetMessageTypesVec;
}
