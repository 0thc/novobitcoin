// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.



#include "random.h"
#include "txhasher.h"

const uint64_t StaticHasherSalt::k0{GetRand(std::numeric_limits<uint64_t>::max())};
const uint64_t StaticHasherSalt::k1{GetRand(std::numeric_limits<uint64_t>::max())};
