#!/usr/bin/env python3
# Copyright (c) 2019 The NavCoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Expanded helper routines for regression testing of the 0AE Coin Ankh fund
#

from test_framework.util import *

def givenIHaveDonatedToTheCFund(node=None,
amount=None):

  if (node is None
  or amount is None):
    print('givenIHaveDonatedToTheCFund: invalid parameters')
    assert(False)

  availableBefore = node.fundstats()["funds"]["available"]

  try:
    node.donatefund(amount)
  except JSONRPCException as e:
    print(e.error)
    assert(False)

  slow_gen(node, 1)
  assert(node.fundstats()["funds"]["available"] == availableBefore + amount)
