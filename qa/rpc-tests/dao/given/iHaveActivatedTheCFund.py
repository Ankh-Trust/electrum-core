#!/usr/bin/env python3
# Copyright (c) 2019 The NavCoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Expanded helper routines for regression testing of the 0AE Coin Ankh fund
#

from test_framework.util import *

def givenIHaveActivatedTheCFund(node=None,
text=None,
questions=None,
withAnswers=False):

  if (node is None):
    print('givenIHaveActivatedTheCFund: invalid parameters')
    assert(False)

  if (get_bip9_status(node, "ankhfund")["status"] == "defined"):
    slow_gen(node, 100)
    # Verify the Ankh Fund is started
    assert (get_bip9_status(node, "ankhfund")["status"] == "started")

  if (get_bip9_status(node, "ankhfund")["status"] == "started"):
    slow_gen(node, 100)
    # Verify the Ankh Fund is locked_in
    assert (get_bip9_status(node, "ankhfund")["status"] == "locked_in")

  if (get_bip9_status(node, "ankhfund")["status"] == "locked_in"):
    slow_gen(node, 100)
    # Verify the Ankh Fund is active
    assert (get_bip9_status(node, "ankhfund")["status"] == "active")
