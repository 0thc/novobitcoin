// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2018-2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.



#include "script.h"
#include "consensus/consensus.h"
#include "instruction_iterator.h"
#include "int_serialization.h"
#include "script_num.h"
#include "utilstrencodings.h"
#include <algorithm>
#include <sstream>

uint64_t CScript::GetSigOpCount(bool& sigOpCountError) const
{
    sigOpCountError = false;
    uint64_t n = 0;
    novo::instruction last_instruction{OP_INVALIDOPCODE};
    const auto it_end{end_instructions()};
    for(auto it{begin_instructions()}; it != it_end; ++it)
    {
        opcodetype lastOpcode{last_instruction.opcode()};

        opcodetype opcode{it->opcode()};
        if(it->opcode() == OP_INVALIDOPCODE)
            break;

        if(opcode == OP_CHECKSIG || opcode == OP_CHECKSIGVERIFY)
        {
            n++;
        }
        else if(opcode == OP_CHECKMULTISIG ||
            opcode == OP_CHECKMULTISIGVERIFY)
        {
            if (lastOpcode >= OP_1 && lastOpcode <= OP_16)
            {
                n += DecodeOP_N(lastOpcode);
            }
            // we always count accurate ops because it's not significantly costlier
            else
            {
                if (lastOpcode == OP_0)
                {
                    // Checking multisig with 0 keys, so nothing to add to n
                }
                else if(last_instruction.operand().size() > CScriptNum::MAXIMUM_ELEMENT_SIZE)
                {
                    // When trying to spend such output EvalScript does not allow numbers bigger than 4 bytes
                    // and the execution of such script would fail and make the coin unspendable
                    sigOpCountError = true;
                    return 0;
                }
                else
                {
                    //  When trying to spend such output EvalScript requires minimal encoding
                    //  and would fail the script if number is not minimally encoded
                    //  We check minimal encoding before calling CScriptNum to avoid
                    //  exception in CScriptNum constructor.
                    if(!novo::IsMinimallyEncoded(
                           last_instruction.operand(),
                           CScriptNum::MAXIMUM_ELEMENT_SIZE))
                    {
                        sigOpCountError = true;
                        return 0;
                    }

                    int numSigs =
                        CScriptNum(last_instruction.operand(), true).getint();
                    if(numSigs < 0)
                    {
                        sigOpCountError = true;
                        return 0;
                    }
                    n += numSigs;
                }
            }
        }
        last_instruction = *it;
    }

    return n;
}

bool IsP2SH(const novo::span<const uint8_t> script) {
    // Extra-fast test for pay-to-script-hash CScripts:
    return script.size() == 23 && script[0] == OP_HASH160 &&
           script[1] == 0x14 && script[22] == OP_EQUAL;
}

bool IsDustReturnScript (novo::span<const uint8_t> script)
{
    // OP_FALSE, OP_RETURN, OP_PUSHDATA, 'dust'
    static constexpr std::array<uint8_t, 7> dust_return = {0x00,0x6a,0x04,0x64,0x75,0x73,0x74};
    if (script.size() != dust_return.size())
        return false;

    return std::equal(script.begin(), script.end(), dust_return.begin());
}

bool CScript::IsPushOnly(const_iterator pc) const {
    while (pc < end()) {
        opcodetype opcode;
        if (!GetOp(pc, opcode)) return false;
        // Note that IsPushOnly() *does* consider OP_RESERVED to be a push-type
        // opcode, however execution of OP_RESERVED fails, so it's not relevant
        // to P2SH/BIP62 as the scriptSig would fail prior to the P2SH special
        // validation code being executed.
        if (opcode > OP_16) return false;
    }
    return true;
}

bool CScript::IsPushOnly() const {
    return this->IsPushOnly(begin());
}

bool CScript::GetStateIterator(iterator &pc) {
    size_t scriptLen = size();
	// opreturn + state + stateLen + version
	if (scriptLen < 1+0+4+1) {
		return false;
	}

    pc -= 5;

    unsigned int stateLen = ReadLE32(&pc[0]);
	if (scriptLen-1-4-1 < stateLen) {
		return false;
	}

    pc -= stateLen;
	if (*(pc-1) != OP_RETURN) {
		return false;
	}

    return true;
}

CScript &CScript::push_int64(int64_t n) {
    if (n == -1 || (n >= 1 && n <= 16)) {
        push_back(n + (OP_1 - 1));
    } else if (n == 0) {
        push_back(OP_0);
    } else {
        std::vector<uint8_t> v;
        v.reserve(sizeof(n));
        novo::serialize(n, back_inserter(v));
        *this << v;
    }
    return *this;
}

CScript &CScript::operator<<(const CScriptNum &b) {
    *this << b.getvch();
    return *this;
}

novo::instruction_iterator CScript::begin_instructions() const
{
    return novo::instruction_iterator{novo::span<const uint8_t>{data(), size()}};
}

novo::instruction_iterator CScript::end_instructions() const
{
    return novo::instruction_iterator{
        novo::span<const uint8_t>{data() + size(), 0}};
}

std::ostream& operator<<(std::ostream& os, const CScript& script)
{
    for(auto it = script.begin_instructions(); it != script.end_instructions();
        ++it)
    {
        os << *it << '\n';
    }

    return os;
}

// used for debugging and pretty-printing in gdb
std::string to_string(const CScript& s)
{
    std::ostringstream oss;
    oss << s;
    return oss.str();
}

size_t CountOp(const novo::span<const uint8_t> s, const opcodetype opcode)
{
    using namespace novo;
    instruction_iterator first{s};
    instruction_iterator last{s.last(0)};
    return std::count_if(first, last, [opcode](const instruction& inst) {
        return inst.opcode() == opcode;
    });
}
