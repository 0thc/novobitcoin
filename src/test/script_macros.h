// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

// Preproccessor definitions to simplify testing

#define PUB_KEY 33, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, \
 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33

#define PUB_KEY_2 PUB_KEY, PUB_KEY
#define PUB_KEY_4 PUB_KEY_2, PUB_KEY_2
#define PUB_KEY_8 PUB_KEY_4, PUB_KEY_4
#define PUB_KEY_16 PUB_KEY_8, PUB_KEY_8
#define PUB_KEY_32 PUB_KEY_16, PUB_KEY_16

#define SCRIPT_HASH 20, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20
#define P2SH_LOCKING OP_HASH160, SCRIPT_HASH, OP_EQUAL

#define MULTISIG_LOCKING_2 OP_1, PUB_KEY_2, OP_2, OP_CHECKMULTISIG
#define MULTISIG_LOCKING_4 OP_1, PUB_KEY_4, OP_4, OP_CHECKMULTISIG
#define MULTISIG_LOCKING_8 OP_1, PUB_KEY_8, OP_8, OP_CHECKMULTISIG
#define MULTISIG_LOCKING_16 OP_1, PUB_KEY_16, OP_16, OP_CHECKMULTISIG
#define MULTISIG_LOCKING_32 OP_1, PUB_KEY_32, 1, 32, OP_CHECKMULTISIG

#define MULTISIG_LOCKING_20 OP_1, PUB_KEY_16, PUB_KEY_4, 1, 20, OP_CHECKMULTISIG
#define MULTISIG_LOCKING_21 OP_1, PUB_KEY_16, PUB_KEY_4, PUB_KEY, 1, 21, OP_CHECKMULTISIG

#define MULTISIG_2_IF_LOCKING MULTISIG_LOCKING_2, OP_IF, OP_CHECKSIG, OP_ENDIF
