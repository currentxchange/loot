# loot by White Puma 

THIS IS NOT A THE CONTRACT YOU ARE LOOKING FOR

it is coded project-specific. We will release this in a new repo on [cXc](https://github.com/currentxchange/) when finished. 

Smart contract NFT staking using Atomicassets standard. Based on [ezstake](https://github.com/benjiewheeler/ezstake).

This contract Modifies [ezstake](https://github.com/benjiewheeler/ezstake) in the following ways. 
- Removes freezing capability. 
- - We intend to release a version with freeze capability once the contract is working.
- Introduces referral system with rewards multiplier
- Adds another rewards multiplier for # of NFTs staked in the template
- Adds a new reward multiplier for # of NFTs staked in the template
- Both rewards multipliers use Tetrahedral Number Sequence


This contract is designed for [White Puma](https://open.spotify.com/artist/3Zeg48XqmHuaWrIX2IbQ5M), and is to test the code before a pubic release of a NFT staking economy. 

All the main features are fully configurable by the contract owner.

## Features


#### For the staker

-   Register
-   Refer people / be referred for bonuses
-   Stake/unstake atomicassets NFTs
-   Claim reward tokens


## Structure

![Structure of Contract](https://images.hive.blog/0x0/https://files.peakd.com/file/peakd-hive/aquarius.academy/23tmmjqriMjzjWeJ7LSNY8yZaBYihjBuq17gtJPsQ9ugX41oNzY8SmUFundjZqBgFKrHY.png)


## Testing
- This contract is untested and meant for personal use of White Puma project. 

-   To build the contract for testing [blanc](https://github.com/haderech/blanc) is required.

```bash
npm install # or yarn or pnpm
npm run build:dev # to compile the contract using blanc++
npm test
```

## Deployment

-   To build & deploy the contract, both of the Antelope [cdt](https://github.com/AntelopeIO/cdt) and [leap](https://github.com/AntelopeIO/leap) are required.

```bash
npm build:prod # to compile the contract using cdt-cpp

# deploy the contract
cleos -u <your_api_endpoint> set contract <account> $PWD contract/ezstake.wasm contract/ezstake.abi -p <account>@active

# dont forget to add eosio.code permission
cleos -u <your_api_endpoint> set account permission <account> active --add-code
```
