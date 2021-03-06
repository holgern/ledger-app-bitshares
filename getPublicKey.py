#!/usr/bin/env python3
"""
/*******************************************************************************
*  Copyright of the Contributing Authors; see CONTRIBUTORS.md.
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/
"""

import struct
from ledgerblue.comm import getDongle
import argparse
import hashlib
from base58 import b58encode
import binascii

def parse_bip32_path(path):
    if len(path) == 0:
        return bytes([])
    result = bytes([])
    elements = path.split('/')
    for pathElement in elements:
        element = pathElement.split('\'')
        if len(element) == 1:
            result = result + struct.pack(">I", int(element[0]))
        else:
            result = result + struct.pack(">I", 0x80000000 | int(element[0]))
    return result

parser = argparse.ArgumentParser()
parser.add_argument('--path', help="BIP 32 path to retrieve")
args = parser.parse_args()

if args.path is None:
    args.path = "48'/1'/1'/0'/0'"

donglePath = parse_bip32_path(args.path)
apdu = binascii.unhexlify("B5020001" + "{:02x}".format(len(donglePath) + 1) + "{:02x}".format(int(len(donglePath) / 4))) + donglePath

dongle = getDongle(True)
result = dongle.exchange(apdu)
offset = 1 + result[0]
address = result[offset + 1: offset + 1 + result[offset]]

public_key = result[1: 1 + result[0]]
head = 0x03 if (public_key[64] & 0x01) == 1 else 0x02
public_key_compressed = bytearray([head]) + public_key[1:33]

print ("           Public key " + binascii.hexlify(public_key).decode())
print ("Public key compressed " + binascii.hexlify(public_key_compressed).decode())

ripemd = hashlib.new('ripemd160')
ripemd.update(public_key_compressed)
check = ripemd.digest()[:4]

buff = public_key_compressed + check
print ("Calculated from public key: Address BTS" + b58encode(bytes(buff)).decode())
print ("      Received from ledger: Address " + address.decode())

apdu = binascii.unhexlify("B5020101" + "{:02x}".format(len(donglePath) + 1) + "{:02x}".format(int(len(donglePath) / 4))) + donglePath
result = dongle.exchange(apdu)
