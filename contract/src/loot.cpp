#include "loot.hpp"

/*/

ACTION loot::setconfig(const uint32_t& min_claim_period, const uint32_t& unstake_period)
{
    // --- Authentication Check --- // 
    check(has_auth(get_self()), "this action is admin only");

    config_t conf_tbl(get_self(), get_self().value);// Get config table 
    auto conf = conf_tbl.get_or_default(config {});// get/create current config

    conf.min_claim_period = min_claim_period;
    conf.unstake_period = unstake_period;

    // --- Save Configuration -- //
    conf_tbl.set(conf, get_self());
}

/*/

ACTION loot::setnftcolrew(const name& user, const name& collection, const symbol& token_symbol, const name& token_contract, const uint32_t& tu_length, const uint32_t& unstake_period,
                        const string& reward_series_referral = string("TETRAHEDRAL"), const double& reward_coefficient_referral = 1.0, const string& reward_series_hodl = string("TETRAHEDRAL"), const double& reward_coefficient_hodl = 1.0)
{
    // --- Authorization Check --- // 
   check(has_auth(get_self()) || has_auth(user), "Missing authorization by user.");
   check(isAuthorized(collection, user), "Missing Authorization. To start rewards, you must be authorized for the collection on atomicassets.");

    // --- Check contract at address --- //
    check(is_account(token_contract), "Token contract account does not exist");

    // --- Basic Token Check --- //
    stat_t stat(token_contract, token_symbol.code().raw());
    stat.require_find(token_symbol.code().raw(), "Token symbol does not exist on the token contract");

        // --- Initialize  config_t --- //'
    config_t config_tbl(get_self(), get_self().value);
    auto config_itr = config_tbl.find(collection.value);

    // --- Upsert the config --- //
    if (config_itr == config_tbl.end()) {
        // Insert new config if not present
        config_tbl.emplace(user, [&](auto& row) {
            row.creator = user;
            row.collection = collection;
            row.token_contract = token_contract;
            row.token_symbol = token_symbol;
            row.min_claim_period = min_claim_period;
            row.unstake_period = unstake_period;
            row.reward_series_referral = reward_series_referral;
            row.reward_coefficient_referral = reward_coefficient_referral;
            row.reward_series_hodl = reward_series_hodl;
            row.reward_coefficient_hodl = reward_coefficient_hodl;
        });
    } else {
        // Modify existing config
        config_tbl.modify(config_itr, user, [&](auto& row) {
            row.token_contract = token_contract;
            row.token_symbol = token_symbol;
            row.min_claim_period = min_claim_period;
            row.unstake_period = unstake_period;
            row.reward_series_referral = reward_series_referral;
            row.reward_coefficient_referral = reward_coefficient_referral;
            row.reward_series_hodl = reward_series_hodl;
            row.reward_coefficient_hodl = reward_coefficient_hodl;
        });
    }
   
                        
}

ACTION loot::settoken(const name& contract, const symbol& symbol)
{
    // --- Authorization Check --- // 
    check(has_auth(get_self()), "This action is admin only");

    // --- Check contract at address --- //
    check(is_account(contract), "Contract account does not exist");

    // --- Basic Token Check --- //
    stat_t stat(contract, symbol.code().raw());
    stat.require_find(symbol.code().raw(), "token symbol does not exist");

    config_t conf_tbl(get_self(), get_self().value);// Get config table 

    auto conf = conf_tbl.get_or_default(config {}); // Get/create current config

    conf.token_contract = contract;
    conf.token_symbol = symbol;

    // --- Save the new config --- //
    conf_tbl.set(conf, get_self());
}

