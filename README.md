# loot by White Puma 

THIS IS NOT A WORKING CONTRACT 
it is coded project-specific. We will release in a new repo on cXc when finished. 

Smart contract NFT staking using Atomicassets standard. Based on [ezstake](https://github.com/benjiewheeler/ezstake).

This contract Modifies [ezstake](https://github.com/benjiewheeler/ezstake) in the following ways. 
- Removes freezing capability. 
- - We intend to release a version with freeze capability once the contract is working.
- Adds a claim multiplier for PUMA of claims and NFTs of template

This contract is designed for White Puma, and is to test the code before a pubic release. 

All the main features are fully configurable by the contract owner.

## Features

#### For the staker:

-   register
-   stake/unstake the assets
-   claim the tokens


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
