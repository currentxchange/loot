#include "loot.hpp"
// (300, 300)
ACTION loot::setconfig(const uint32_t& min_claim_period, const uint32_t& unstake_period)
{
    // check contract auth
    check(has_auth(get_self()), "this action is admin only");

    // get config table instance
    config_t conf_tbl(get_self(), get_self().value);

    // get/create current config
    auto conf = conf_tbl.get_or_default(config {});

    conf.min_claim_period = min_claim_period;
    conf.unstake_period = unstake_period;

    // save the new config
    conf_tbl.set(conf, get_self());
}

ACTION loot::settoken(const name& contract, const symbol& symbol)
{
    // check contract auth
    check(has_auth(get_self()), "this action is admin only");

    // check if the contract exists
    check(is_account(contract), "contract account does not exist");

    // check if the contract has a token and is valid
    stat_t stat(contract, symbol.code().raw());
    stat.require_find(symbol.code().raw(), "token symbol does not exist");

    // get config table instance
    config_t conf_tbl(get_self(), get_self().value);

    // get/create current config
    auto conf = conf_tbl.get_or_default(config {});

    conf.token_contract = contract;
    conf.token_symbol = symbol;

    // save the new config
    conf_tbl.set(conf, get_self());
}

ACTION loot::addtemplates(const int32_t& template_id, const name& collection, const asset& timeunit_rate)
{
    // check contract auth
    check(has_auth(get_self()), "this action is admin only");

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get templates table instance
    template_t template_tbl(get_self(), get_self().value);


    // check if the hourly rate is valid
    check(t.timeunit_rate.amount > 0, "timeunit_rate must be positive");
    check(config.token_symbol == t.timeunit_rate.symbol, "symbol mismatch");

    // check if the template exists in atomicassets and it's valid
    const auto& aa_template_tbl = atomicassets::get_templates(t.collection);

    const auto& aa_template_itr = aa_template_tbl.find(uint64_t(t.template_id));

    if (aa_template_itr == aa_template_tbl.end()) {
        check(false, string("template (" + to_string(t.template_id) + ") not found in collection " + t.collection.to_string()).c_str());
    }

    const auto& template_row = template_tbl.find(uint64_t(t.template_id));

    // insert the new template or update it if it already exists
    if (template_row == template_tbl.end()) {
        template_tbl.emplace(get_self(), [&](template_s& row) {
            row.template_id = t.template_id;
            row.collection = t.collection;
            row.timeunit_rate = t.timeunit_rate;
        });
    } else {
        template_tbl.modify(template_row, get_self(), [&](template_s& row) {
            row.template_id = t.template_id;
            row.collection = t.collection;
            row.timeunit_rate = t.timeunit_rate;
        });
    }

}

ACTION loot::rmvtemplates(const int32_t& template_id, const name& collection, const asset& timeunit_rate)
{
    // check contract auth
    check(has_auth(get_self()), "this action is admin only");

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get templates table instance
    template_t template_tbl(get_self(), get_self().value);


    const auto& template_row = template_tbl.find(uint64_t(t.template_id));

    // erase the template if it already exists
    if (template_row != template_tbl.end()) {
        template_tbl.erase(template_row);
    }

}

ACTION loot::resetuser(const name& user)
{
    // check contract auth
    check(has_auth(get_self()), "this action is admin only");

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get users table instance
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(user.value);

    // erase the user if the it is already registered
    if (user_itr != user_tbl.end()) {
        user_tbl.erase(user_itr);
    }

    // get asset table instance
    asset_t asset_tbl(get_self(), get_self().value);

    vector<uint64_t> staked_assets = {};

    // get the secondary index
    //auto owner_idx = asset_tbl.get_index<name("owner")>();
    auto owner_idx = asset_tbl.get_index<"owner"_n>();
    auto owner_itr = owner_idx.lower_bound(user.value);

    // iterate through the rows and erase them
    while (owner_itr != owner_idx.end() && owner_itr->owner == user) {
        staked_assets.push_back(owner_itr->asset_id);
        owner_idx.erase(owner_itr++);
    }

    // return the assets back to the user if there's any
    if (staked_assets.size() > 0) {
        // send the assets back
        action(permission_level { get_self(), name("active") }, atomicassets::ATOMICASSETS_ACCOUNT, name("transfer"),
            make_tuple(get_self(), user, staked_assets, string("Unstaking")))
            .send();
    }
}