ACTION loot::addtemplates(const int32_t& template_id, const name& collection, const asset& timeunit_rate)
{
    // --- Authentication Check --- // 
    check(has_auth(get_self()), "this action is admin only");

    // --- Import config --- //
    const auto& config = check_config();

    // --- Get templates table  --- //
    template_t template_tbl(get_self(), get_self().value);
    
    check(timeunit_rate.amount > 0, "timeunit_rate must be positive");// check if the hourly rate is valid
    check(config.token_symbol == timeunit_rate.symbol, "symbol mismatch");

    // check if the template exists in atomicassets and it's valid
    const auto& aa_template_tbl = atomicassets::get_templates(collection);

    const auto& aa_template_itr = aa_template_tbl.find(uint64_t(template_id));

    if (aa_template_itr == aa_template_tbl.end()) {
        check(false, string("Template id: " + to_string(template_id) + " not found in collection " + collection.to_string()).c_str());
    }

    const auto& template_row = template_tbl.find(uint64_t(template_id));

    // --- Upsert the template --- //
    if (template_row == template_tbl.end()) {
        template_tbl.emplace(get_self(), [&](template_s& row) {
            row.template_id = template_id;
            row.collection = collection;
            row.timeunit_rate = timeunit_rate;
        });
    } else {
        template_tbl.modify(template_row, get_self(), [&](template_s& row) {
            row.template_id = template_id;
            row.collection = collection;
            row.timeunit_rate = timeunit_rate;
        });
    }

}

ACTION loot:: rmtemplates(const int32_t& template_id, const name& collection, const asset& timeunit_rate)
{
    // --- Authentication Check --- // 
    check(has_auth(get_self()), "this action is admin only");

    // --- Import config --- //
    const auto& config = check_config();

    // --- Get templates table  --- //
    template_t template_tbl(get_self(), get_self().value);

    const auto& template_row = template_tbl.find(uint64_t(template_id));

    // --- Erase the Template  --- //
    if (template_row != template_tbl.end()) {
        template_tbl.erase(template_row);
    }

}

ACTION loot::resetuser(const name& user)
{
    // --- Authentication Check --- // 
     check(has_auth(get_self()) || has_auth(user), "Missing authorization by admin or the user themselves.");

    // --- Import config --- //
    const auto& config = check_config();

    user_t user_tbl(get_self(), get_self().value);// Get users table

    const auto& user_itr = user_tbl.find(user.value);

    // --- Delete the User --- //
    if (user_itr != user_tbl.end()) {
        user_tbl.erase(user_itr);
    }

    // --- Get asset table --- //
    asset_t asset_tbl(get_self(), get_self().value);

    vector<uint64_t> staked_assets = {};

    // --- Get NFTs by owner --- //
    auto owner_idx = asset_tbl.get_index<"owner"_n>();
    auto owner_itr = owner_idx.lower_bound(user.value);

    // --- Erase NFTs from table --- //
    while (owner_itr != owner_idx.end() && owner_itr->owner == user) {
        staked_assets.push_back(owner_itr->asset_id);
        owner_idx.erase(owner_itr++);
    }

    // --- Return NFTs to owner --- //
    if (staked_assets.size() > 0) {
        action(permission_level { get_self(), name("active") }, atomicassets::ATOMICASSETS_ACCOUNT, name("transfer"),
            make_tuple(get_self(), user, staked_assets, string("Returning your NFTs, stake more anytime")))
            .send();
    }
}


