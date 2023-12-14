# loot by [White Puma](https://puma.red)

Smart contract for NFT staking using Atomicassets standard. Based on [ezstake](https://github.com/benjiewheeler/ezstake).

This contract is designed to work out-of-the-box, without the need to modify the source code.

All the main features are fully configurable by the contract owner.

## Features

#### For the admin:

-   customizable configurations:
    -   token contract and symbol
    -   minimum claim period
    -   unstaking period
    -   hourly rate per template
    -   per template control
-   freeze/unfreeze the contract functionalities
-   force reset/unstake user's assets

#### For the user:

-   register
-   stake/unstake the assets
-   claim the tokens

## Testing

The contract is fully tested using proton's [VeRT](https://docs.protonchain.com/contract-sdk/testing.html)

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
