// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2014-2017 The DASH Core developers
// Copyright (c) 2017-2018 The Aywa Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

#include "arith_uint256.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * CBlock(hash=00000ffd590b14, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=e0028e, nTime=1390095618, nBits=1e0ffff0, nNonce=28917698, vtx=1)
 *   CTransaction(hash=e0028e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d01044c5957697265642030392f4a616e2f3230313420546865204772616e64204578706572696d656e7420476f6573204c6976653a204f76657273746f636b2e636f6d204973204e6f7720416363657074696e6720426974636f696e73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0xA9037BAC7050C479B121CF)
 *   vMerkleTree: e0028e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Wall Street Journal. September 30, 2018. Consumers Drive the Economy...";
    const CScript genesisOutputScript = CScript() << ParseHex("04a6f0b8cfe60fe5fa46a4de82d7475c5fdf6a2de0cf0d7bc7e58868440ce88a0ce96d406af6a702fa9ca6956fb9f09a615670bb030a0c7dc1e268e0c1c24995c2");
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
        genesis.nTime=1538344800;
        consensus.nPowTargetSpacing = 90;//New block every ~ 1.5 minutes
        consensus.nPowTargetTimespan = 900;//10 * consensus.nPowTargetSpacing;//adjust diffuculty
        consensus.nSubsidyHalvingInterval =  1402560;//86400/consensus.nPowTargetSpacing*(365*4+1);//4 years
        consensus.nMasternodePaymentsStartBlock = 80;
        consensus.nInstantSendKeepLock = 24;
        consensus.nBudgetPaymentsStartBlock = 160;//160;
        consensus.nBudgetPaymentsCycleBlocks = 960; //86400/consensus.nPowTargetSpacing - Superblock generates daily about 22:00 UTC
        consensus.nBudgetPaymentsWindowBlocks = 480;//consensus.nBudgetPaymentsCycleBlocks/2;
        consensus.nBudgetProposalEstablishingTime = 172800;//consensus.nBudgetPaymentsCycleBlocks*consensus.nPowTargetSpacing*2;
        consensus.nSuperblockStartBlock = 320;
        consensus.nSuperblockCycle = 960;//consensus.nBudgetPaymentsCycleBlocks;
        consensus.nGovernanceMinQuorum = 10;
        consensus.nGovernanceFilterElements = 20000;
        consensus.nMasternodeMinimumConfirmations = 15;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x0002595c1765d6f2ea85d26987d7e3430a86d96481678bc3b4e14d94baa2183a");
        consensus.powLimit = uint256S("0x0002fffff0000000000000000000000000000000000000000000000000000000");
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nPowDGWHeight = 0;
        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1538344800;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1538344800+31535999;// One year

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1538344800;//genesis.nTime;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1538344800+31535999;

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 1538344800;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 1538344800+31535999;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 4032;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 3226; // 80% of 4032

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00"); // TODO:

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");//TODO:
        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x6e;//bf;
        pchMessageStart[1] = 0x31;//0c;
        pchMessageStart[2] = 0xbc;//6b;
        pchMessageStart[3] = 0xfd;//bd;
        vAlertPubKey = ParseHex("043ba4688b05d6a475085c91791fc95705981de85baf3dd285dfab9a06badeb44cc2851ff6d1f32e56d1572ee5fc6f4a3a74d8df56cfe4c1f6a0511e4846514189");
        nDefaultPort = 2777;
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 100000;
        genesis.nBits = UintToArith256(consensus.powLimit).GetCompact();
        genesis.nNonce = 25487;
        genesis = CreateGenesisBlock(genesis.nTime, genesis.nNonce, genesis.nBits, 1, COIN);
                                     //1538344800, 25487, 0x1e0ffff0, 1, COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0002b2f8bf24aaa3fede2f2faf353eafea6f22124b6bd7db77f27185e38dd78c"));
        assert(genesis.hashMerkleRoot == uint256S("0xfc7364c773f4d75b8aaf86eef027c6c0fb940f86abdc78e95df8332611d65fea"));
        vSeeds.push_back(CDNSSeedData("45.32.36.139", "45.32.36.139"));
        vSeeds.push_back(CDNSSeedData("149.28.207.48", "149.28.207.48"));
        vSeeds.push_back(CDNSSeedData("199.247.4.106", "199.247.4.106"));

        // Aywa addresses start with 'A'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,23);
        // Aywa script addresses start with 'a'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,83);
        // Aywa private keys start with 'P'
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,150);
        // Aywa BIP32 pubkeys start with 'xpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x88)(0xB2)(0x1E).convert_to_container<std::vector<unsigned char> >();
        // Aywa BIP32 prvkeys start with 'xprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x88)(0xAD)(0xE4).convert_to_container<std::vector<unsigned char> >();

        // Aywa BIP44 coin type is '5'
        nExtCoinType = 2777;//5;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        vFixedSeeds.clear();

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 60*60; // fulfilled requests expire in 1 hour
        strSporkPubKey = "04b942ba4f1a0233c264a16f572777c8bf566ab0668dfa98a8f283f8708b892e870e6da613ca2ad2ce30429fbf244651260b4e87b5e3c761c46ae79f429c6de105";
        checkpointData = CCheckpointData {
            boost::assign::map_list_of
            (     1, uint256S("0x0002595c1765d6f2ea85d26987d7e3430a86d96481678bc3b4e14d94baa2183a"))
            (    10, uint256S("0x00006d3379b721a3362d939ba419f9078044f5072d957bca1d13168aa0064e4b"))
            (   100, uint256S("0x0000b52c390ec6d4e7dff89c5247e641601326f2e4a5a520714a534572a0d1a6"))
            (  1000, uint256S("0x000069f37e1b6b6a747f8d14e4914994dba22beb3eba596b2ec2a3eff44735fc"))
            (  2500, uint256S("0x0000c7e0026f4ce4a2d497d9610e6e26c75fb2c94832699ed7dcd2453c29401b"))
            (  5000, uint256S("0x00011ad3b5019acb974d528304082ac658f3a008fbe1f720cf55d9a9bc08eade"))
            (  7000, uint256S("0x0001c29d047dbbbf85df497e9c8dde805853e293292d4f559553ae59aa37df0b"))
            (  7199, uint256S("0x00014bef149165553c5b2be2a7dee5fb365920e4bde753ca182af40270dd3ab4"))
            ( 10000, uint256S("0x0000f7692369f3f5fb4e78de09f82ae834211d5b19a65b55e09d44a339b9fb00"))
            ( 20000, uint256S("0x0000795591fea9bc4cd5d339a14a919ddad387b4c175ed43920682458a1ebfbb"))
            ( 30000, uint256S("0x0000017d8f1d38ddc2d1ec5925a638fecb7aa4efd5c4b15d59e8a38e92059cbc"))
            ( 32000, uint256S("0x00000221d0998561a319c7a70b94862117cc06060f3d68bed18ba2d191366f84"))    ,
              1541225101, // * UNIX timestamp of last checkpoint block
              34519,       // * total number of transactions between genesis and last checkpoint
                          //   (the tx=... number in the SetBestChain debug.log lines)
              5000        // * estimated number of transactions per day after checkpoint



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
        genesis.nTime = 1538145886;
        consensus.nSubsidyHalvingInterval = 31535999*10;//10 years for testnet;
        consensus.nMasternodePaymentsStartBlock = 960;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 160;
        consensus.nBudgetPaymentsCycleBlocks = 80;//2 hours
        consensus.nBudgetPaymentsWindowBlocks = 40;
        consensus.nBudgetProposalEstablishingTime = 14400; //4 hours
        consensus.nSuperblockStartBlock = 160;
        consensus.nSuperblockCycle = 160; // Superblocks can be issued every 4 hours on testnet
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 500;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 100;
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256S("0x0000047d24635e347be3aaaeb66c26be94901a2f962feccd4f95090191f208c1");
        consensus.powLimit = uint256S("0x000ffffff0000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 900;// 10 blocks
        consensus.nPowTargetSpacing = 90;// Block every ~90 seconds
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nPowDGWHeight = 1;//Start DGW+TGW after The Genesis Block
        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = genesis.nTime;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = genesis.nTime+31535999;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = genesis.nTime;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = genesis.nTime+31535999;

        // Deployment of DIP0001
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = genesis.nTime;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = genesis.nTime+31535999;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nWindowSize = 100;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nThreshold = 50; // 50% of 100

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");//0000000000000000000000000000000000000000000000000924e924a21715"); // 37900

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");//00000004f5aef732d572ff514af99a995702c92e4452c7af10858231668b1f"); // 37900

        pchMessageStart[0] = 0xce;
        pchMessageStart[1] = 0xe2;
        pchMessageStart[2] = 0xca;
        pchMessageStart[3] = 0xff;
        vAlertPubKey = ParseHex("0442291a5eed5bff026752987923c52aeadd265e7140fe13185cb32c2d648d8f3117a00a3fbca3bac05246dee477229f8c16e85ba350024509f10a7762a514d53c"); //e3f040d96db3dc4e052ca8c1baf57b75981d6a8b608e59cbbededa8b3a94fa92
        nDefaultPort = 27770;
        nMaxTipAge = 0x7fffffff; // allow mining on top of old blocks for testnet
        nDelayGetHeadersTime = 24 * 60 * 60;
        nPruneAfterHeight = 1000;

        //genesis
        //std::cout << "===TESTNET===:\n";
        genesis.nBits = UintToArith256(consensus.powLimit).GetCompact();//0x1f0fffff;
