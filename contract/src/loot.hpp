#pragma once

#include "atomicassets-interface.hpp"
#include "integer-series.hpp"
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>


using namespace eosio;


CONTRACT loot : public contract
{
public:
    using contract::contract;

    // === Data Structures === //

    // === Admin Actions === //

    // --- Register NFT collection and rewards --- //
    ACTION setnftcolrew(const name& user, const name& collection, const symbol& token_symbol, const name& token_contract, const uint32_t& time_unit_length, const uint32_t& unstake_period,
                        const string& reward_series_referral, const double& reward_coefficient_referral, const string& reward_series_hodl, const double& reward_coefficient_hodl);
    
    // --- Refund rewards --- //
    ACTION refund(const name& user, const name& collection, const asset& refund_amount);

    // --- Add stakable NFT templates --- //
    ACTION addtemplates(const name& user, const uint32_t& template_id, const name& collection, const asset& timeunit_rate);

    // --- Remove stakable NFT templates --- //
    ACTION rmtemplates(const name& user, const uint32_t& template_id, const name& collection);

    // --- Unstake all assets & reset a user from the contract (user can call this now) --- //
    ACTION resetuser(const name &user);// Used to remove bad actor, and if a user can't unstake a removed template 

    // === User Actions === //

    // --- Register a user (needed to claim) --- //
    ACTION regnewuser(const name &user, const name &referrer);

    // -- Claim the reward tokens --- //
    ACTION claim(const name& user, const name& collection);

    // --- Unstake NFTs --- //
    ACTION unstake(const name &user, const vector<uint64_t> &asset_ids);

    // === Notify Handlers === //

    // --- Recieves Rewards from the NFT collection --- //
    void on_transfer(name from, name to, asset quantity, string memo);

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
        uint32_t refscore = 0; // Number of referrals made by the user

        auto primary_key() const { return user.value; }
        //uint64_t by_referrals() const { (uint64_t)refscore; } // Secondary index to sort/query the users by their number of referrals
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
        uint32_t template_id;
        name collection;
        asset timeunit_rate; // Amount of token given 
        auto primary_key() const { return uint64_t(template_id); }
    };

    TABLE user_templ_s {
        name user;
        name collection;
        uint32_t template_id;
        uint32_t amount_staked;
        time_point_sec last_claim;

        uint64_t primary_key() const { return user.value; }
        uint64_t by_collection() const { return collection.value; }
    };

    TABLE bank_s {
        name collection;
        name token_contract;
        asset amount;

        auto primary_key() const { return collection.value; }
    };
    typedef multi_index<name("bank"), bank_s> bank_t;

    // --- Reward Token --- // 
    TABLE config
    {
        name creator;
        name collection; 
        name token_contract;
        symbol token_symbol;
        uint32_t time_unit_length;// Minimum wait (in seconds) for claiming rewards
        uint32_t unstake_period;// Minimum wait (in seconds) for claiming rewards
        string reward_series_referral;
        double reward_coefficient_referral;
        string reward_series_hodl;
        double reward_coefficient_hodl;
        time_point_sec last_claim;

        auto primary_key() const { return collection.value; }
    };


    typedef multi_index<name("stat"), stat_s> stat_t;

    typedef multi_index<name("users"), user_s
        //, indexed_by<name("referrals"), const_mem_fun<user_s, uint64_t, &user_s::by_referrals>>
    > user_t;

    typedef multi_index<name("usertempls"), user_templ_s,
        indexed_by<name("collection"), const_mem_fun<user_templ_s, uint64_t, &user_templ_s::by_collection>>
    > user_templ_t;

    typedef multi_index<name("assets"), asset_s,
                        indexed_by<name("owner"), const_mem_fun<asset_s, uint64_t, &asset_s::by_owner>>>
        asset_t;

    typedef multi_index<name("templates"), template_s> template_t;
    typedef multi_index<name("config"), config> config_t;



    // === Contract Utilities === //

    // --- Check if user is authorized on NFT collection --- //
    bool isAuthorized(name collection, name user)
    {
        auto itrCollection = atomicassets::collections.require_find(collection.value, "No collection with this name exists.");
        bool authorized = false;
        vector<name> authAccounts = itrCollection->authorized_accounts;
        for (auto it = authAccounts.begin(); it != authAccounts.end() && !authorized; it++)
        {
        if (user == name(*it))
        {
            authorized = true;
            break;
        }
        }
        return authorized;
    }

};//END CONTRACT loot 