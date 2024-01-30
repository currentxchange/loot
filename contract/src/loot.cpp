#include "loot.hpp"


ACTION loot::setnftcolrew(const name& user, const name& collection, const symbol& token_symbol, const name& token_contract, const uint32_t& time_unit_length, const uint32_t& unstake_period,
                        const string& reward_series_referral = string("TETRAHEDRAL"), const double& reward_coefficient_referral = 1.0, const string& reward_series_hodl = string("TETRAHEDRAL"), const double& reward_coefficient_hodl = 1.0){
    // --- Authorization Check --- // 
   check(has_auth(get_self()) || has_auth(user), "Missing authorization by user.");
   check(isAuthorized(collection, user), "Missing Authorization. To start rewards, you must be authorized for the collection on atomicassets.");

    // --- Check contract at address --- //
    check(is_account(token_contract), "Token contract account does not exist");

    // --- Basic Token Check --- //
    stat_t stat(token_contract, token_symbol.code().raw());
    stat.require_find(token_symbol.code().raw(), "Token symbol does not exist on the token contract");

    // --- Initialize config_t --- //'
    config_t config_tbl(get_self(), get_self().value);
    auto config_itr = config_tbl.find(collection.value);

    // --- Upsert the config --- //
    if (config_itr == config_tbl.end()) {
        config_tbl.emplace(user, [&](auto& row) {
            row.creator = user;
            row.collection = collection;
            row.token_contract = token_contract;
            row.token_symbol = token_symbol;
            row.time_unit_length = time_unit_length;
            row.unstake_period = unstake_period;
            row.reward_series_referral = reward_series_referral;
            row.reward_coefficient_referral = reward_coefficient_referral;
            row.reward_series_hodl = reward_series_hodl;
            row.reward_coefficient_hodl = reward_coefficient_hodl;
        });
    } else {
        config_tbl.modify(config_itr, user, [&](auto& row) {
            row.token_contract = token_contract;
            row.token_symbol = token_symbol;
            row.time_unit_length = time_unit_length;
            row.unstake_period = unstake_period;
            row.reward_series_referral = reward_series_referral;
            row.reward_coefficient_referral = reward_coefficient_referral;
            row.reward_series_hodl = reward_series_hodl;
            row.reward_coefficient_hodl = reward_coefficient_hodl;
        });
    }
   
                        
}



ACTION loot::addtemplates(const name& user, const uint32_t& template_id, const name& collection, const asset& timeunit_rate){
    // --- Authentication Check --- //
    check(has_auth(user) || has_auth(get_self()), "Missing authorization by user.");
    check(isAuthorized(collection, user), "Missing Authorization. To start rewards, you must be authorized for the collection on atomicassets.");

    // --- Access Config Collection Table --- //
    config_t config_tbl(get_self(), get_self().value);
    auto config_itr = config_tbl.find(collection.value);
    check(config_itr != config_tbl.end(), "Collection config not found.");

    // --- Check if the timeunit_rate symbol matches the collection config symbol --- //
    check(timeunit_rate.amount > 0, "timeunit_rate must be positive"); // Check if the hourly rate is valid
    check(config_itr->token_symbol == timeunit_rate.symbol, "symbol mismatch");

    // --- Check if the Template Exists in AtomicAssets and is Valid --- //
    auto aa_template_tbl = atomicassets::get_templates(collection);
    auto aa_template_itr = aa_template_tbl.find(uint64_t(template_id));
    check(aa_template_itr != aa_template_tbl.end(), "Template id: " + std::to_string(template_id) + " not found in collection " + collection.to_string());

    // --- Get Templates Table --- //
    template_t template_tbl(get_self(), get_self().value);
    auto template_row = template_tbl.find(uint64_t(template_id));

    // --- Upsert the Template --- //
    if (template_row == template_tbl.end()) {
        template_tbl.emplace(get_self(), [&](auto& row) {
            row.template_id = template_id;
            row.collection = collection;
            row.timeunit_rate = timeunit_rate;
        });
    } else {
        template_tbl.modify(template_row, get_self(), [&](auto& row) {
            row.template_id = template_id;
            row.collection = collection;
            row.timeunit_rate = timeunit_rate;
        });
    }
}


ACTION loot::rmtemplates(const name& user, const uint32_t& template_id, const name& collection) {
    // --- Authentication Check --- //
   check(isAuthorized(collection, user), "Missing Authorization. To remove templates, you must be authorized for the collection on atomicassets.");

    // --- Access Config Collection Table --- //
    config_t config_tbl(get_self(), get_self().value);
    auto config_itr = config_tbl.find(collection.value);
    check(config_itr != config_tbl.end(), "Collection config not found.");

    // --- Get Templates Table --- //
    template_t template_tbl(get_self(), get_self().value);
    auto template_row = template_tbl.find(uint64_t(template_id));

    // --- Erase the Template --- //
    check(template_row != template_tbl.end(), "Template not found in the collection.");
    template_tbl.erase(template_row);
}

