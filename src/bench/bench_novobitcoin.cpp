// Copyright (c) 2015-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Bitcoin developers
// Copyright (c) 2021-2022 The Novo Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "bench.h"
#include "crypto/sha256.h"
#include "key.h"
#include "random.h"
#include "util.h"

int main(int argc, char** argv)
{
    SHA256AutoDetect();
    RandomInit();
    SetupEnvironment();

    // don't want to write to debug.log file
    GetLogger().fPrintToDebugLog = false;

    benchmark::BenchRunner::RunAll();
}
