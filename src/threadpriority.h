// Copyright (c) 2018-2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#pragma once

#include <enum_cast.h>

// Some pre-defined thread priority levels.
enum class ThreadPriority : int
{
    Low = 0,
    Normal = 1,
    High = 2
};

// Enable enum_cast for ThreadPriority, so we can log informatively
const enumTableT<ThreadPriority>& enumTable(ThreadPriority);
