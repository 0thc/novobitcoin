// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2017-2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef BITCOIN_SCRIPT_SCRIPTFLAGS_H
#define BITCOIN_SCRIPT_SCRIPTFLAGS_H

/** Script verification flags */
enum {
    SCRIPT_VERIFY_NONE = 0,

    // Passing a non-strict-DER signature or one with undefined hashtype to a
    // checksig operation causes script failure. Evaluating a pubkey that is not
    // (0x04 + 64 bytes) or (0x02 or 0x03 + 32 bytes) by checksig causes script
    // failure.
    SCRIPT_VERIFY_STRICTENC = (1U << 1),

    // Passing a non-strict-DER signature to a checksig operation causes script
    // failure (softfork safe, BIP62 rule 1)
    SCRIPT_VERIFY_DERSIG = (1U << 2),

    // Passing a non-strict-DER signature or one with S > order/2 to a checksig
    // operation causes script failure
    // (softfork safe, BIP62 rule 5).
    SCRIPT_VERIFY_LOW_S = (1U << 3),

    // verify dummy stack item consumed by CHECKMULTISIG is of zero-length
    // (softfork safe, BIP62 rule 7).
    SCRIPT_VERIFY_NULLDUMMY = (1U << 4),

    // Using a non-push operator in the scriptSig causes script failure
    // (softfork safe, BIP62 rule 2).
    SCRIPT_VERIFY_SIGPUSHONLY = (1U << 5),

    // Require minimal encodings for all push operations (OP_0... OP_16,
    // OP_1NEGATE where possible, direct pushes up to 75 bytes, OP_PUSHDATA up
    // to 255 bytes, OP_PUSHDATA2 for anything larger). Evaluating any other
    // push causes the script to fail (BIP62 rule 3). In addition, whenever a
    // stack element is interpreted as a number, it must be of minimal length
    // (BIP62 rule 4).
    // (softfork safe)
    SCRIPT_VERIFY_MINIMALDATA = (1U << 6),

    // Discourage use of NOPs reserved for upgrades (NOP1-10)
    //
    // Provided so that nodes can avoid accepting or mining transactions
    // containing executed NOP's whose meaning may change after a soft-fork,
    // thus rendering the script invalid; with this flag set executing
    // discouraged NOPs fails the script. This verification flag will never be a
    // mandatory flag applied to scripts in a block. NOPs that are not executed,
    // e.g.  within an unexecuted IF ENDIF block, are *not* rejected.
    SCRIPT_VERIFY_DISCOURAGE_UPGRADABLE_NOPS = (1U << 7),

    // Require that only a single stack element remains after evaluation. This
    // changes the success criterion from "At least one stack element must
    // remain, and when interpreted as a boolean, it must be true" to "Exactly
    // one stack element must remain, and when interpreted as a boolean, it must
    // be true".
    // (softfork safe, BIP62 rule 6)
    SCRIPT_VERIFY_CLEANSTACK = (1U << 8),

    // remove BIP65/68/112

    // Require the argument of OP_IF/NOTIF to be exactly 0x01 or empty vector
    //
    SCRIPT_VERIFY_MINIMALIF = (1U << 13),

    // Signature(s) must be empty vector if an CHECK(MULTI)SIG operation failed
    //
    SCRIPT_VERIFY_NULLFAIL = (1U << 14),

    // Public keys in scripts must be compressed
    //
    SCRIPT_VERIFY_COMPRESSED_PUBKEYTYPE = (1U << 15),

    // Not actual flag. Used for marking largest flag value.
    SCRIPT_FLAG_LAST = (1U << 16)
};

#endif // BITCOIN_SCRIPT_SCRIPTFLAGS_H
