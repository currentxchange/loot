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

    // ------------ structs ------------

    // public template struct for the addtemplates/rmtemplates actions
    struct template_item
    {
        // id of the template (from the atomicassets)
        int32_t template_id;
        // name of the collection of this template
        name collection;
        // the staking power provided by this template
        asset hourly_rate;
    };

    // ------------ admin actions ------------

    // set the contract config
    ACTION setconfig(const uint32_t &min_claim_period, const uint32_t &unstake_period);

    // set the contract token config
    ACTION settoken(const name &contract, const symbol &symbol);

    // add the staking assets templates
    ACTION addtemplates(const std::vector<template_item> &templates);

    // remove the staking assets templates
    ACTION rmtemplates(const std::vector<template_item> &templates);

    // unstake all assets & reset a user from the contract
    // used in cases of emergencies such as when a user can't unstake a removed template
    ACTION resetuser(const name &user);

    // ------------ user actions ------------

    // register a new user
    ACTION regnewuser(const name &user, const name &referrer = ""_n);

    // claim the generated tokens
    ACTION claim(const name &user, const vector<uint64_t> &asset_ids);

    // unstake the user's assets
    ACTION unstake(const name &user, const vector<uint64_t> &asset_ids);

    // ------------ notify handlers ------------

    // receiver assets from the user
    void receiveassets(name from, name to, vector<uint64_t> asset_ids, string memo);

private:
    // token stat struct
    // taken from the reference eosio.token contract
    struct stat_s
    {
        asset supply;
        asset max_supply;
        name issuer;

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
    };

    TABLE user_s
    {
        name user;                        // Name of the user
        asset hourly_rate;                // The total hourly_rate this user has
        name referrer;                    // Name of the referrer (if any)
        uint64_t refscore = 0; // Number of referrals made by the user

        auto primary_key() const { return user.value; }
        // Secondary index to sort/query the users by their rate
        uint64_t by_rate() const { return hourly_rate.amount; }
        // Secondary index to sort/query the users by their number of referrals
        uint64_t by_referrals() const { return refscore; }
    };

    TABLE asset_s
    {
        // id of the assets (from the atomicassets)
        uint64_t asset_id;
        // name of the owner of the asset
        name owner;
        // timestamp of the last claim
        time_point_sec last_claim;

        auto primary_key() const { return asset_id; }
        // secondary index to sort/query assets by their owner
        uint64_t by_owner() const { return owner.value; }
    };

    TABLE template_s
    {
        // id of the template (from the atomicassets)
        int32_t template_id;
        // name of the collection of this template
        name collection;
        // the staking power provided by this template
        asset hourly_rate;

        auto primary_key() const { return uint64_t(template_id); }
    };

    TABLE config
    {
        // the name of the token contract
        name token_contract = name("moneda.puma");
        // the name of the token symbol
        symbol token_symbol = symbol(symbol_code("P"), 8);
        // the minimum time (in seconds) that a user is required to wait between each claim action
        uint32_t min_claim_period = 600;
        // the minimum time (in seconds) that a user is required to wait until they can unstake their assets
        uint32_t unstake_period = 86400 * 3;
    };

    // token stat table definition
    typedef multi_index<name("stat"), stat_s> stat_t;

typedef multi_index<name("users"), user_s,
    indexed_by<name("rate"), const_mem_fun<user_s, uint64_t, &user_s::by_rate>>,
    indexed_by<name("referrals"), const_mem_fun<user_s, uint64_t, &user_s::by_referrals>>
> user_t;

    typedef multi_index<name("assets"), asset_s,
                        indexed_by<name("owner"), const_mem_fun<asset_s, uint64_t, &asset_s::by_owner>>>
        asset_t;

    typedef multi_index<name("templates"), template_s> template_t;
    typedef singleton<name("config"), config> config_t;

    // Utilities

    // check if the contract is initialized
    config check_config()
    {
        // get config table
        config_t conf_tbl(get_self(), get_self().value);

        // check if a config exists
        check(conf_tbl.exists(), "The contract is not initialized yet");

        // get  current config
        const auto &conf = conf_tbl.get();

        return conf;
    }
};
