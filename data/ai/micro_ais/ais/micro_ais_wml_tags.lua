local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require("ai/lua/ai_helper.lua")

local function add_CAs(side, CA_parms, CA_cfg)
    -- Add the candidate actions defined in 'CA_parms' to the AI of 'side'
    -- CA_parms is an array of tables, one for each CA to be added (CA setup parameters)
    -- CA_cfg is a table with the parameters passed to the eval/exec functions
    --
    -- Required keys for CA_parms:
    --  - ca_id: is used for CA id/name and the eval/exec function names
    --  - score: the evaluation score

    for i,parms in ipairs(CA_parms) do
        -- Make sure the id/name of each CA are unique.
        -- We do this by seeing if a CA by that name exists already.
        -- If not, we use the passed id in parms.ca_id
        -- If yes, we add a number to the end of parms.ca_id until we find an id that does not exist yet
        local ca_id, id_found = parms.ca_id, true

        local n = 1
        while id_found do -- This is really just a precaution
            id_found = false

            for ai_tag in H.child_range(wesnoth.sides[side].__cfg, 'ai') do
                for stage in H.child_range(ai_tag, 'stage') do
                    for ca in H.child_range(stage, 'candidate_action') do
                        if (ca.name == ca_id) then id_found = true end
                        --print('---> found CA:', ca.name, id_found)
                    end
                end
            end

            if (id_found) then ca_id = parms.ca_id .. n end
            n = n+1
        end

        -- Always pass the ca_id and ca_score to the eval/exec functions
        CA_cfg.ca_id = ca_id
        CA_cfg.ca_score = parms.score

        local CA = {
            engine = "lua",
            id = ca_id,
            name = ca_id,
            max_score = parms.score
        }

        if parms.location then  -- if using external CAs
            CA.location = parms.location
            local cfg = string.sub(AH.serialize(CA_cfg), 2, -2) -- need to strip surrounding {}
            CA.eval_parms = cfg
            CA.exec_parms = cfg
        else  -- if using "regular" CAs
            CA.evaluation = "return (...):" .. (parms.eval_id or parms.ca_id) .. "_eval(" .. AH.serialize(CA_cfg) .. ")"
            CA.execution = "(...):" .. (parms.eval_id or parms.ca_id) .. "_exec(" .. AH.serialize(CA_cfg) .. ")"
        end

        W.modify_ai {
            side = side,
            action = "add",
            path = "stage[main_loop].candidate_action",
            { "candidate_action", CA }
        }
    end
end

local function delete_CAs(side, CA_parms)
    -- Delete the candidate actions defined in 'CA_parms' from the AI of 'side'
    -- CA_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_CAs(), although only the
    -- CA_parms.ca_id field is needed

    for i,parms in ipairs(CA_parms) do
        local ca_id = parms.ca_id

        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "stage[main_loop].candidate_action[" .. ca_id .. "]"
        }
    end
end

local function add_aspects(side, aspect_parms)
    -- Add the aspects defined in 'aspect_parms' to the AI of 'side'
    -- aspect_parms is an array of tables, one for each aspect to be added
    --
    -- Required keys for aspect_parms:
    --  - aspect: the aspect name (e.g. 'attacks' or 'aggression')
    --  - facet: A table describing the facet to be added
    --
    -- Examples of facets:
    -- 1. Simple aspect, e.g. aggression
    -- { value = 0.99 }
    --
    -- 2. Composite aspect, e.g. attacks
    --  {   name = "ai_default_rca::aspect_attacks",
    --      id = "dont_attack",
    --      invalidate_on_gamestate_change = "yes",
    --      { "filter_own", {
    --          type = "Dark Sorcerer"
    --      } }
    --  }

    for i,parms in ipairs(aspect_parms) do
        W.modify_ai {
            side = side,
            action = "add",
            path = "aspect[" .. parms.aspect .. "].facet",
            { "facet", parms.facet }
        }
    end
end