//        arith_uint256 hashTarget = arith_uint256().SetCompact(genesis.nBits);
//        std::cout << strNetworkID << " ---\n";
//        std::cout << "  hashTarget: " << hashTarget.ToString() <<  "\n";
//        std::cout << "   time: " << genesis.nTime << "\n";
//        std::cout << "UInttoArith:" << consensus.powLimit.ToString() << "\n";
//        uint32_t hashTargwtUint= hashTarget.GetCompact();
//        std::cout << "UInttoArith:" << hashTargwtUint << "\n";
        //get genesis
        if (false) { //looking for the genesis
            for (genesis.nNonce=1; genesis.nNonce < 0x7FFFFFFF; ++genesis.nNonce) {
                genesis = CreateGenesisBlock(genesis.nTime, genesis.nNonce, genesis.nBits, 1, COIN);
                consensus.hashGenesisBlock = genesis.GetHash();
                if (genesis.nNonce % 1000 == 0)
                    std::cout << strNetworkID << " nonce: " << genesis.nNonce << " time: " << genesis.nTime << " hash: " << genesis.GetHash().ToString().c_str() << "\n";
                if (UintToArith256(consensus.hashGenesisBlock) < UintToArith256(consensus.powLimit))
                    break;
            }
            std::cout << strNetworkID << " ---\n";
            std::cout << "  nonce: " << genesis.nNonce <<  "\n";
            std::cout << "   time: " << genesis.nTime << "\n";
            std::cout << "   hash: " << genesis.GetHash().ToString().c_str() << "\n";
            std::cout << "   merklehash: "  << genesis.hashMerkleRoot.ToString().c_str() << "\n";
            std::cout << "Finished calculating " << strNetworkID << "Testnet Genesis Block:\n";
        }

        genesis.nNonce = 2036;

        genesis = CreateGenesisBlock(genesis.nTime, genesis.nNonce, 0x1f0fffff, 1, COIN);
        consensus.hashGenesisBlock = genesis.GetHash();

        //assert(consensus.hashGenesisBlock == uint256S("0xb103ecd018e289c50dea39e8abdbf3e812aa3037301aaf2adc763dd9d314b358"));
                                                      //"00000bafbc94add76cb75e2ec92894837288a481e5c005f6563d91623bf8bc2c"));
        assert(genesis.hashMerkleRoot == uint256S("0xfc7364c773f4d75b8aaf86eef027c6c0fb940f86abdc78e95df8332611d65fea"));
                                                  //e0028eb9648db56b1ac77cf090b99048a8007e2bb64b68f092c03c7f56a662c7"));
        vFixedSeeds.clear();
        vSeeds.clear();
        vSeeds.push_back(CDNSSeedData("aywadot.io",  "testnet-seed.aywadot.io"));
        vSeeds.push_back(CDNSSeedData("masternode.io", "test.dnsseed.masternode.io"));
        vSeeds.clear();
        // Testnet Aywa addresses start with 'a'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);//140
        // Testnet Aywa script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Testnet private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Testnet Aywa BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Testnet Aywa BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Testnet Aywa BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        nPoolMaxTransactions = 3;
        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes
        strSporkPubKey = "040a37d85e71e89e3a43f2299cadca623df9f668a9bbcbc1f965a1ee88b892c9fe187326002f829cdb768cb68588881eaf8d60d202e120f8da5469651ff57ef80a";//ef5099dd08bd4836289165c91c150a7988eeb7dcbaa5cf938c268044a1f77f2f

        checkpointData = CCheckpointData {
            boost::assign::map_list_of
            (    0, uint256S("0x000002654654dec99fe90f066ca51c7abf7ca1c7527487379a0f3de43c9b6629")),
            //(   1999, uint256S("0x00000052e538d27fa53693efe6fb6892a0c1d26c0235f599171c48a3cce553b1"))
            //(   2999, uint256S("0x0000024bc3f4f4cb30d29827c13d921ad77d2c6072e586c7f60d83c2722cdcc5")),

            1531901186,//1462856598, // * UNIX timestamp of last checkpoint block
            0,//3094,       // * total number of transactions between genesis and last checkpoint
                        //   (the tx=... number in the SetBestChain debug.log lines)
            500         // * estimated number of transactions per day after checkpoint
        };

    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.nMasternodePaymentsStartBlock = 240;
        consensus.nMasternodePaymentsIncreaseBlock = 350;
        consensus.nMasternodePaymentsIncreasePeriod = 10;
        consensus.nInstantSendKeepLock = 6;
        consensus.nBudgetPaymentsStartBlock = 1000;
        consensus.nBudgetPaymentsCycleBlocks = 50;
        consensus.nBudgetPaymentsWindowBlocks = 10;
        consensus.nBudgetProposalEstablishingTime = 60*20;
        consensus.nSuperblockStartBlock = 1500;
        consensus.nSuperblockCycle = 10;
        consensus.nGovernanceMinQuorum = 1;
        consensus.nGovernanceFilterElements = 100;
        consensus.nMasternodeMinimumConfirmations = 1;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.BIP34Height = -1; // BIP34 has not necessarily activated on regtest
        consensus.BIP34Hash = uint256();
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 24 * 60 * 60; // Aywa: 1 day
        consensus.nPowTargetSpacing = 2.5 * 60; // Aywa: 2.5 minutes
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nPowKGWHeight = 15200; // same as mainnet
        consensus.nPowDGWHeight = 34140; // same as mainnet
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_DIP0001].nTimeout = 999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfc;
        pchMessageStart[1] = 0xc1;
        pchMessageStart[2] = 0xb7;
        pchMessageStart[3] = 0xdc;
        nMaxTipAge = 6 * 60 * 60; // ~144 blocks behind -> 2 x fork detection time, was 24 * 60 * 60 in bitcoin
        nDelayGetHeadersTime = 0; // never delay GETHEADERS in regtests
        nDefaultPort = 19994;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1417713337, 1096447, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        //assert(consensus.hashGenesisBlock == uint256S("0xa309877fb3d28a1772503edc5db0fcd5bcc19a5728f00133f6142730a0f0c9a7"));
                                                      //"000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e"));
        assert(genesis.hashMerkleRoot == uint256S("0xb4449294ea09570970dc0e1339f94c63df118629f992643ef3803992154bfe13"));
                                                  //"e0028eb9648db56b1ac77cf090b99048a8007e2bb64b68f092c03c7f56a662c7"));

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        nFulfilledRequestExpireTime = 5*60; // fulfilled requests expire in 5 minutes

        checkpointData = CCheckpointData{
            boost::assign::map_list_of
            ( 0, uint256S("0x000008ca1832a4baf228eb1553c03d3a2c8e02399550dd6ea8d65cec3ef23d2e")),
            0,
            0,
            0
        };
        // Regtest Aywa addresses start with 'y'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,140);
        // Regtest Aywa script addresses start with '8' or '9'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,19);
        // Regtest private keys start with '9' or 'c' (Bitcoin defaults)
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        // Regtest Aywa BIP32 pubkeys start with 'tpub' (Bitcoin defaults)
        base58Prefixes[EXT_PUBLIC_KEY] = boost::assign::list_of(0x04)(0x35)(0x87)(0xCF).convert_to_container<std::vector<unsigned char> >();
        // Regtest Aywa BIP32 prvkeys start with 'tprv' (Bitcoin defaults)
        base58Prefixes[EXT_SECRET_KEY] = boost::assign::list_of(0x04)(0x35)(0x83)(0x94).convert_to_container<std::vector<unsigned char> >();

        // Regtest Aywa BIP44 coin type is '1' (All coin's testnet default)
        nExtCoinType = 1;
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
