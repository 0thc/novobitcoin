#!/usr/bin/env python3
# Copyright (c) 2018 The Bitcoin developers
# Copyright (c) 2019 The Bitcoin developers
# Copyright (c) 2021-2022 The Novo Bitcoin developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

import os
import sys


def main(test_name, input_file):
    with open(input_file, "rb") as f:
        contents = f.read()

    print("namespace json_tests{")
    print("   static unsigned const char {}[] = {{ // {}".format(test_name, len(contents)))

    for i in range((len(contents)+7)//8):
        print(" ".join(map(lambda x: "0x{:02x},".format(x), contents[i*8:i*8+8])))

    print(" };")
    print("};")


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("We need additional pylons!")
        os.exit(1)

    main(sys.argv[1], sys.argv[2])