local function delete_aspects(side, aspect_parms)
    -- Delete the aspects defined in 'aspect_parms' from the AI of 'side'
    -- aspect_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_aspects(), although only the
    -- aspect_parms.aspect_id field is needed

    for i,parms in ipairs(aspect_parms) do
        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "aspect[attacks].facet[" .. parms.aspect_id .. "]"
        }
    end
end

function wesnoth.wml_actions.micro_ai(cfg)
    -- Set up the [micro_ai] tag functionality for each Micro AI
    cfg = cfg.__parsed

    -- Add translation for old-syntax animal MAIs to new syntax plus deprecation message
    if (cfg.ai_type == 'animals') and (cfg.animal_type) then
        wesnoth.message("The syntax 'ai_type=animals animal_type=" .. cfg.animal_type .. "' is deprecated.  Use 'ai_type=" .. cfg.animal_type .. "' instead.")

        cfg.ai_type = cfg.animal_type
        cfg.animal_type = nil
    end

    -- Add translation for old-syntax guardian MAIs to new syntax plus deprecation message
    if (cfg.ai_type == 'guardian_unit') and (cfg.guardian_type) then
        wesnoth.message("The syntax 'ai_type=guardian_unit guardian_type=" .. cfg.guardian_type .. "' is deprecated.  Use 'ai_type=" .. cfg.guardian_type .. "' instead.")

        cfg.ai_type = cfg.guardian_type
        cfg.guardian_type = nil
    end

    -- Add translation for old-syntax hunter_unit MAI to new syntax plus deprecation message
    if (cfg.ai_type == 'hunter_unit') then
        wesnoth.message("'ai_type=hunter_unit' is deprecated.  Use 'ai_type=hunter' instead.")

        cfg.ai_type = 'hunter'
    end

    -- Add translation for old-syntax patrol_unit MAI to new syntax plus deprecation message
    if (cfg.ai_type == 'patrol_unit') then
        wesnoth.message("'ai_type=patrol_unit' is deprecated.  Use 'ai_type=patrol' instead.")

        cfg.ai_type = 'patrol'
    end

    -- Check that the required common keys are all present and set correctly
    if (not cfg.ai_type) then H.wml_error("[micro_ai] is missing required ai_type= key") end
    if (not cfg.side) then H.wml_error("[micro_ai] is missing required side= key") end
    if (not cfg.action) then H.wml_error("[micro_ai] is missing required action= key") end

    if (cfg.action ~= 'add') and (cfg.action ~= 'delete') and (cfg.action ~= 'change') then
        H.wml_error("[micro_ai] unknown value for action=. Allowed values: add, delete or change")
    end

    -- Set up the configuration tables for the different Micro AIs
    local required_keys, optional_keys, CA_parms = {}, {}, {}

    --------- Healer Support Micro AI ------------------------------------
    if (cfg.ai_type == 'healer_support') then
        optional_keys = { "aggression", "injured_units_only", "max_threats", "filter", "filter_second" }
        -- Scores for this AI need to be hard-coded, it does not work otherwise
        CA_parms = {
            { ca_id = 'mai_healer_initialize', location = 'ai/micro_ais/cas/ca_healer_initialize.lua', score = 999990 },
            { ca_id = 'mai_healer_move', location = 'ai/micro_ais/cas/ca_healer_move.lua', score = 105000 },
        }

        -- The healers_can_attack CA is only added to the table if aggression ~= 0
        -- But: make sure we always try removal
        if (cfg.action == 'delete') or (tonumber(cfg.aggression) ~= 0) then
            table.insert(CA_parms, { ca_id = 'mai_healer_may_attack', location = 'ai/micro_ais/cas/ca_healer_may_attack.lua', score = 99990 })
        end

    --------- Bottleneck Defense Micro AI -----------------------------------
    elseif (cfg.ai_type == 'bottleneck_defense') then
        required_keys = { "x", "y", "enemy_x", "enemy_y" }
        optional_keys = { "healer_x", "healer_y", "leadership_x", "leadership_y", "active_side_leader" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            { ca_id = 'mai_bottleneck_move', location = 'ai/micro_ais/cas/ca_bottleneck_move.lua', score = score },
            { ca_id = 'mai_bottleneck_attack', location = 'ai/micro_ais/cas/ca_bottleneck_attack.lua', score = score - 1 }
        }

    --------- Messenger Escort Micro AI ------------------------------------
    elseif (cfg.ai_type == 'messenger_escort') then
        required_keys = { "id", "waypoint_x", "waypoint_y" }
        optional_keys = { "enemy_death_chance", "messenger_death_chance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            { ca_id = 'mai_messenger_attack', location = 'ai/micro_ais/cas/ca_messenger_attack.lua', score = score },
            { ca_id = 'mai_messenger_move', location = 'ai/micro_ais/cas/ca_messenger_move.lua', score = score - 1 },
            { ca_id = 'mai_messenger_escort_move', location = 'ai/micro_ais/cas/ca_messenger_escort_move.lua', score = score - 2 }
        }

    --------- Lurkers Micro AI ------------------------------------
    elseif (cfg.ai_type == 'lurkers') then
        required_keys = { "filter", "filter_location" }
        optional_keys = { "stationary", "filter_location_wander" }
        CA_parms = { { ca_id = 'mai_lurkers', location = 'ai/micro_ais/cas/ca_lurkers.lua', score = cfg.ca_score or 300000 } }

    --------- Protect Unit Micro AI ------------------------------------
    elseif (cfg.ai_type == 'protect_unit') then
        required_keys = { "id", "goal_x", "goal_y" }
        -- Scores for this AI need to be hard-coded, it does not work otherwise
        CA_parms = {
            { ca_id = 'mai_protect_unit_finish',  score = 300000 },
            { ca_id = 'mai_protect_unit_attack', score = 95000 },
            { ca_id = 'mai_protect_unit_move', score = 94999 }
        }

        -- [unit] tags need to be dealt with separately
        cfg.id, cfg.goal_x, cfg.goal_y = {}, {}, {}
        if (cfg.action ~= 'delete') then
            for u in H.child_range(cfg, "unit") do
                if (not u.id) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required id= key")
                end
                if (not u.goal_x) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_x= key")
                end
                if (not u.goal_y) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_y= key")
                end
                table.insert(cfg.id, u.id)
                table.insert(cfg.goal_x, u.goal_x)
                table.insert(cfg.goal_y, u.goal_y)
            end

            if (not cfg.id[1]) then
                H.wml_error("Protect Unit Micro AI is missing required [unit] tag")
            end
        end

        -- Optional key disable_move_leader_to_keep: needs to be dealt with
        -- separately as it affects a default CA
        if cfg.disable_move_leader_to_keep then
            W.modify_ai {
                side = side,
                action = "try_delete",
                path = "stage[main_loop].candidate_action[move_leader_to_keep]"
            }
        end

        -- attacks aspects also needs to be set separately
        local unit_ids_str = 'dummy'
        for i,id in ipairs(cfg.id) do
            unit_ids_str = unit_ids_str .. ',' .. id
        end
        local aspect_parms = {
            {
                aspect = "attacks",
                facet = {
                    name = "ai_default_rca::aspect_attacks",
                    ca_id = "dont_attack",
                    invalidate_on_gamestate_change = "yes",
                    { "filter_own", {
                        { "not", {
                            id = unit_ids_str
                        } }
                    } }
                }
            }
        }

        if (cfg.action == "delete") then
            delete_aspects(cfg.side, aspect_parms)
            -- We also need to add the move_leader_to_keep CA back in
            -- This works even if it was not removed, it simply overwrites the existing CA
            W.modify_ai {
                side = side,
                action = "add",
                path = "stage[main_loop].candidate_action",
                { "candidate_action", {
                    id="move_leader_to_keep",
                    engine="cpp",
                    name="ai_default_rca::move_leader_to_keep_phase",
                    max_score=160000,
                    score=160000
                } }
            }
        else
            add_aspects(cfg.side, aspect_parms)
        end

    --------- Micro AI Guardian -----------------------------------
    elseif (cfg.ai_type == 'stationed_guardian') then
        required_keys = { "id", "distance", "station_x", "station_y", "guard_x", "guard_y" }
        CA_parms = { { ca_id = 'mai_stationed_guardian', location = 'ai/micro_ais/cas/ca_stationed_guardian.lua', score = cfg.ca_score or 300000 } }

    elseif (cfg.ai_type == 'zone_guardian') then
        required_keys = { "id", "filter_location" }
        optional_keys = { "filter_location_enemy", "station_x", "station_y" }
        CA_parms = { { ca_id = 'mai_zone_guardian', location = 'ai/micro_ais/cas/ca_zone_guardian.lua', score = cfg.ca_score or 300000 } }

    elseif (cfg.ai_type == 'return_guardian') then
        required_keys = { "id", "return_x", "return_y" }
        CA_parms = { { ca_id = 'mai_return_guardian', location = 'ai/micro_ais/cas/ca_return_guardian.lua', score = cfg.ca_score or 100010 } }

    elseif (cfg.ai_type == 'coward') then
        -- id= key is required also for CA deletion; needs to be checked separately
        if (not cfg.id) then
            H.wml_error("[micro_ai] tag (coward) is missing required parameter: id")
        end
        required_keys = { "id", "distance" }
        optional_keys = { "seek_x", "seek_y","avoid_x","avoid_y" }
        CA_parms = { { ca_id = 'mai_coward', location = 'ai/micro_ais/cas/ca_coward.lua', score = cfg.ca_score or 300000 } }

    --------- Micro AI Animals  ------------------------------------
    elseif (cfg.ai_type == 'big_animals') then
        required_keys = { "filter"}
        optional_keys = { "avoid_unit", "filter_location", "filter_location_wander" }
        CA_parms = { { ca_id = "mai_big_animals", location = 'ai/micro_ais/cas/ca_big_animals.lua', score = cfg.ca_score or 300000 } }

    elseif (cfg.ai_type == 'wolves') then
        required_keys = { "filter", "filter_second" }
        optional_keys = { "avoid_type" }
        local score = cfg.ca_score or 90000
        CA_parms = {
            { ca_id = "mai_wolves_move", location = 'ai/micro_ais/cas/ca_wolves_move.lua', score = score },
            { ca_id = "mai_wolves_wander", location = 'ai/micro_ais/cas/ca_wolves_wander.lua', score = score - 1 }
        }

       local wolves_aspects = {
            {
                aspect = "attacks",
                facet = {
                    name = "ai_default_rca::aspect_attacks",
                    id = "dont_attack",
                    invalidate_on_gamestate_change = "yes",
                    { "filter_enemy", {
                        { "not", {
                            type=cfg.avoid_type
                        } }
                    } }
                }
            }
        }
        if (cfg.action == "delete") then
            delete_aspects(cfg.side, wolves_aspects)
        else
            add_aspects(cfg.side, wolves_aspects)
        end

    elseif (cfg.ai_type == 'herding') then
        required_keys = { "filter_location", "filter", "filter_second", "herd_x", "herd_y" }
        optional_keys = { "attention_distance", "attack_distance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            { ca_id = "mai_herding_attack_close_enemy", location = 'ai/micro_ais/cas/ca_herding_attack_close_enemy.lua', score = score },
            { ca_id = "mai_herding_sheep_runs_enemy", location = 'ai/micro_ais/cas/ca_herding_sheep_runs_enemy.lua', score = score - 1 },
            { ca_id = "mai_herding_sheep_runs_dog", location = 'ai/micro_ais/cas/ca_herding_sheep_runs_dog.lua', score = score - 2 },
            { ca_id = "mai_herding_herd_sheep", location = 'ai/micro_ais/cas/ca_herding_herd_sheep.lua', score = score - 3 },
            { ca_id = "mai_herding_sheep_move", location = 'ai/micro_ais/cas/ca_herding_sheep_move.lua', score = score - 4 },
            { ca_id = "mai_herding_dog_move", location = 'ai/micro_ais/cas/ca_herding_dog_move.lua', score = score - 5 }
        }

    elseif (cfg.ai_type == 'forest_animals') then
        optional_keys = { "rabbit_type", "rabbit_number", "rabbit_enemy_distance", "rabbit_hole_img",
            "tusker_type", "tusklet_type", "deer_type", "filter_location"
        }
        local score = cfg.ca_score or 300000
        CA_parms = {
            { ca_id = "mai_forest_animals_new_rabbit", location = 'ai/micro_ais/cas/ca_forest_animals_new_rabbit.lua', score = score },
            { ca_id = "mai_forest_animals_tusker_attack", location = 'ai/micro_ais/cas/ca_forest_animals_tusker_attack.lua', score = score - 1 },
            { ca_id = "mai_forest_animals_move", location = 'ai/micro_ais/cas/ca_forest_animals_move.lua', score = score - 2 },
            { ca_id = "mai_forest_animals_tusklet_move", location = 'ai/micro_ais/cas/ca_forest_animals_tusklet_move.lua', score = score - 3 }
        }

    elseif (cfg.ai_type == 'swarm') then
        optional_keys = { "scatter_distance", "vision_distance", "enemy_distance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            { ca_id = "mai_swarm_scatter", location = 'ai/micro_ais/cas/ca_swarm_scatter.lua', score = score },
            { ca_id = "mai_swarm_move", location = 'ai/micro_ais/cas/ca_swarm_move.lua', score = score - 1 }
        }

    elseif (cfg.ai_type == 'wolves_multipacks') then
        optional_keys = { "type", "pack_size", "show_pack_number" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            { ca_id = "mai_wolves_multipacks_attack", location = 'ai/micro_ais/cas/ca_wolves_multipacks_attack.lua', score = score },
            { ca_id = "mai_wolves_multipacks_wander", location = 'ai/micro_ais/cas/ca_wolves_multipacks_wander.lua', score = score - 1 }
        }

    elseif (cfg.ai_type == 'hunter') then
        required_keys = { "id", "home_x", "home_y" }
        optional_keys = { "filter_location", "rest_turns", "show_messages" }
        CA_parms = { { ca_id = "mai_hunter", location = 'ai/micro_ais/cas/ca_hunter.lua', score = cfg.ca_score or 300000 } }

    --------- Patrol Micro AI ------------------------------------
    elseif (cfg.ai_type == 'patrol') then
        required_keys = { "id", "waypoint_x", "waypoint_y" }
        optional_keys = { "attack", "one_time_only", "out_and_back" }
        CA_parms = { { ca_id = "mai_patrol", location = 'ai/micro_ais/cas/ca_patrol.lua', score = cfg.ca_score or 300000 } }

    --------- Recruiting Micro AI ------------------------------------
    elseif (cfg.ai_type == 'recruiting') then
        if (cfg.recruiting_type == 'rushers') then
            optional_keys = { "randomness" }
            CA_parms = { { ca_id = "mai_rusher_recruit", score = cfg.ca_score or 180000 } }

        elseif (cfg.recruiting_type == 'random') then
            optional_keys = { "skip_low_gold_recruiting", "type", "prob" }
            CA_parms = { { ca_id = "mai_random_recruit", score = cfg.ca_score or 180000 } }

            -- The 'probability' tags need to be handled separately here
            cfg.type, cfg.prob = {}, {}
            for p in H.child_range(cfg, "probability") do
                if (not p.type) then
                    H.wml_error("Random Recruiting Micro AI [probability] tag is missing required type= key")
                end
                if (not p.probability) then
                    H.wml_error("Random Recruiting Micro AI [probability] tag is missing required probability= key")
                end
                table.insert(cfg.type, p.type)
                table.insert(cfg.prob, p.probability)
            end

        else
            H.wml_error("[micro_ai] tag (recruiting) recruiting_type= key is missing or has unknown value")
        end

        -- Also need to delete/add the default recruitment CA
        if cfg.action == 'add' then
            W.modify_ai {
                side = cfg.side,
                action = "try_delete",
                path = "stage[main_loop].candidate_action[recruitment]"
            }
        elseif cfg.action == 'delete' then
            -- We need to add the recruitment CA back in
            -- This works even if it was not removed, it simply overwrites the existing CA
            W.modify_ai {
                side = cfg.side,
                action = "add",
                path = "stage[main_loop].candidate_action",
                { "candidate_action", {
                    id="recruitment",
                    engine="cpp",
                    name="ai_default_rca::aspect_recruitment_phase",
                    max_score=180000,
                    score=180000
                } }
            }
        end

    --------- Goto Micro AI ------------------------------------
    elseif (cfg.ai_type == 'goto') then
        required_keys = { "filter_location" }
        optional_keys = {
            "avoid_enemies", "filter", "ignore_units", "ignore_enemy_at_goal",
            "release_all_units_at_goal", "release_unit_at_goal", "unique_goals", "use_straight_line"
        }
        CA_parms = { { ca_id = 'mai_goto', location = 'ai/micro_ais/cas/ca_goto.lua', score = cfg.ca_score or 300000 } }

    --------- Hang Out Micro AI ------------------------------------
    elseif (cfg.ai_type == 'hang_out') then
        optional_keys = { "filter", "filter_location", "avoid", "mobilize_condition", "mobilize_on_gold_less_than" }
        CA_parms = { { ca_id = 'mai_hang_out', location = 'ai/micro_ais/cas/ca_hang_out.lua', score = cfg.ca_score or 170000 } }

    --------- Simple Attack Micro AI ---------------------------
    elseif (cfg.ai_type == 'simple_attack') then
        optional_keys = { "filter", "filter_second" }
        CA_parms = { { ca_id = 'mai_simple_attack', location = 'ai/micro_ais/cas/ca_simple_attack.lua', score = cfg.ca_score or 110000 } }

    -- If we got here, none of the valid ai_types was specified
    else
        H.wml_error("unknown value for ai_type= in [micro_ai]")
    end

    --------- Now go on to setting up the CAs ---------------------------------
    -- If cfg.ca_id is set, it gets added to the ca_id= key of all CAs
    -- This allows for selective removal of CAs
    if cfg.ca_id then
        for i,parms in ipairs(CA_parms) do
            -- Need to save eval_id first though
            parms.eval_id = parms.ca_id
            parms.ca_id = parms.ca_id .. '_' .. cfg.ca_id
        end
    end

    -- If action=delete, we do that and are done, but we do need to pass
    if (cfg.action == 'delete') then
        delete_CAs(cfg.side, CA_parms)
        return
    end

    -- Otherwise, set up the cfg table to be passed to the CA eval/exec functions
    local CA_cfg = {}

    -- Required keys
    for k, v in pairs(required_keys) do
        local child = H.get_child(cfg, v)
        if (not cfg[v]) and (not child) then
            H.wml_error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: " .. v)
        end
        CA_cfg[v] = cfg[v]
        if child then CA_cfg[v] = child end
    end

    -- Optional keys
    for k, v in pairs(optional_keys) do
        CA_cfg[v] = cfg[v]
        local child = H.get_child(cfg, v)
        if child then CA_cfg[v] = child end
    end

    -- Finally, set up the candidate actions themselves
    if (cfg.action == 'add') then add_CAs(cfg.side, CA_parms, CA_cfg) end
    if (cfg.action == 'change') then
        delete_CAs(cfg.side, CA_parms, cfg.id)
        add_CAs(cfg.side, CA_parms, CA_cfg)
    end
end