/*/
ACTION loot::regnewuser(const name& user)
{
    // check user auth
    if (!has_auth(user)) {
        check(false, string("user " + user.to_string() + " has not authorized this action").c_str());
    }

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get users table instance
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(user.value);

    // check if the user isn't already registered
    if (user_itr != user_tbl.end()) {
        check(false, string("user " + user.to_string() + " is already registered").c_str());
    }

    user_tbl.emplace(user, [&](user_s& row) {
        row.user = user;
        row.timeunit_rate = asset(0, config.token_symbol);
    });
}
/*/

ACTION loot::regnewuser(const name& user, const name& referrer ) {
    // Check user auth
    if (!has_auth(user)) {
        check(false, string("user " + user.to_string() + " has not authorized this action").c_str());
    }

    // Check if the contract isn't frozen
    const auto& config = check_config();

    // Get users table instance
    user_t user_tbl(get_self(), get_self().value);

    // Check if the user isn't already registered
    auto user_itr = user_tbl.find(user.value);
    check(user_itr == user_tbl.end(), string("user " + user.to_string() + " is already registered").c_str());

    // Handle referrer
    if (referrer != ""_n && referrer != user) {
        // Check if the referrer is already registered
        auto referrer_itr = user_tbl.find(referrer.value);

        // If the referrer is not registered, register the referrer
        if (referrer_itr == user_tbl.end()) {
            user_tbl.emplace(get_self(), [&](auto& row) {
                row.user = referrer;
                row.timeunit_rate = asset(0, config.token_symbol);
                row.referrer = ""_n; // No referrer for the referrer
                row.refscore = 0;
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

    // Register the new user
    user_tbl.emplace(get_self(), [&](auto& row) {
        row.user = user;
        row.timeunit_rate = asset(0, config.token_symbol);
        row.referrer = (referrer != user) ? referrer : ""_n;
        row.refscore = newscore;
    });

    // Increment referrer's and referrer's referrer score
    if (referrer != ""_n && referrer != user) {
        auto referrer_itr = user_tbl.find(referrer.value);
        if (referrer_itr != user_tbl.end()) {
            user_tbl.modify(referrer_itr, get_self(), [&](auto& row) {
                row.refscore++;

                // Increment the referrer's referrer score if exists
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


/*/
ACTION loot::claim(const name& user, const vector<uint64_t>& asset_ids)
{
    // check user auth
    if (!has_auth(user)) {
        check(false, string("user " + user.to_string() + " has not authorized this action").c_str());
    }

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get users table instance
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(user.value);

    // check if the user is registered
    if (user_itr == user_tbl.end()) {
        check(false, string("user " + user.to_string() + " is not registered").c_str());
    }

    // get template table instance
    template_t template_tbl(get_self(), get_self().value);
    // get asset table instance
    asset_t asset_tbl(get_self(), get_self().value);

    asset claimed_amount = asset(0, config.token_symbol);

    // get the assets table (scoped to the contract)
    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());

    for (const uint64_t& asset_id : asset_ids) {
        // find the asset data, to get the template id from it
        const auto& asset_itr = asset_tbl.find(asset_id);

        // check if the asset is staked
        if (asset_itr == asset_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") is not staked").c_str());
        }

        // check if the asset belongs to the user
        if (asset_itr->owner != user) {
            check(false, string("asset (" + to_string(asset_id) + ") does not belong to " + user.to_string()).c_str());
        }

        // find the asset data, to get the template id from it
        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") does not exist").c_str());
        }

        // check if the asset's template is stakeable
        const auto& template_itr = template_tbl.find(aa_asset_itr->template_id);

        if (template_itr == template_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") is not stakeable").c_str());
        }

        auto period_sec = current_time_point().sec_since_epoch() - asset_itr->last_claim.sec_since_epoch();

        // check if the asset is not in cooldown
        if (period_sec < config.min_claim_period) {
            check(false, string("asset (" + to_string(asset_id) + ") is still in cooldown").c_str());
        }

        // increment the claimed amount
        claimed_amount.amount += (template_itr->timeunit_rate.amount * period_sec) / 300;

        // reset the last claim time
        asset_tbl.modify(asset_itr, user, [&](asset_s& row) { row.last_claim = current_time_point(); });
    }

    // fail if the reward is 0
    check(claimed_amount.amount > 0, "nothing to claim");

    // send the tokens
    action(permission_level { get_self(), name("active") }, config.token_contract, name("transfer"),
        make_tuple(get_self(), user, claimed_amount, string("Staking reward")))
        .send();
}
/*/

//





ACTION loot::claim(const name& user, const vector<uint64_t>& asset_ids)
{
    // check user auth
    if (!has_auth(user)) {
        check(false, string("user " + user.to_string() + " has not authorized this action").c_str());
    }

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get users table instance
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(user.value);
    
    // --- Calculate Bonus Reward --- //
    uint64_t refscore = user_itr->refscore;
    uint32_t refscore_lvl;

    // get template table instance
    template_t template_tbl(get_self(), get_self().value);
    // get asset table instance
    asset_t asset_tbl(get_self(), get_self().value);


    std::vector<uint64_t> tetrahedral = {1, 4, 10, 20, 35, 56, 84, 120, 165, 220, 286, 364, 455, 560, 680, 816, 969, 1140, 1330, 1540, 1771, 2024, 2300, 2600, 2925, 3276, 3654, 4060, 4495, 4960, 5456, 5984, 6545, 7140, 7770, 8436, 9139, 9880, 10660, 11480, 12341, 13244, 14190, 15180, 999999999};

    for (size_t i = 0; i < tetrahedral.size(); ++i) {
        if (refscore > (tetrahedral[i] + 1)) {
            refscore_lvl = i + 1;
        }
    }


    // Calculate the total number of templates staked by the user
    uint32_t total_templates = 0;
    uint32_t template_lvl = 0;

    auto owner_index = asset_tbl.get_index<"owner"_n>();
    auto owner_itr = owner_index.find(user.value);
    if (owner_itr != owner_index.end())
    {
        //total_templates = owner_index.rows.at(0).count;
        auto it = owner_itr;
            auto end = owner_index.end();
            while (it != end) {
                total_templates++;
                it++;
            }
    }


    for (size_t i = 0; i < tetrahedral.size(); ++i) {
        if (total_templates > (tetrahedral[i] + 1)) {
            template_lvl = i + 1;
        }
    }


    uint32_t multiplier = template_lvl * refscore_lvl;
    // --- Sanity check --- //
    check(multiplier < 999, string("You've broken the timespace continuum").c_str());

    // check if the user is registered
    if (user_itr == user_tbl.end()) {
        check(false, string("Wild Puma codenamed " + user.to_string() + " is not registered").c_str());
    }



    asset claimed_amount = asset(0, config.token_symbol);

    // get the assets table (scoped to the contract)
    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());

    for (const uint64_t& asset_id : asset_ids) {
        // find the asset data, to get the template id from it
        const auto& asset_itr = asset_tbl.find(asset_id);

        // check if the asset is staked
        if (asset_itr == asset_tbl.end()) {
            check(false, string("puma asset (" + to_string(asset_id) + ") is not staked").c_str());
        }

        // check if the asset belongs to the user
        if (asset_itr->owner != user) {
            check(false, string("puma asset  (" + to_string(asset_id) + ") does not belong to " + user.to_string()).c_str());
        }

        // find the asset data, to get the template id from it
        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("puma asset  (" + to_string(asset_id) + ") does not exist").c_str());
        }

        // check if the asset's template is stakeable
        const auto& template_itr = template_tbl.find(aa_asset_itr->template_id);

        if (template_itr == template_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") is not stakeable").c_str());
        }

        auto period_sec = current_time_point().sec_since_epoch() - asset_itr->last_claim.sec_since_epoch();

        // check if the asset is not in cooldown
        if (period_sec < config.min_claim_period) {
            check(false, string("asset (" + to_string(asset_id) + ") isn't ripe to collect. 5 minutes between claims.").c_str());
        }

        // increment the claimed amount + add multiplier 
        claimed_amount.amount += (template_itr->timeunit_rate.amount * period_sec) / 300 * multiplier;

        // reset the last claim time
        asset_tbl.modify(asset_itr, user, [&](asset_s& row) { row.last_claim = current_time_point(); });
    }

    // fail if the reward is 0
    check(claimed_amount.amount > 0, "nothing to claim");

    // send the tokens
    action(permission_level { get_self(), name("active") }, config.token_contract, name("transfer"),
        make_tuple(get_self(), user, claimed_amount, string("Loot reward with referral bonus multiplier of " + to_string(refscore) + "via lamanadapuma collection. Seek el White Puma: https://puma.red")))
        .send();
}


ACTION loot::unstake(const name& user, const vector<uint64_t>& asset_ids)
{
    // check user auth
    if (!has_auth(user)) {
        check(false, string("user " + user.to_string() + " has not authorized this action").c_str());
    }

    // check if there's any requested asset_ids at all
    check(asset_ids.size() > 0, "must unstake at least 1 asset");

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get users table instance
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(user.value);

    // check if the user is registered
    if (user_itr == user_tbl.end()) {
        check(false, string("user " + user.to_string() + " is not registered").c_str());
    }

    // get template table instance
    template_t template_tbl(get_self(), get_self().value);
    // get asset table instance
    asset_t asset_tbl(get_self(), get_self().value);

    asset removed_rate = asset(0, config.token_symbol);

    // get the assets table (scoped to the contract)
    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());

    for (const uint64_t& asset_id : asset_ids) {
        // find the asset data, to get the template id from it
        const auto& asset_itr = asset_tbl.find(asset_id);

        // check if the asset is staked
        if (asset_itr == asset_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") is not staked").c_str());
        }

        // check if the asset belongs to the user
        if (asset_itr->owner != user) {
            check(false, string("asset (" + to_string(asset_id) + ") does not belong to " + user.to_string()).c_str());
        }

        // find the asset data, to get the template id from it
        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") does not exist").c_str());
        }

        // check if the asset's template is stakeable
        const auto& template_itr = template_tbl.find(aa_asset_itr->template_id);

        if (template_itr == template_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") is not stakeable").c_str());
        }

        auto period_sec = current_time_point().sec_since_epoch() - asset_itr->last_claim.sec_since_epoch();

        // check if the asset can be unstaked
        if (period_sec < config.unstake_period) {
            check(false, string("asset (" + to_string(asset_id) + ") cannot be unstaked yet").c_str());
        }

        // increment the removed amount
        removed_rate += template_itr->timeunit_rate;

        // remove the assets from the user's staked assets
        asset_tbl.erase(asset_itr);
    }

    // sanity check
    // this should never happen unless the template rate was changed after staking
    check(removed_rate <= user_itr->timeunit_rate, "unstaked rate larger than user's rate; this shouldn't happen !!");

    // save the new rate
    user_tbl.modify(user_itr, same_payer, [&](auto& row) {
        row.timeunit_rate -= removed_rate;
    });

    // send the assets back
    action(permission_level { get_self(), name("active") }, atomicassets::ATOMICASSETS_ACCOUNT, name("transfer"),
        make_tuple(get_self(), user, asset_ids, string("Unstaking Loot")))
        .send();
}

