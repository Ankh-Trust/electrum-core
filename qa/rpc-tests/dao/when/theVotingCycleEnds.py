#!/usr/bin/env python3
# Copyright (c) 2019 The NavCoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Expanded helper routines for regression testing of the 0AE Coin Ankh fund
#

from test_framework.util import *

def whenTheVotingCycleEnds(node=None,
cycles=1):

  if (node is None
  or cycles < -1):
    print('whenTheVotingCycleEnds: invalid parameters')
    assert(False)

  try:
    blocksRemaining = node.fundstats()["votingPeriod"]["ending"] - node.fundstats()["votingPeriod"]["current"]
    periodStart = node.fundstats()["votingPeriod"]["starting"]
    periodEnd = node.fundstats()["votingPeriod"]["ending"]
  except JSONRPCException as e:
    print(e.error)
    assert(False)

  assert(node.getblockcount() >= periodStart)
  assert(node.getblockcount() <= periodEnd)

  slow_gen(node, blocksRemaining)

  assert(node.getblockcount() == periodEnd)

  # parsing -1 will end a full round of proposal voting cycles
  if (cycles == -1):
    cycles = node.fundstats()["consensus"]["maxCountVotingCycleProposals"] + 1

  if (cycles > 1):
    blocksPerCycle = node.fundstats()["consensus"]["blocksPerVotingCycle"]
    for i in range(1, cycles):
      slow_gen(node, blocksPerCycle)
      assert(node.getblockcount() == periodEnd + (i * blocksPerCycle))

  slow_gen(node, 1) # proceed to the first block of the next cycle