ACTION loot::regnewuser(const name& user, const name& referrer ) {

    // --- Authentication Check --- // 
    if (!has_auth(user)) {
        check(false, string("User " + user.to_string() + " has not authorized this action.").c_str());
    }

    // --- Import config --- //
    const auto& config = check_config();
    asset stacked = asset(0, config.token_symbol);

    user_t user_tbl(get_self(), get_self().value); // Get users table 

    // --- See if user is already registered --- //
    auto user_itr = user_tbl.find(user.value);
    check(user_itr == user_tbl.end(), string("Looks like " + user.to_string() + " is already registered").c_str());

    // --- Handle referrer --- //
    if (referrer != ""_n && referrer != user) {

        // --- If the referrer is not registered, register them --- //
        auto referrer_itr = user_tbl.find(referrer.value);
        if (referrer_itr == user_tbl.end()) {

            // --- Check user exists --- //
            check(is_account(referrer), "Referring user can't be found.");

            user_tbl.emplace(get_self(), [&](auto& row) {
                row.user = referrer;
                row.stacked = stacked;
                row.referrer = ""_n; // No referrer for the referrer
                row.refscore = 1;
            });
        }
    }

        // --- Bonus if you are refered --- //
        uint32_t newscore;
        if (referrer != ""_n && referrer != user) {
            newscore = 1;
        } else {
            newscore = 0;
        }

    // --- Register the new user --- //
    user_tbl.emplace(get_self(), [&](auto& row) {
        row.user = user;
        row.stacked = stacked;
        row.referrer = (referrer != user) ? referrer : ""_n;
        row.refscore = newscore;
    });

    // --- Handle ref score --- //
    if (referrer != ""_n && referrer != user) {
        auto referrer_itr = user_tbl.find(referrer.value);
        if (referrer_itr != user_tbl.end()) {
            user_tbl.modify(referrer_itr, get_self(), [&](auto& row) {
                row.refscore++;

                // --- Increment the referrer's referrer score if exists --- //
                if (row.referrer != ""_n) {
                    auto ref_of_referrer_itr = user_tbl.find(row.referrer.value);
                    if (ref_of_referrer_itr != user_tbl.end()) {
                        user_tbl.modify(ref_of_referrer_itr, get_self(), [&](auto& ref_row) {
                            ref_row.refscore++;
                        });
                    }
                }
            });
        }
    }
}


ACTION loot::claim(const name& user, const name& collection)
{
    // --- Authentication Check --- //
    if (!has_auth(user)) {
        check(false, string("User " + user.to_string() + " has not authorized this action").c_str());
    }

    // --- Import config --- //
    const auto& config = check_config();
    const uint32_t &epoch = config.min_claim_period;// Get the min wait period from config 

    user_t user_tbl(get_self(), get_self().value);// Get users table

    const auto& user_itr = user_tbl.find(user.value);
    
    // --- Calculate Bonus Reward --- //
    uint64_t refscore = user_itr->refscore;
    uint32_t refscore_lvl;

    
    template_t template_tbl(get_self(), get_self().value);// get template table
   

   // --- Get the integer bonus table --- //
    for (size_t i = 0; i < tetrahedral.size(); ++i) {
        if (refscore > (tetrahedral[i] + 1)) {
            refscore_lvl = i + 1;
        }
    }

    // --- Calculate the total number of templates staked by the user --- //
    uint32_t template_lvl = 0;
    uint32_t multiplier = refscore_lvl;


    // --- Check if the user is registered --- //
    if (user_itr == user_tbl.end()) {
        check(false, string("Wild Puma codenamed " + user.to_string() + " is not registered").c_str());
    }

    // --- Keep track of NFT counts for each template --- //
    std::map<int32_t, uint32_t> template_nft_counts;

    asset claimed_amount = asset(0, config.token_symbol);

    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());// get the assets table (scoped to the contract)


    check(claimed_amount.amount > 0, "No rewards to claim. Wait a bit.");// Fail if the reward is 0

    // --- Add claimed amount to stacked --- // 
    user_tbl.modify(user_itr, same_payer, [&](auto& row) { row.stacked += claimed_amount; });

    // --- Count the number of templates with more than one NFT --- //
    uint32_t templates_with_multiple_nfts = 0;
    for (const auto& pair : template_nft_counts) {
        if (pair.second > 1) {
            templates_with_multiple_nfts++;
        }
    }


    // --- Make memo different if user claims rewards from more than one template --- //
    string memo; 
    if (templates_with_multiple_nfts > 1){
        memo = string("Loot! " + to_string(refscore) + " referrals, " + to_string(refscore_lvl) + "X bonus, and " + to_string(template_lvl) + "X for # staked, totaling " + to_string(multiplier) + "X!! via lamanadapuma collection. Seek el White Puma: https://puma.red");
    } else {
        memo = string("Loot! " + to_string(refscore) + " referrals, " + to_string(refscore_lvl) + "X bonus, and " + to_string(templates_with_multiple_nfts) + " templates with bonuses! via lamanadapuma collection. Seek el White Puma: https://puma.red");

    } 

    // --- Send the reward tokens --- //
    action(permission_level { get_self(), name("active") }, config.token_contract, name("transfer"),
        make_tuple(get_self(), user, claimed_amount, memo))
        .send();
}