ACTION loot::resetuser(const name& user){
    // --- Authentication Check --- // 
     check(has_auth(get_self()) || has_auth(user), "Missing authorization by admin or the user themselves.");

    // --- Delete the User --- //
    user_t user_tbl(get_self(), get_self().value);
    const auto& user_itr = user_tbl.find(user.value);
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


ACTION loot::regnewuser(const name& user, const name& referrer = ""_n) {

    // --- Authentication Check --- // 
    if (!has_auth(user)) {
        check(false, string("User " + user.to_string() + " has not authorized this action.").c_str());
    }


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



ACTION loot::claim(const name& user, const name& collection) {
    // --- Authentication Check --- //
    check(has_auth(user), "User " + user.to_string() + " has not authorized this action");

    // --- Access Tables --- //
    config_t config_tbl(get_self(), get_self().value);
    auto config_itr = config_tbl.find(collection.value);
    check(config_itr != config_tbl.end(), "Collection configuration not found.");

    user_templ_t user_template_tbl(get_self(), user.value);
    template_t template_tbl(get_self(), get_self().value);

    // --- Initialize reward tabulator --- //
    asset claimed_amount(0, config_itr->token_symbol);

    // --- Get reward series --- //
    vector<uint64_t> reward_series_referral = getSeries(string(config_itr->reward_series_referral));
    vector<uint64_t> reward_series_hodl = getSeries(string(config_itr->reward_series_hodl));

    //TODO Ensure there's a 0-reward option (MAke it intuitive)
    // --- Determine the user's position in the referral reward series --- //
    user_t user_tbl(get_self(), get_self().value);
    const auto& user_itr = user_tbl.find(user.value);

    uint64_t refscore = user_itr->refscore;
    uint32_t refscore_lvl;

    if (reward_series_referral != std::vector<uint64_t>()){
        uint32_t series_size_referral = reward_series_referral.size();

        for (size_t i = 0; i < series_size_referral; ++i) {
            if (refscore < reward_series_referral[i]) {
                refscore_lvl = i + 1; // Update the refscore level based on the series
                break;
            }
        }
    } else { 
        // --- Ensure refscore_lvl isn't 0 for multiplication --- //
        (refscore) ? refscore_lvl = refscore : refscore_lvl = 1 ;
    }


    uint32_t series_size_hodl;
    uint32_t template_count;
    uint32_t hodl_lvl;
    asset reward_for_template(0, config_itr->token_symbol);
    uint32_t time_units_passed;



    // --- Calculate reward for each template --- //
    for (auto user_template_itr = user_template_tbl.begin(); user_template_itr != user_template_tbl.end(); ++user_template_itr) {
        if (user_template_itr->collection == collection) {
        
            // Get the template info
            auto template_itr = template_tbl.find(user_template_itr->template_id);
            check(template_itr != template_tbl.end(), "Template not found.");

            // Calculate the reward for this template
            time_units_passed = ( current_time_point().sec_since_epoch() - user_template_itr->last_claim.sec_since_epoch() )/ config_itr->time_unit_length;
            reward_for_template.amount = time_units_passed * user_template_itr->amount_staked * template_itr->timeunit_rate.amount;
            
            series_size_hodl = reward_series_hodl.size();

            template_count = user_template_itr->amount_staked;

            // --- Get hodl level --- //
            if (reward_series_hodl != std::vector<uint64_t>()){
                for (size_t i = 0; i < series_size_hodl; ++i) {
                    if (template_count < reward_series_hodl[i]) {
                        hodl_lvl = i + 1;
                        break;
                    }
                }
            } else { 
                // --- Linear reward series --- //
                hodl_lvl = template_count;
            }

            // --- Add to the claimed amount --- //
            claimed_amount.amount += (
                (config_itr->reward_coefficient_referral > 0 ? refscore_lvl * config_itr->reward_coefficient_referral : 1.0) *
                (config_itr->reward_coefficient_hodl > 0 ? hodl_lvl * config_itr->reward_coefficient_hodl : 1.0) *
                reward_for_template.amount
            );

            user_template_tbl.modify(user_template_itr, get_self(), [&](auto& row) {
                row.last_claim = time_point_sec(current_time_point());
            });
        
        }
    }

    check(claimed_amount.amount > 0, "No rewards to claim. Wait a bit.");

    // --- Create memo for transaction --- //
    //string memo = "Claimed rewards via " + collection.to_string() + " collection.";


    // --- Debugging Memo --- //
    string memo = "";
    memo += "HODL Lvl: " + std::to_string(hodl_lvl) + ", ";
    memo += "Bonus: " + std::to_string(config_itr->reward_coefficient_hodl) + "x, ";
    memo += "Invite Lvl: " + std::to_string(refscore_lvl) + ", ";
    memo += "Bonus: " + std::to_string(refscore) + "x, ";
    memo += "Reward tpl: " + std::to_string(reward_for_template.amount) + "TUs: " + std::to_string(time_units_passed);
   


    // --- Send the reward tokens --- //
    action(
        permission_level { get_self(), name("active") },
        config_itr->token_contract,
        name("transfer"),
        make_tuple(get_self(), user, claimed_amount, memo)
    ).send();
}




ACTION loot::unstake(const name& user, const vector<uint64_t>& asset_ids) {
    // --- Authentication Check --- //
    check(has_auth(user), "user " + user.to_string() + " has not authorized this action");

    check(asset_ids.size() > 0, "Must unstake at least 1 asset");

    user_templ_t user_template_tbl(get_self(), user.value);
    asset_t asset_tbl(get_self(), get_self().value); // get asset table 
    template_t template_tbl(get_self(), get_self().value); // get template table 

    // --- Iterate over each asset to unstake --- //
    for (const uint64_t& asset_id : asset_ids) {
        auto asset_itr = asset_tbl.find(asset_id); // Find the asset data
        check(asset_itr != asset_tbl.end(), "NFT id: " + std::to_string(asset_id) + " is not staked");
        check(asset_itr->owner == user, "NFT id: " + std::to_string(asset_id) + " does not belong to " + user.to_string());

        // --- Get the assets table --- //
        const auto& aa_asset_tbl = atomicassets::get_assets(get_self());
        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("assert (" + to_string(asset_id) + ") does not exist").c_str());
        }

        // --- Access the collection's config --- //
        config_t config_tbl(get_self(), get_self().value);
        auto config_itr = config_tbl.find(aa_asset_itr->collection_name.value);
        check(config_itr != config_tbl.end(), "Config not found for collection: " + aa_asset_itr->collection_name.to_string());

        // --- Check if the asset can be unstaked (based on unstake_period) --- //
        auto period_sec = current_time_point().sec_since_epoch() - asset_itr->last_claim.sec_since_epoch();
        check(period_sec >= config_itr->unstake_period, "NFT id: " + std::to_string(asset_id) + " cannot be unstaked yet");

        // --- Update the amount staked in the user_templ_s table --- //
        auto user_template_itr = user_template_tbl.find(user.value);

        for (auto user_template_itr = user_template_tbl.begin(); user_template_itr != user_template_tbl.end(); ++user_template_itr) {
            if (user_template_itr->collection.value == aa_asset_itr->collection_name.value) {
                //TODO delete record when user unstakes all NFTs
                user_template_tbl.modify(user_template_itr, get_self(), [&](auto& row) {
                    row.amount_staked -= 1; // Decrement by 1 for each unstaked asset
                });
            }
        }

        // --- Erase the record from the asset table --- //
        asset_tbl.erase(asset_itr);
    }

    // --- Send the assets back --- //
    action(
        permission_level { get_self(), name("active") },
        atomicassets::ATOMICASSETS_ACCOUNT,
        name("transfer"),
        make_tuple(get_self(), user, asset_ids, string("Unstaking Loot"))
    ).send();

}


