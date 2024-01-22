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

    // --- Series Constants --- //
    //Also accepts ZERO which sets rewards to Zero and COUNT which counts from one. 
    namespace series {
    
    const std::vector<uint64_t> FIBONACCI = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181, 6765, 10946, 17711, 28657, 9999999999999999999};
    const std::vector<uint64_t> SILVER = {1, 2, 5, 12, 29, 70, 169, 408, 985, 2378, 5741, 13860, 33461, 80782, 195025, 470832, 1136689, 2744210, 6625109, 15994428, 38613965, 93222358, 225058681, 54333972, 9999999999999999999};

    const std::vector<uint64_t> TETRAHEDRAL = {1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680, 816, 969, 1140, 1330, 1540, 1771, 2024, 2300, 2600, 9999999999999999999};
    const std::vector<uint64_t> OCTAHEDRAL = {1, 6, 19, 44, 85, 146, 231, 344, 489, 670, 891, 1156, 1471, 1842, 2275, 2776, 3351, 4006, 4747, 5580, 6511, 7546, 8691, 9952, 9999999999999999999};
    const std::vector<uint64_t> HEXAHEDRAL = {1, 8, 27, 64, 125, 216, 343, 512, 729, 1000, 1331, 1728, 2197, 2744, 3375, 4096, 4913, 5832, 6859, 8000, 9261, 10648, 12167, 13824, 9999999999999999999};
    const std::vector<uint64_t> ICOSAHEDRAL = {1, 12, 42, 92, 162, 252, 362, 492, 642, 812, 1002, 1212, 1442, 1692, 1962, 2252, 2562, 2892, 3242, 3612, 4002, 4412, 4842, 5292, 9999999999999999999};
    const std::vector<uint64_t> DODECAHEDRAL = {1, 20, 84, 220, 455, 812, 1330, 2024, 2925, 4060, 5456, 7140, 9141, 11480, 14156, 17264, 20899, 25056, 29830, 35216, 41219, 47844, 55196, 63280, 9999999999999999999};

    const std::vector<uint64_t> LUCAS = {2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364, 2207, 3571, 5778, 9349, 15127, 24476, 39603, 64079, 9999999999999999999};
    const std::vector<uint64_t> TRIANGULAR = {1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231, 253, 276, 300, 9999999999999999999};
    const std::vector<uint64_t> SQUARE = {1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576, 9999999999999999999};
    const std::vector<uint64_t> PENTAGONAL = {1, 5, 12, 22, 35, 51, 70, 92, 117, 145, 176, 210, 247, 287, 330, 376, 425, 477, 532, 590, 651, 715, 782, 852, 9999999999999999999};
    const std::vector<uint64_t> HEXAGONAL = {1, 6, 15, 28, 45, 66, 91, 120, 153, 190, 231, 276, 325, 378, 435, 496, 561, 630, 703, 780, 861, 946, 1035, 1128, 9999999999999999999};

    }


    // === Admin Actions === //

    // --- Set the contract configuration --- //
    ACTION setconfig(const uint32_t &min_claim_period, const uint32_t &unstake_period);

    // --- Register NFT collection and rewards --- //
    ACTION setnftcolrew(const name& user, const name& collection, const symbol& token_symbol, const name& token_contract, const uint32_t& tu_length, const uint32_t& unstake_period,
                        const string& reward_series_referral = string("TETRAHEDRAL"), const double& reward_coefficient_referral = 1.0, const string& reward_series_hodl = string("TETRAHEDRAL"), const double& reward_coefficient_hodl = 1.0)

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

    TABLE user_templ_s {
        name user;
        name collection;
        name template;
        asset amount_staked;

        uint64_t primary_key() const { return user.value; }
        uint64_t by_collection() const { return collection.value; }
    };

    TABLE bank_s {
        name collection;
        asset amount;

        uint64_t primary_key() const { return collection.value; }
    };

    // --- Reward Token --- // 
    TABLE config
    {
        name creator;
        name collection; 
        name token_contract;
        symbol token_symbol;
        uint32_t min_claim_period;// Minimum wait (in seconds) for claiming rewards
        uint32_t unstake_period;// Minimum wait (in seconds) for claiming rewards
        string reward_series_referral;
        double reward_coefficient_referral;
        string reward_series_hodl;
        double reward_coefficient_hodl;

        auto primary_key() const { return collection.value; }
    };


    typedef multi_index<name("stat"), stat_s> stat_t;

    typedef multi_index<name("users"), user_s,
        indexed_by<name("stacked"), const_mem_fun<user_s, uint64_t, &user_s::by_stacked>>,
        indexed_by<name("referrals"), const_mem_fun<user_s, uint64_t, &user_s::by_referrals>>
    > user_t;

    typedef multi_index<name("usertempls"), user_templs,
        indexed_by<name("collection"), const_mem_fun<user_templs, uint64_t, &user_templs::by_collection>>
    > usertempls_t;

    typedef multi_index<name("assets"), asset_s,
                        indexed_by<name("owner"), const_mem_fun<asset_s, uint64_t, &asset_s::by_owner>>>
        asset_t;

    typedef multi_index<name("templates"), template_s> template_t;
    typedef multi_index<name("config"), config> config_t;
    typedef multi_index<name("bank"), bank_s> bank_t;

    // === Contract Utilities === //

    // --- Returns the config object for use withing actions --- //
    config check_config()
    {
        config_t conf_tbl(get_self(), get_self().value);// Get config table
        check(conf_tbl.exists(), "The contract is not initialized yet"); // Check if a config exists
        const auto &conf = conf_tbl.get(); // Get  current config

        return conf; // Returns the config table for the 
    }

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
            }
         }
         return authorized;
      }

        // --- Return a Series --- //
        vector<uint64_t> getSeries(string series){
            switch (series) {

                case "FIBONACCI":
                return series::FIBONACCI;
                
                case "SILVER":
                return series::SILVER;
                
                case "TETRAHEDRAL":
                return series::TETRAHEDRAL;
                
                case "OCTAHEDRAL":
                return series::OCTAHEDRAL;

                case "HEXAHEDRAL":
                return series::HEXAHEDRAL;
                
                case "ICOSAHEDRAL":
                return series::ICOSAHEDRAL;

                case "DODECAHEDRAL":
                return series::DODECAHEDRAL;

                case "LUCAS":
                return series::LUCAS;
                
                case "TRIANGULAR":
                return series::TRIANGULAR;
                
                case "SQUARE":
                return series::SQUARE;

                case "PENTAGONAL":
                return series::PENTAGONAL;
                
                case "HEXAGONAL":
                return series::HEXAGONAL;

                case "COUNT": // MOVE TO FUNCTION
                return series::HEXAGONAL;

                case "NONE":
                return series::HEXAGONAL;

                case "NONE":
                return series::HEXAGONAL;
                
                default:

                return std::vector<uint64_t>(); 
        }


        }



};