ACTION loot::unstake(const name& user, const vector<uint64_t>& asset_ids)
{
    // --- Authentication Check --- //
    if (!has_auth(user)) {
        check(false, string("user " + user.to_string() + " has not authorized this action").c_str());
    }

    check(asset_ids.size() > 0, "Must unstake at least 1 asset");

    // --- Import config --- //
    const auto& config = check_config();

    // --- Handle user --- //
    user_t user_tbl(get_self(), get_self().value);// get users table
    const auto& user_itr = user_tbl.find(user.value);
    
    if (user_itr == user_tbl.end()) {// Check the user is registered
        check(false, string("user " + user.to_string() + " is not registered").c_str());
    }
    
    template_t template_tbl(get_self(), get_self().value);// get template table 
  
    asset_t asset_tbl(get_self(), get_self().value);// get asset table 

    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());// get the assets table (scoped to the contract)

    // --- Get user's assets --- //
    for (const uint64_t& asset_id : asset_ids) {
        const auto& asset_itr = asset_tbl.find(asset_id);// Find the asset data, get the template id 

        if (asset_itr == asset_tbl.end()) {// Check if the asset is staked
            check(false, string("NFT id: " + to_string(asset_id) + " is not staked").c_str());
        }

        if (asset_itr->owner != user) {// Check if the asset belongs to the user
            check(false, string("NFT id: " + to_string(asset_id) + " does not belong to " + user.to_string()).c_str());
        }

        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);// Find the asset data, get the template id 

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("NFT id: " + to_string(asset_id) + " does not exist").c_str());
        }

        const auto& template_itr = template_tbl.find(aa_asset_itr->template_id);// Check if the asset's template is stakeable

        if (template_itr == template_tbl.end()) {
            check(false, string("NFT id: " + to_string(asset_id) + " is not stakeable").c_str());
        }

        auto period_sec = current_time_point().sec_since_epoch() - asset_itr->last_claim.sec_since_epoch();

        // check if the asset can be unstaked
        if (period_sec < config.unstake_period) {
            check(false, string("NFT id: " + to_string(asset_id) + " cannot be unstaked yet").c_str());
        }

        asset_tbl.erase(asset_itr);// delete the NFTs
    }

    // send the assets back
    action(permission_level { get_self(), name("active") }, atomicassets::ATOMICASSETS_ACCOUNT, name("transfer"),
        make_tuple(get_self(), user, asset_ids, string("Unstaking Loot")))
        .send();
}

[[eosio::on_notify("atomicassets::transfer")]] void
loot::receiveassets(name from, name to, vector<uint64_t> asset_ids, string memo)
{
   
    if (to != get_self() || from == get_self()) {// Ignore outgoing transactions and transaction not destined to the dapp itself
        return;
    }

    // --- Import config --- //
    const auto& config = check_config();

    // --- Get users table --- //
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(from.value);

    // --- Check if the user is registered --- //
    if (user_itr == user_tbl.end()) {
        check(false, string("user " + from.to_string() + " is not registered").c_str());
    }

    // --- Get the assets table (scoped to the contract) --- //
    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());

    template_t template_tbl(get_self(), get_self().value);// get template table 

    asset_t asset_tbl(get_self(), get_self().value);// get asset table 

    asset added_rate = asset(0, config.token_symbol);

    for (const uint64_t& asset_id : asset_ids) {
        // --- Get the Template ID --- //
        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("NFT id: " + to_string(asset_id) + " does not exist").c_str());
        }

        // --- Check the template is stakeable --- //
        const auto& template_itr = template_tbl.find(aa_asset_itr->template_id);

        if (template_itr == template_tbl.end()) {
            check(false, string("NFT id: " + to_string(asset_id) + " is not stakeable. Collection owner must add it first.").c_str());
        }

        // --- Store the asset --- //
        asset_tbl.emplace(get_self(), [&](asset_s& row) {
            row.asset_id = asset_id;
            row.owner = from;
            row.last_claim = time_point_sec(current_time_point());
        });
    }


}