ACTION loot::refund(const name& user, const name& collection, const asset& refund_amount) {
    // --- Authentication _ Validity Checks --- //
    check(has_auth(user), "user " + user.to_string() + " has not authorized this action");
    check(refund_amount.amount > 0, "Refund amount must be positive");
    check(isAuthorized(collection, user), "User is not authorized for this collection");

    // --- Access the bank table --- //
    bank_t bank_tbl(get_self(), get_self().value);
    auto bank_itr = bank_tbl.find(collection.value);

    // --- Check if there is a record for the collection --- //
    check(bank_itr != bank_tbl.end(), "No record found for this collection in the bank");

    // --- Check if there are enough funds to refund --- //
    check(bank_itr->amount >= refund_amount, "Insufficient funds to refund");

    // --- Send the refund to the user --- //
    action(
        permission_level{get_self(), "active"_n},
        bank_itr->token_contract, // Assuming the token contract is stored in the bank record
        "transfer"_n,
        std::make_tuple(get_self(), user, refund_amount, std::string("Refund for collection: ") + collection.to_string())
    ).send();

        // --- Update the record with the remaining amount --- //
        bank_tbl.modify(bank_itr, same_payer, [&](auto& row) {
            row.amount -= refund_amount;
        });
}


    [[eosio::on_notify("atomicassets::transfer")]] void
    loot::receiveassets(name from, name to, vector<uint64_t> asset_ids, string memo) {
        
        if (to != get_self() || from == get_self()) {// Ignore outgoing transactions and transaction not destined to the dapp itself
            return;
        }

        // --- Get users table --- //
        user_t user_tbl(get_self(), get_self().value);

        const auto& user_itr = user_tbl.find(from.value);

        // --- Check if the user is registered --- //
        if (user_itr == user_tbl.end()) {
            check(false, string("user " + from.to_string() + " is not registered").c_str());
        }

        // --- Get the assets table (scoped to the contract) --- //
        const auto& aa_asset_tbl = atomicassets::get_assets(get_self());
        asset_t asset_tbl(get_self(), get_self().value);// get asset table 
        template_t template_tbl(get_self(), get_self().value);
        user_templ_t user_template_tbl(get_self(), from.value);

        // --- Iterate through each asset received --- //
        for (const uint64_t& asset_id : asset_ids) {
            // --- Check the atomic assets table --- //
            auto aa_asset_itr = aa_asset_tbl.find(asset_id);
            check(aa_asset_itr != aa_asset_tbl.end(), "Asset ID: " + std::to_string(asset_id) + " does not exist.");

            // --- Check if the assert is stackable --- //
            auto template_itr = template_tbl.find(aa_asset_itr->template_id);
            check(template_itr != template_tbl.end(), "Template ID: " + std::to_string(aa_asset_itr->template_id) + " is not stakeable. Collection owner must add it first.");

            // --- Store a record in the assets table --- //
            asset_tbl.emplace(get_self(), [&](auto& row) {
                row.asset_id = asset_id;
                row.owner = from;
                row.last_claim = time_point_sec(current_time_point());
            });

            // --- Update or insert into the user assets  --- //

            auto newtempid = aa_asset_itr->collection_name;
            auto user_template_itr = user_template_tbl.find(from.value);
            if (user_template_itr == user_template_tbl.end()) {
                user_template_tbl.emplace(get_self(), [&](auto& row) {
                    row.user = from;
                    row.collection = newtempid;
                    row.template_id = aa_asset_itr->template_id;
                    row.amount_staked = 1; // Increment by 1 for each asset
                    row.last_claim = time_point_sec(current_time_point());
                });
            } else {
                user_template_tbl.modify(user_template_itr, get_self(), [&](auto& row) {
                    row.amount_staked += 1; // Increment by 1 for each asset
                });
            }
        }
    }

    [[eosio::on_notify("*::transfer")]] void loot::on_transfer(name from, name to, asset quantity, string memo) {
        if (to != get_self() || from == get_self()) {
            return;
        }

        // --- Parse the memo for the collection name --- //
        name collection = name(memo);

        // --- Access the config table, scoped to this contract --- //
        config_t config_tbl(get_self(), get_self().value);
        auto config_itr = config_tbl.find(collection.value);

        // --- Check if the collection exists in the config --- //
        check(config_itr != config_tbl.end(), "Memo must be the collection name. Collection not found.");

        // --- Check if the token is the correct type for this collection --- //
        check(quantity.symbol == config_itr->token_symbol, "Incorrect token type for this collection");
        check(get_first_receiver() == config_itr->token_contract, "Incorrect token contract for this collection");

        // --- Check if the sender is authorized for the collection -- //
        check(isAuthorized(collection, from), "Sender is not authorized for this collection");

        // --- Access the bank table, scoped to this contract --- //
        bank_t bank_tbl(get_self(), get_self().value);
        auto bank_itr = bank_tbl.find(collection.value);

        // --- Insert or update the bank record --- //
        if (bank_itr == bank_tbl.end()) {

            // --- If the collection is not in the bank, add it --- //
            bank_tbl.emplace(get_self(), [&](auto& row) {
                row.collection = collection;
                row.token_contract = get_first_receiver(); // Store the contract of the token
                row.amount = quantity;
            });
        } else {
            // --- If the collection is already in the bank, update the amount --- //
            bank_tbl.modify(bank_itr, same_payer, [&](auto& row) {
                check(row.token_contract == get_first_receiver(), "Token contract mismatch. You can only have one reward token per collection.");
                row.amount += quantity;
            });
        }
    }