[[eosio::on_notify("atomicassets::transfer")]] void
loot::receiveassets(name from, name to, vector<uint64_t> asset_ids, string memo)
{
    // ignore outgoing transactions and transaction not destined to the dapp itself
    if (to != get_self() || from == get_self()) {
        return;
    }

    /*/ ignore transactions if the memo isn't stake
    if (memo != "stake") {
        return;
    }/*/

    // check if the contract isn't frozen
    const auto& config = check_config();

    // get users table instance
    user_t user_tbl(get_self(), get_self().value);

    const auto& user_itr = user_tbl.find(from.value);

    // check if the user is registered
    if (user_itr == user_tbl.end()) {
        check(false, string("user " + from.to_string() + " is not registered").c_str());
    }

    // get the assets table (scoped to the contract)
    const auto& aa_asset_tbl = atomicassets::get_assets(get_self());

    // get template table instance
    template_t template_tbl(get_self(), get_self().value);
    // get asset table instance
    asset_t asset_tbl(get_self(), get_self().value);

    asset added_rate = asset(0, config.token_symbol);

    for (const uint64_t& asset_id : asset_ids) {
        // find the asset data, to get the template id from it
        const auto& aa_asset_itr = aa_asset_tbl.find(asset_id);

        if (aa_asset_itr == aa_asset_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") does not exist").c_str());
        }

        // check if the asset's template is stakeable
        const auto& template_itr = template_tbl.find(aa_asset_itr->template_id);

        if (template_itr == template_tbl.end()) {
            check(false, string("asset (" + to_string(asset_id) + ") is not stakeable").c_str());
        }

        // increment the added rate
        added_rate += template_itr->timeunit_rate;

        // save the asset
        asset_tbl.emplace(get_self(), [&](asset_s& row) {
            row.asset_id = asset_id;
            row.owner = from;
            row.last_claim = time_point_sec(current_time_point());
        });
    }

    // save the new rate
    user_tbl.modify(user_itr, same_payer, [&](auto& row) {
        row.timeunit_rate += added_rate;
    });
}