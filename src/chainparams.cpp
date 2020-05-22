// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>
#include <streams.h>

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << CScriptNum(0) << CScriptNum(42) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    // txNew.vout[0].scriptPubKey.clear();
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey.clear();
    txNew.strDZeel = "Electrum genesis block";

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.vtx[0].nTime = nTime;
    genesis.vtx[0].UpdateHash();
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "BBC News 20 Sep: Trump denies promise that led to formal complaint from intelligence official";
    const CScript genesisOutputScript = CScript() << ParseHex("0464c57bba1ef866f7551bada31f6320da513fea13d56c7926d03e8449981b4f64f301043b509b7f98bce6180e93884e530f5a56333d7ac84a26f6a25f0dd09a77") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock CreateGenesisBlockTestnet(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "BBC News 20 Sep: Climate change: Arctic expedition to drift in sea-ice for a year";
    const CScript genesisOutputScript = CScript() << ParseHex("043ebea3223e70f2c8205ecf2c32f567fbca8bf8e74e1138f6090c4cd442589c69f719199b4a12a42bd8022eed0541e43744e387bff564dd28dba51da146170626") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x00007db8870afcd774ef9851107340256e899d58b0db163bc782f2128a4c33ed");
        consensus.powLimit = ArithToUint256(~arith_uint256(0) >> 16);
        consensus.nPowTargetTimespan = 30;
        consensus.nPowTargetSpacing = 30;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 15120; // 75% of 20160
        consensus.nMinerConfirmationWindow = 20160;
        consensus.nStakeMinAge = 60 * 60 * 2;	// minimum for coin age: 2 hours
        consensus.nTargetSpacing = 30; // Blocktime: 30 secs
        consensus.nStakeCombineThreshold = 1000 * COIN;
        consensus.nStakeSplitThreshold = 2 * consensus.nStakeCombineThreshold;
        consensus.nDailyBlockCount =  (24 * 60 * 60) / consensus.nTargetSpacing;
        consensus.nModifierInterval = 10 * 60; // time to elapse before new modifier is computed
        consensus.nTargetTimespan = 25 * 30;
        consensus.nLastPOWBlock = 1000;
        consensus.nBlocksPerVotingCycle = 2880 * 7; // 7 Days
        consensus.nMinimumQuorum = 0.5;
        consensus.nMinimumQuorumFirstHalf = 0.5;
        consensus.nMinimumQuorumSecondHalf = 0.4;
        consensus.nVotesAcceptProposal = 0.7;
        consensus.nVotesRejectProposal = 0.7;
        consensus.nVotesAcceptPaymentRequest = 0.7;
        consensus.nVotesRejectPaymentRequest = 0.7;
        consensus.nCommunityFundMinAge = 50;
        consensus.nProposalMinimalFee = 5000000000;
        consensus.sigActivationTime = 1567296000;
        consensus.nCoinbaseTimeActivationHeight = 0;
        consensus.nBlockSpreadCFundAccumulation = 500;
        consensus.nCommunityFundAmount = 0.1 * COIN;
        consensus.nCommunityFundAmountV2 = 0.1 * COIN;
        consensus.nCyclesProposalVoting = 6;
        consensus.nCyclesPaymentRequestVoting = 8;
        consensus.nPaymentRequestMaxVersion = 3;
        consensus.nProposalMaxVersion = 3;
        consensus.nMaxFutureDrift = 60;
        consensus.nStaticReward = 1.0 * COIN;
        consensus.nHeightv451Fork = 1;
        consensus.nHeightv452Fork = 2;
        consensus.fDaoClientActivated = false;

        /** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
        consensus.nCoinbaseMaturity = 50;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT_LEGACY].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT_LEGACY].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT_LEGACY].nTimeout = 1598918400; // September 1st, 2020

        consensus.vDeployments[Consensus::DEPLOYMENT_CSV_LEGACY].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV_LEGACY].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV_LEGACY].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Cold Staking
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nStartTime = 4070908800; // Disabled
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nTimeout = 4070908800; // Disabled

        // Deployment of Community Fund Accumulation
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nStartTime = 4070908800; // Disabled
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nTimeout = 4070908800; // Disabled

        // Deployment of NTP Sync
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Community Fund Accumulation Spread(NPIP-0003)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].bit = 14;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nStartTime = 4070908800; // Disabled
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nTimeout = 4070908800; // Disabled

        // Increase in Community Fund Accumulation Ammonut (NPIP-0004)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].bit = 16;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nStartTime = 4070908800; // Disabled
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nTimeout = 4070908800; // Disabled

        // Deployment of Static Reward
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].bit = 15;
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Quorum reduction for the Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].bit = 17;
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nStartTime = 4070908800; // Disabled
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nTimeout = 4070908800; // Disabled

        // Deployment of Cold Staking Pool Fee
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].bit = 18;
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nStartTime = 4070908800; // Disabled
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nTimeout = 4070908800; // Disabled


        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x84;
        pchMessageStart[1] = 0x54;
        pchMessageStart[2] = 0x38;
        pchMessageStart[3] = 0x24;
        nDefaultPort = 44840;
        nPruneAfterHeight = 100000;
        bnProofOfWorkLimit = arith_uint256(~arith_uint256() >> 16);

        uint32_t nTimestamp = 1569005000;
        uint256 hashGenesisBlock = uint256S("0x00007db8870afcd774ef9851107340256e899d58b0db163bc782f2128a4c33ed");
        uint256 hashMerkleRoot = uint256S("0x77242bb9675ba1000fb144eb0bc92542c4fc9a34a9d1ee328dbbd987a3de7e2c");
        uint32_t nNonce = 2043738044;

        genesis = CreateGenesisBlock(nTimestamp, nNonce, 0x1f00ffff, 1, 0);
	consensus.hashGenesisBlock = genesis.GetHash();

        if (true && (genesis.GetHash() != hashGenesisBlock || genesis.hashMerkleRoot != hashMerkleRoot))
        {
            printf("recalculating params for mainnet.\n");
            printf("old mainnet genesis nonce: %d\n", genesis.nNonce);
            printf("old mainnet genesis hash:  %s\n", hashGenesisBlock.ToString().c_str());
            // deliberately empty for loop finds nonce value.
            for(; genesis.GetHash() > consensus.powLimit; genesis.nNonce++){ }
            printf("new mainnet genesis merkle root: %s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("new mainnet genesis nonce: %d\n", genesis.nNonce);
            printf("new mainnet genesis hash: %s\n", genesis.GetHash().ToString().c_str());
        }

        assert(consensus.hashGenesisBlock == hashGenesisBlock);
        assert(genesis.hashMerkleRoot == hashMerkleRoot);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,33);
        base58Prefixes[COLDSTAKING_ADDRESS] = std::vector<unsigned char>(1,21); // cold staking addresses start with 'X'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,85);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,150);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // vSeeds.push_back(CDNSSeedData("", ""));
        // vSeeds.push_back(CDNSSeedData("", ""));

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0, uint256S("0x00007db8870afcd774ef9851107340256e899d58b0db163bc782f2128a4c33ed"))
            ( 250, uint256S("0x668e05f32eaa6ff639b157abfaf9fe4972770a92ed7c69d566b3f4ec00d8d015"))
            ( 500, uint256S("0xa6dd84d1f35b15aae402e72a7ff4f18dca08ac001f2b95cbb4588bdfa16eeb08"))
            ( 1000, uint256S("0x8a75993a531726f54542bdcaffbfed023bfe97f99d9c4f025eb749c4d2ded54e"))
            ( 2500, uint256S("0x5951c151346fb911a77a9247e0a2bfa4fd6af39dc53c64f7e0221c55b8d7bf7c"))
            ( 5000, uint256S("0xee6193d758165b0347cff9a90228864ee1be3d6ddb6e34a19da18ae012977887"))
            ( 10000, uint256S("0x1cfd7579b47e09e574a4e215e2cf1d5d4447cd3ac47fc7b62b1015327b21481f"))
            ( 25000, uint256S("0xf2ea9180b7a93aa3529db3991e719673ea9d76bf876caf2f5fcb29b87b80c5e0"))
            ( 50000, uint256S("0x7a25b61cb771d98c71f91452e3af628c69d9d8055ea6d776c49c09d1763ecd3c"))
            ( 100000, uint256S("0xaabf04d3961c67fc6dbb09c7b898bde1523d035d5f5ed26db739659152a89d11"))
            ( 250000, uint256S("0xaabf04d3961c67fc6dbb09c7b898bde1523d035d5f5ed26db739659152a89d11")),
            1569005000, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            7000        // * estimated number of transactions per day after checkpoint
        };
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x00003a40545d761d6727b925c0d187b47335295b43163d075287fa7ff931e4fb");
        consensus.powLimit = ArithToUint256(~arith_uint256(0) >> 16);
        consensus.nPowTargetTimespan = 30;
        consensus.nPowTargetSpacing = 30;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 300; // 75% of 400
        consensus.nMinerConfirmationWindow = 400;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.nStakeMinAge = 2;	// minimum for coin age: 2 seconds
        consensus.nTargetSpacing = 30; // Blocktime: 30 secs
        consensus.nStakeCombineThreshold = 1000 * COIN;
        consensus.nStakeSplitThreshold = 2 * consensus.nStakeCombineThreshold;
        consensus.nDailyBlockCount =  (24 * 60 * 60) / consensus.nTargetSpacing;
        consensus.nModifierInterval = 10 * 60; // time to elapse before new modifier is computed
        consensus.nTargetTimespan = 25 * 30;
        consensus.nLastPOWBlock = 1000000;
        consensus.nBlocksPerVotingCycle = 180; // 1.5 hours
        consensus.nMinimumQuorum = 0.5;
        consensus.nMinimumQuorumFirstHalf = 0.5;
        consensus.nMinimumQuorumSecondHalf = 0.4;
        consensus.nVotesAcceptProposal = 0.7;
        consensus.nVotesRejectProposal = 0.7;
        consensus.nVotesAcceptPaymentRequest = 0.7;
        consensus.nVotesRejectPaymentRequest = 0.7;
        consensus.nCommunityFundMinAge = 5;
        consensus.nProposalMinimalFee = 10000;
        consensus.sigActivationTime = 1512826692;
        consensus.nCoinbaseTimeActivationHeight = 30000;
        consensus.nBlockSpreadCFundAccumulation = 500;
        consensus.nCommunityFundAmount = 0.25 * COIN;
        consensus.nCommunityFundAmountV2 = 0.5 * COIN;
        consensus.nCyclesProposalVoting = 4;
        consensus.nCyclesPaymentRequestVoting = 4;
        consensus.nPaymentRequestMaxVersion = 3;
        consensus.nProposalMaxVersion = 3;
        consensus.nMaxFutureDrift = 60;
        consensus.nStaticReward = 1 * COIN;
        consensus.nHeightv451Fork = 100000;
        consensus.nHeightv452Fork = 100000;
        consensus.fDaoClientActivated = true;

        /** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
        consensus.nCoinbaseMaturity = 50;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of Cold Staking
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 4;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of Community Fund Accumulation
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of NTP Sync
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nTimeout = 1556712000; // 1622548800; // Jun 1st, 2021

        // Deployment of Community Fund Accumulation Spread(NPIP-0003)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].bit = 14;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nTimeout = 1622548800; // Jun 1st, 2021

        // Increate in Community Fund Accumulation Ammonut (NPIP-0004)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].bit = 16;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of Static Reward
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].bit = 15;
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of Quorum reduction for the Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].bit = 17;
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nTimeout = 1622548800; // Jun 1st, 2021

        // Deployment of Cold Staking Pool Fee
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].bit = 18;
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nTimeout = 1622548800; // Jun 1st, 2021

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x43;
        pchMessageStart[1] = 0xa8;
        pchMessageStart[2] = 0x56;
        pchMessageStart[3] = 0x26;
        nDefaultPort = 45556;
        nPruneAfterHeight = 1000;
        bnProofOfWorkLimit = arith_uint256(~arith_uint256() >> 16);

        uint32_t nTimestamp = 1569005000;
        uint256 hashGenesisBlock = uint256S("0x00003a40545d761d6727b925c0d187b47335295b43163d075287fa7ff931e4fb");
        uint256 hashMerkleRoot = uint256S("0x82248aea7886a8a448e42cad754ceae4a25d95b024923f6b4dfb208bb479e27a");
        uint32_t nNonce = 2043802847;

        genesis = CreateGenesisBlockTestnet(nTimestamp, nNonce, 0x1d00ffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();

        if (true && (genesis.GetHash() != hashGenesisBlock || genesis.hashMerkleRoot != hashMerkleRoot))
        {
            printf("recalculating params for testnet.\n");
            printf("old testnet genesis nonce: %d\n", genesis.nNonce);
            printf("old testnet genesis hash:  %s\n", hashGenesisBlock.ToString().c_str());
            // deliberately empty for loop finds nonce value.
            for(; genesis.GetHash() > consensus.powLimit; genesis.nNonce++){ }
            printf("new testnet genesis merkle root: %s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("new testnet genesis nonce: %d\n", genesis.nNonce);
            printf("new testnet genesis hash: %s\n", genesis.GetHash().ToString().c_str());
        }

        // vSeeds.push_back(CDNSSeedData("", ""));
        // vSeeds.push_back(CDNSSeedData("", ""));

        assert(consensus.hashGenesisBlock == hashGenesisBlock);
        assert(genesis.hashMerkleRoot == hashMerkleRoot);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,94);
        base58Prefixes[COLDSTAKING_ADDRESS] = std::vector<unsigned char>(1,8); // cold staking addresses start with 'C/D'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x40)(0x88)(0x2B)(0xE1).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x40)(0x88)(0xDA)(0x4E).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0,     hashGenesisBlock),
            1569005000, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            7000         // * estimated number of transactions per day after checkpoint
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Devnet (v3)
 */
class CDevNetParams : public CChainParams {
public:
    CDevNetParams() {
        strNetworkID = "dev";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0000af88d8cae858d9e3e22d5931871e9eefd411984dc241e9bc2de1b7d88326");
        consensus.powLimit = ArithToUint256(~arith_uint256(0) >> 16);
        consensus.nPowTargetTimespan = 30;
        consensus.nPowTargetSpacing = 30;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 75; // 75% of 400
        consensus.nMinerConfirmationWindow = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.nStakeMinAge = 2;	// minimum for coin age: 2 seconds
        consensus.nTargetSpacing = 5; // Blocktime: 5 secs
        consensus.nStakeCombineThreshold = 1000 * COIN;
        consensus.nStakeSplitThreshold = 2 * consensus.nStakeCombineThreshold;
        consensus.nDailyBlockCount =  (24 * 60 * 60) / consensus.nTargetSpacing;
        consensus.nModifierInterval = 10 * 60; // time to elapse before new modifier is computed
        consensus.nTargetTimespan = 25 * 30;
        consensus.nLastPOWBlock = 100000;
        consensus.nBlocksPerVotingCycle = 30; // 15 minutes
        consensus.nMinimumQuorum = 0.5;
        consensus.nMinimumQuorumFirstHalf = 0.5;
        consensus.nMinimumQuorumSecondHalf = 0.4;
        consensus.nVotesAcceptProposal = 0.7;
        consensus.nVotesRejectProposal = 0.7;
        consensus.nVotesAcceptPaymentRequest = 0.7;
        consensus.nVotesRejectPaymentRequest = 0.7;
        consensus.nCommunityFundMinAge = 5;
        consensus.nProposalMinimalFee = 10000;
        consensus.sigActivationTime = 1512826692;
        consensus.nCoinbaseTimeActivationHeight = 0;
        consensus.nBlockSpreadCFundAccumulation = 500;
        consensus.nCommunityFundAmount = 0.25 * COIN;
        consensus.nCommunityFundAmountV2 = 0.5 * COIN;
        consensus.nCyclesProposalVoting = 4;
        consensus.nCyclesPaymentRequestVoting = 4;
        consensus.nPaymentRequestMaxVersion = 3;
        consensus.nProposalMaxVersion = 3;
        consensus.nMaxFutureDrift = 60000;
        consensus.nStaticReward = 1 * COIN;
        consensus.nHeightv451Fork = 1000;
        consensus.nHeightv452Fork = 1000;
        consensus.fDaoClientActivated = true;

        /** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
        consensus.nCoinbaseMaturity = 5;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1462060800; // May 1st, 2016
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Cold Staking
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nStartTime = 1525132800; // May 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1493424000; // May 1st, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nStartTime = 1493424000; // May 1st, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of NTP Sync
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nStartTime = 1525132800; // May 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Community Fund Accumulation
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nStartTime = 1525132800; // May 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Community Fund Accumulation Spread(NPIP-0003)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].bit = 14;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nStartTime = 1525132800; // May 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nTimeout = 1651363200; // May 1st, 2022

        // Increate in Community Fund Accumulation Ammonut (NPIP-0004)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].bit = 16;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nStartTime = 1533081600; // Aug 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Static Reward
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].bit = 15;
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nStartTime = 1533081600; // August 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Quorum reduction for the Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].bit = 17;
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nStartTime = 1543622400; // Dec 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nTimeout = 1651363200; // May 1st, 2022

        // Deployment of Cold Staking Pool Fee
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].bit = 18;
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nStartTime = 1559390400; // Jun 1st, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nTimeout = 1622548800; // Jun 1st, 2021

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xac;
        pchMessageStart[1] = 0xb7;
        pchMessageStart[2] = 0x8c;
        pchMessageStart[3] = 0xfe;
        nDefaultPort = 48886;
        nPruneAfterHeight = 1000;
        bnProofOfWorkLimit = arith_uint256(~arith_uint256() >> 16);

        // To create a new devnet:
        //
        // 1) Replace nTimestamp with current timestamp.
        uint32_t nTimestamp = 1569005000;
        // 2) Rebuild
        // 3) Launch daemon. It'll calculate the new parameters.
        // 4) Update the following variables with the new values:
        uint256 hashGenesisBlock = uint256S("0x0000af88d8cae858d9e3e22d5931871e9eefd411984dc241e9bc2de1b7d88326");
        uint256 hashMerkleRoot = uint256S("0x82248aea7886a8a448e42cad754ceae4a25d95b024923f6b4dfb208bb479e27a");
        uint32_t nNonce = 2043685926;
        // 5) Rebuild. Launch daemon.
        // 6) Generate first block using RPC command "./electrum-cli generate 1"

        genesis = CreateGenesisBlockTestnet(nTimestamp, nNonce, 0xffffffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();

        if (true && (genesis.GetHash() != hashGenesisBlock || genesis.hashMerkleRoot != hashMerkleRoot))
        {
            printf("recalculating params for devnet.\n");
            printf("old devnet genesis merkle root:  0x%s\n\n", hashMerkleRoot.ToString().c_str());
            printf("old devnet genesis nonce: %d\n", genesis.nNonce);
            printf("old devnet genesis hash:  0x%s\n\n", hashGenesisBlock.ToString().c_str());
            // deliberately empty for loop finds nonce value.
            for(; genesis.GetHash() > consensus.powLimit; genesis.nNonce++){ }
            printf("new devnet genesis merkle root: 0x%s\n", genesis.hashMerkleRoot.ToString().c_str());
            printf("new devnet genesis nonce: %d\n", genesis.nNonce);
            printf("new devnet genesis hash: 0x%s\n", genesis.GetHash().ToString().c_str());
            printf("use the new values to update CDevNetParams class in src/chainparams.cpp\n");
        }

        // vSeeds.push_back(CDNSSeedData("", ""));
        // vSeeds.push_back(CDNSSeedData("", ""));

        assert(consensus.hashGenesisBlock == hashGenesisBlock);
        assert(genesis.hashMerkleRoot == hashMerkleRoot);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,94);
        base58Prefixes[COLDSTAKING_ADDRESS] = std::vector<unsigned char>(1,63); // cold staking addresses start with 'S'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x40)(0x88)(0x2B)(0xE1).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x40)(0x88)(0xDA)(0x4E).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_dev, pnSeed6_dev + ARRAYLEN(pnSeed6_dev));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0,     hashGenesisBlock),
            1569005000, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            7000         // * estimated number of transactions per day after checkpoint
        };

    }
};
static CDevNetParams devNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 900000;
        consensus.BIP34Hash = uint256S("0xecb7444214d068028ec1fa4561662433452c1cbbd6b0f8eeb6452bcfa1d0a7d6");
        consensus.powLimit = ArithToUint256(~arith_uint256(0) >> 1);
        consensus.nPowTargetTimespan = 30;
        consensus.nPowTargetSpacing = 30;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 75; // 75% of 100
        consensus.nMinerConfirmationWindow = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.nStakeMinAge = 2;	// minimum for coin age: 2 seconds
        consensus.nTargetSpacing = 30; // Blocktime: 30 secs
        consensus.nStakeCombineThreshold = 1000 * COIN;
        consensus.nStakeSplitThreshold = 2 * consensus.nStakeCombineThreshold;
        consensus.nDailyBlockCount =  (24 * 60 * 60) / consensus.nTargetSpacing;
        consensus.nModifierInterval = 10 * 60; // time to elapse before new modifier is computed
        consensus.nTargetTimespan = 25 * 30;
        consensus.nLastPOWBlock = 100000;
        consensus.nBlocksPerVotingCycle = 180; // 1.5 hours
        consensus.nMinimumQuorum = 0.5;
        consensus.nMinimumQuorumFirstHalf = 0.5;
        consensus.nMinimumQuorumSecondHalf = 0.4;
        consensus.nVotesAcceptProposal = 0.7;
        consensus.nVotesRejectProposal = 0.7;
        consensus.nVotesAcceptPaymentRequest = 0.7;
        consensus.nVotesRejectPaymentRequest = 0.7;
        consensus.nCommunityFundMinAge = 5;
        consensus.nProposalMinimalFee = 10000;
        consensus.sigActivationTime = 0;
        consensus.nCoinbaseTimeActivationHeight = 0;
        consensus.nBlockSpreadCFundAccumulation = 10;
        consensus.nCommunityFundAmount = 0.25 * COIN;
        consensus.nCommunityFundAmountV2 = 0.5 * COIN;
        consensus.nCyclesProposalVoting = 4;
        consensus.nCyclesPaymentRequestVoting = 4;
        consensus.nPaymentRequestMaxVersion = 3;
        consensus.nProposalMaxVersion = 3;
        consensus.nMaxFutureDrift = 60000;
        consensus.nStaticReward = 1 * COIN;
        consensus.nHeightv451Fork = 1000;
        consensus.nHeightv452Fork = 1000;
        consensus.fDaoClientActivated = true;

        /** Coinbase transaction outputs can only be spent after this number of new blocks (network rule) */
        consensus.nCoinbaseMaturity = 50;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 2556712000;

        // Deployment of Cold Staking
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].bit = 3;
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COLDSTAKING].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of SegWit (BIP141 and BIP143)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 5;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 2556712000;

        // Deployment of Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].bit = 6;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND].nTimeout = 2556712000;

        // Deployment of Community Fund Accumulation
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].bit = 7;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION].nTimeout = 2556712000;

        // Deployment of NTP Sync
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].bit = 8;
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_NTPSYNC].nTimeout = 2556712000;

        // Deployment of Community Fund Accumulation Spread(NPIP-0003)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].bit = 14;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_ACCUMULATION_SPREAD].nTimeout = 1598918400; // September 1st, 2020

        // Increate in Community Fund Accumulation Ammonut (NPIP-0004)
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].bit = 16;
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_COMMUNITYFUND_AMOUNT_V2].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Static Reward
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].bit = 15;
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_STATIC_REWARD].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Quorum reduction for the Community Fund
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].bit = 17;
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_QUORUM_CFUND].nTimeout = 1598918400; // September 1st, 2020

        // Deployment of Cold Staking Pool Fee
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].bit = 18;
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nStartTime = 1567296000; // September 1st, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_POOL_FEE].nTimeout = 1598918400; // September 1st, 2020

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x81;
        pchMessageStart[1] = 0x15;
        pchMessageStart[2] = 0xbb;
        pchMessageStart[3] = 0x8d;
        nDefaultPort = 48886;
        nPruneAfterHeight = 1000;
        bnProofOfWorkLimit = arith_uint256(~arith_uint256() >> 16);

        uint32_t nTimestamp = GetTimeNow();
        uint256 hashGenesisBlock = uint256S("0x");
        uint256 hashMerkleRoot = uint256S("0x");
        uint32_t nNonce = 2043184832;

        genesis = CreateGenesisBlockTestnet(nTimestamp, nNonce, 0x1d00ffff, 1, 0);
        consensus.hashGenesisBlock = genesis.GetHash();

        if ((genesis.GetHash() != hashGenesisBlock || genesis.hashMerkleRoot != hashMerkleRoot))
        {
            nTimestamp = GetTimeNow();
            // deliberately empty for loop finds nonce value.
            for(; genesis.GetHash() > consensus.powLimit; genesis.nNonce++){ }
            hashGenesisBlock = genesis.GetHash();
            nNonce = genesis.nNonce;
            hashMerkleRoot = genesis.hashMerkleRoot;
        }

        // vSeeds.push_back(CDNSSeedData("", ""));
        // vSeeds.push_back(CDNSSeedData("", ""));

        consensus.hashGenesisBlock = genesis.GetHash();

        assert(consensus.hashGenesisBlock == hashGenesisBlock);
        assert(genesis.hashMerkleRoot == hashMerkleRoot);

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,94);
        base58Prefixes[COLDSTAKING_ADDRESS] = std::vector<unsigned char>(1,63); // cold staking addresses start with 'S'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x40)(0x88)(0x2B)(0xE1).convert_to_container<std::vector<unsigned char> >();
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x40)(0x88)(0xDA)(0x4E).convert_to_container<std::vector<unsigned char> >();

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            ( 0,         hashGenesisBlock),
            nTimestamp, // * UNIX timestamp of last checkpoint block
            0,          // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            7000        // * estimated number of transactions per day after checkpoint
        };
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::DEVNET)
            return devNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}
