#pragma once

#include "atomicassets-interface.hpp"
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>


using namespace eosio;

CONTRACT loot : public contract
{
public:
    using contract::contract;

    // === Data Structures === //

    // --- Template struct for the addtemplates/rmtemplates actions --- //
    struct template_item
    {
        // --- Atomicassets Template ID --- //
        int32_t template_id;
        // --- Collection Template Belongs to -- //
        name collection;
        // --- Base amount of token paid per Time Unit --- // 
        asset timeunit_rate;
    };

    // === Admin Actions === //

    // --- Set the contract configuration --- //
    ACTION setconfig(const uint32_t &min_claim_period, const uint32_t &unstake_period);

    // --- Set the reward token --- //
    ACTION settoken(const name &contract, const symbol &symbol);

    // --- Add stakable NFT templates --- //
    ACTION addtemplates(const int32_t& template_id, const name& collection, const asset& timeunit_rate);

    // --- Remove stakable NFT templates --- //
    ACTION rmtemplates(const int32_t& template_id, const name& collection, const asset& timeunit_rate);

    // --- Unstake all assets & reset a user from the contract --- //
    ACTION resetuser(const name &user);// Used to remove bad actor, and if a user can't unstake a removed template 

    // === User Actions === //

    // --- Register a user (needed to claim) --- //
    ACTION regnewuser(const name &user, const name &referrer = ""_n);

    // -- Claim the reward tokens --- //
    ACTION claim(const name &user, const vector<uint64_t> &asset_ids);

    // --- Unstake NFTs --- //
    ACTION unstake(const name &user, const vector<uint64_t> &asset_ids);

    // === Notify Handlers === //

    // --- Recieves NFTs from the user --- //
    void receiveassets(name from, name to, vector<uint64_t> asset_ids, string memo);

private:
    // --- Token stat struct from eosio.token --- //
    struct stat_s
    {
        asset supply;
        asset max_supply;
        name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    // --- Registered Users + Referrers --- // 
    TABLE user_s
    {
        name user; // Name of the user
        name referrer; // Name of the referrer (if any)
        asset stacked;
        uint64_t refscore = 0; // Number of referrals made by the user

        auto primary_key() const { return user.value; }
        uint64_t by_stacked() const { return stacked.amount; }
        uint64_t by_referrals() const { return refscore; } // Secondary index to sort/query the users by their number of referrals
    };

    // --- NFTs + Claims --- // 
    TABLE asset_s
    {
        uint64_t asset_id; // Atomicassets id of the NFTs
        name owner;
        time_point_sec last_claim;

        auto primary_key() const { return asset_id; }
        uint64_t by_owner() const { return owner.value; } // Sort assets by owner
    };

    // --- Stakeable NFTs --- // 
    TABLE template_s
    {
        int32_t template_id;
        name collection;
        asset timeunit_rate; // Amount of token given 
        auto primary_key() const { return uint64_t(template_id); }
    };

    // --- Reward Token --- // 
    TABLE config
    {
        name token_contract = name("moneda.puma");
        symbol token_symbol = symbol(symbol_code("PUMA"), 8);
        uint32_t min_claim_period = 300;// Minimum wait (in seconds) for claiming rewards
        uint32_t unstake_period = 300;// Minimum wait (in seconds) for claiming rewards
    };


    typedef multi_index<name("stat"), stat_s> stat_t;

    typedef multi_index<name("users"), user_s,
        indexed_by<name("stacked"), const_mem_fun<user_s, uint64_t, &user_s::by_stacked>>,
        indexed_by<name("referrals"), const_mem_fun<user_s, uint64_t, &user_s::by_referrals>>
    > user_t;

    typedef multi_index<name("assets"), asset_s,
                        indexed_by<name("owner"), const_mem_fun<asset_s, uint64_t, &asset_s::by_owner>>>
        asset_t;

    typedef multi_index<name("templates"), template_s> template_t;
    typedef singleton<name("config"), config> config_t;

    // === Contract Utilities === //

    // --- Returns the config object for use withing actions --- //
    config check_config()
    {
        config_t conf_tbl(get_self(), get_self().value);// Get config table
        check(conf_tbl.exists(), "The contract is not initialized yet"); // Check if a config exists
        const auto &conf = conf_tbl.get(); // Get  current config

        return conf; // Returns the config table for the 
    }
};
