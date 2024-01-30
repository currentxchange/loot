Here's a structured README for your repository that explains the actions and features of your NFT staking smart contract using AtomicAssets:

---

# Loot 💰 NFT Staking Smart Contract for AtomicAssets NFT collections

[Testnet deployment](https://testnet.waxblock.io/account/whitepumats2)

## 🌞 Setting Up Rewards for Collection Owners
To set up NFT staking rewards for your collection, follow these steps:

- Step 1: Register as a User
Call `regnewuser` to register yourself on the contract if not already registered.

- Step 2: Register Your Collection
Use `setnftcolrew` to register your collection with its reward parameters and time unit settings.

- Step 3: Add Eligible NFT Templates
Call `addtemplates` to define which templates in your collection can be staked for rewards.

- Step 4: Fund the Contract
Transfer the designated reward tokens to the contract with a `memo` stating your collection name, establishing a `bank` for rewards.

- Step 5: Optional - Manage Templates
Adjust eligible templates or remove them as needed with `addtemplates` or `rmtemplates`. Read below for important info.

- Step 6: Communicate with Your Community
Inform your community about the staking process, rewards, and any changes to ensure transparency and engagement.








**For NFT Holders:**
- Register on the contract and include a Referrer for a +1 Referral score.
- Send your NFT to the contract. If it's not eligible, it will be returned.
- Claim rewards after 5 minutes (or the set time unit length).
- Receive Epic bonuses for participation and staking:
  - Intuitive leveling system
  - Referral Score Multiplier
  - Number of NFTs staked multiplier (per template)

**For Project Owners:**
- Offer various rewards for multiple templates within your collection, and allow your users to easily claim their loot. 
- Incentivize participation with reward multipliers for the number of NFT staked, and the number of invites a user has. 
- You can turn off for word multipliers for the referral bonus as well as the NFT state amount bonus by setting the coefficient to 0 when calling `

### Features

- Customizable time unit length (1 TU = 1 reward).
- Template management for stakable NFTs.
- Referral system with rewards multiplier.
- Rewards multiplier based on the number of NFTs staked per template.


### Contract Structure

The contract is structured to handle user registrations, NFT staking/unstaking, reward calculations, and payouts efficiently. The following diagram illustrates the main data stores:


![Contract Structure Diagram](https://files.peakd.com/file/peakd-hive/douglasjames/23sxpDajoY99Db88iboGorTNUdyAhKXnhM8mHkoCMBwdmbQLD5a6jU4q5CCNAoAGC3Ff3.png)



### Leveled Rewards using Integer Series

The contract utilizes various integer sequences for calculating rewards, offering a range of series including Fibonacci, Silver Ratio, Tetrahedral, and more. This allows for a rich and flexible rewards system that can be tailored to the specific needs and goals of the NFT project. See them in `integer-series.hpp`

---

# Actions to know

Below are the actions within the Loot contract that can be called by end-users, along with explanations for their arguments:

#### `regnewuser`

- **Description**: Registers a new user for NFT staking and optionally assigns a referrer to the new user. You were able to set   any account as the referral, and if they are not registered the contract will register them. It is recommended to include your refer, as on sign up you will also get a bonus of one point for being referred.
- **Arguments**:
  - `user`: The account name of the user being registered.
  - `referrer` (optional): The account name of the user who referred the new user. This can be left empty.

#### `claim`

- **Description**: Allows a user to claim their accumulated rewards based on the NFTs they have staked. You only need to place the collection, not the individual asset IDs, making claims easier without a UI. You will get your tokens and a memo that explains how the rewards were calculated.
- **Arguments**:
  - `user`: The account name of the user claiming rewards.
  - `collection`: The NFT collection name from which the user wants to claim rewards.

#### `unstake`

- **Description**: Allows a user to unstake one or more NFTs, removing them from the staking pool and ending their reward accumulation. NOTE: The unstake action does NOT also `claim`, be sure you `claim` your rewards before on staking your NFTs. 
- **Arguments**:
  - `user`: The account name of the user unstaking NFTs.
  - `asset_ids`: An array of asset IDs representing the NFTs to be unstaked.

#### `resetuser`

- **Description**: Resets a user's staking status, unstaking all their NFTs and removing them from the user registry. This action is intended for use in case of errors or the need to withdraw all assets. You are allowed to reset your own stats. 
- **Arguments**:
  - `user`: The account name of the user to be reset.

#### `refund`

- **Description**: Processes a refund of staked tokens to a user. This action might be used in exceptional circumstances, such as contract migration or error correction.
- **Arguments**:
  - `user`: The account name of the user receiving the refund.
  - `collection`: The NFT collection name related to the refund.
  - `refund_amount`: The amount of tokens to be refunded to the user.


#### `setnftcolrew`

- **Description**: Registers an NFT collection for staking, setting the parameters for reward distribution.

A time unit = seconds for one unit of reward, so how often a user can claim something. Set the length of the time unit here, and the reward for each individual template 

There are some important concepts to understand when calling this action to set up your collection rewards. 

1. The reward series arguments require sending an all-caps identifier of the reward series used. You can find the reward series available in `integer-series.hpp`. This defaults to a tetrahedral series, and if you pass a non-validating value it will default to make the level = the count (linear series, n=n).

Reward Level Seriec Options: FIBONACCI, SILVER, TETRAHEDRAL, OCTAHEDRAL, HEXAHEDRAL, ICOSAHEDRAL, DODECAHEDRAL, LUCAS, TRIANGULAR, SQUARE, PENTAGONAL, HEXAGONAL


2. The coefficient value will also be multiplied. If you don't want to have a bonus reward for either the HODL or the referral, you can set the coefficient to 0, acts as a special value. 

3. The multipliers are both multiplied together, meaning you can have some extremely high rewards. For this reason, it's important to set a very small amount when setting the rewards for the template, which you will do when calling the `addtemplates` action

- **Arguments**:
  - `user`: The account name of the user or the project owner setting up the NFT collection.
  - `collection`: The NFT collection name to be registered for staking.
  - `token_symbol`: The symbol of the tokens to be used for rewards.
  - `token_contract`: The contract where the reward tokens are managed.
  - `time_unit_length`: The length of time units in seconds for reward calculation.
  - `unstake_period`: The minimum period in seconds that NFTs must be staked before they can be unstaked.
  - `reward_series_referral`: The series used to calculate the referral reward multiplier.
  - `reward_coefficient_referral`: The coefficient used to multiply the referral reward.
  - `reward_series_hodl`: The series used to calculate the holding reward multiplier.
  - `reward_coefficient_hodl`: The coefficient used to multiply the holding reward.

#### `addtemplates`

- **Description**: Adds NFT templates to the staking pool, enabling NFTs based on these templates to be staked for rewards. The most important piece is the time unit rate. If you're using the multipliers, we suggest starting with a very small rate, as the multiplication can create large rewards.
- **Arguments**:
  - `user`: The account name of the user or the project owner adding the templates.
  - `template_id`: The ID of the template being added to the staking pool.
  - `collection`: The NFT collection to which the template belongs.
  - `timeunit_rate`: The rate of rewards per time unit for NFTs based on this template.

#### `rmtemplates`

- **Description**: Removes NFT templates from the staking pool, preventing NFTs based on these templates from being staked for rewards.
- **Arguments**:
  - `user`: The account name of the user or the project owner removing the templates.
  - `template_id`: The ID of the template being removed from the staking pool.
  - `collection`: The NFT collection from which the template is being removed.

___

For more information and updates, follow [White Puma](https://open.spotify.com/artist/3Zeg48XqmHuaWrIX2IbQ5M) and stay tuned to the [cXc repository](https://github.com/currentxchange/).

### Disclaimer of Liability
This code + compiled contract is provided "AS IS" without warranty of any kind. The information herein is for informational purposes only and is not intended as financial, legal, or professional advice. The technology discussed is experimental and may contain risks or bugs. Use or interaction with any smart contracts or protocols is at your own risk. Creators and contributors disclaim any liability for losses or damages arising from the use of this content. Always perform due diligence and consult with professionals before engaging with any smart contracts or protocols.


## Thanks 🙏
Loosely based on ezstake from [benjiewheeler](https://github.com/benjiewheeler) with support from [WAX Labs](https://labs.wax.io/proposals/84) 

<center>

Created with 💜 by [cXc](https://linktr.ee/cXc.world) inspired by [White Puma](https://open.spotify.com/artist/3Zeg48XqmHuaWrIX2IbQ5M)

</center>