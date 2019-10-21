// Copyright (c) 2018 The NAVcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ELECTRUM_POS_H
#define ELECTRUM_POS_H

#define COMMUNITY_FUND_AMOUNT 25000000

static const int STAKE_TIMESTAMP_MASK = 15;

double GetDifficulty(const CBlockIndex* blockindex);

double GetPoWMHashPS();

double GetPoSKernelPS();

extern uint64_t nLastCoinStakeSearchInterval;

#endif // ELECTRUM_POS_H
