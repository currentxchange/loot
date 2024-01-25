



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


/*/
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
/*/

/*/
ACTION loot::addtemplates(const uint32_t& template_id, const name& collection, const asset& timeunit_rate)
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

/*/


/*/

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

/*/



    /*/ --- Update user's claimed amount in the users table --- //
    user_t users_tbl(get_self(), get_self().value);
    auto user_itr = users_tbl.find(user.value);
    check(user_itr != users_tbl.end(), "User not found in users table.");

    users_tbl.modify(user_itr, get_self(), [&](auto& row) {
        row.stacked += claimed_amount;
    });

    /*/