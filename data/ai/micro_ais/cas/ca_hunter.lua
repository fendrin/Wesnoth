local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local function hunter_attack_weakest_adj_enemy(ai, hunter)
    -- Attack the enemy with the fewest hitpoints adjacent to 'hunter', if there is one
    -- Returns status of the attack:
    --   'attacked': if a unit was attacked
    --   'killed': if a unit was killed
    --   'no_attack': if no unit was attacked

    -- First check that the hunter exists and has attacks left
    if (not hunter.valid) then return 'no_attack' end
    if (hunter.attacks_left == 0) then return 'no_attack' end

    local min_hp, target = 9e99, {}
    for x, y in H.adjacent_tiles(hunter.x, hunter.y) do
        local enemy = wesnoth.get_unit(x, y)
        if enemy and wesnoth.is_enemy(enemy.side, wesnoth.current.side) then
            if (enemy.hitpoints < min_hp) then
                min_hp, target = enemy.hitpoints, enemy
            end
        end
    end

    if target.id then
        --W.message { speaker = hunter.id, message = 'Attacking weakest adjacent enemy' }
        AH.checked_attack(ai, hunter, target)
        if target.valid then
            return 'attacked'
        else
            return 'killed'
        end
    end

    return 'no_attack'
end

local function get_hunter(cfg)
    local filter = cfg.filter or { id = cfg.id }
    local hunter = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return hunter
end

local ca_hunter = {}

function ca_hunter:evaluation(ai, cfg)
    if get_hunter(cfg) then return cfg.ca_score end
    return 0
end

-- cfg parameters: id, hunting_ground, home_x, home_y, rest_turns, show_messages
function ca_hunter:execution(ai, cfg)
    -- Unit with the given ID goes on a hunt, doing a random wander in area given by
    -- hunting_ground, then retreats to
    -- position given by 'home_x,home_y' for 'rest_turns' turns, or until fully healed

    local hunter = get_hunter(cfg)

    -- If hunting_status is not set for the hunter -> default behavior -> hunting
    local hunter_vars = MAIUV.get_mai_unit_variables(hunter, cfg.ai_id)
    if (not hunter_vars.hunting_status) then
        -- Hunter gets a new goal if none exist or on any move with 10% random chance
        local r = math.random(10)
        if (not hunter_vars.goal_x) or (r <= 1) then
            -- 'locs' includes border hexes, but that does not matter here
            locs = AH.get_passable_locations((cfg.filter_location or {}), hunter)
            local rand = math.random(#locs)
            --print('#locs', #locs, rand)
            hunter_vars.goal_x, hunter_vars.goal_y = locs[rand][1], locs[rand][2]
            MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
        end
        --print('Hunter goto: ', hunter_vars.goal_x, hunter_vars.goal_y, r)

        -- Hexes the hunter can reach
        local reach_map = AH.get_reachable_unocc(hunter)

        -- Now find the one of these hexes that is closest to the goal
        local max_rating, best_hex = -9e99, {}
        reach_map:iter( function(x, y, v)
            -- Distance from goal is first rating
            local rating = - H.distance_between(x, y, hunter_vars.goal_x, hunter_vars.goal_y)

            -- Proximity to an enemy unit is a plus
            local enemy_hp = 500
            for xa, ya in H.adjacent_tiles(x, y) do
                local enemy = wesnoth.get_unit(xa, ya)
                if enemy and wesnoth.is_enemy(enemy.side, wesnoth.current.side) then
                    if (enemy.hitpoints < enemy_hp) then enemy_hp = enemy.hitpoints end
                end
            end
            rating = rating + 500 - enemy_hp  -- prefer attack on weakest enemy

            reach_map:insert(x, y, rating)
            if (rating > max_rating) then
                max_rating, best_hex = rating, { x, y }
            end
        end)
        --print('  best_hex: ', best_hex[1], best_hex[2])
        --AH.put_labels(reach_map)

        if (best_hex[1] ~= hunter.x) or (best_hex[2] ~= hunter.y) then
            AH.checked_move(ai, hunter, best_hex[1], best_hex[2])  -- partial move only
            if (not hunter) or (not hunter.valid) then return end
        else  -- If hunter did not move, we need to stop it (also delete the goal)
            AH.checked_stopunit_moves(ai, hunter)
            if (not hunter) or (not hunter.valid) then return end
            hunter_vars.goal_x, hunter_vars.goal_y = nil, nil
            MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
        end

        -- Or if this gets the hunter to the goal, we also delete the goal
        if (hunter.x == hunter_vars.goal_x) and (hunter.y == hunter_vars.goal_y) then
            hunter_vars.goal_x, hunter_vars.goal_y = nil, nil
            MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
        end

        -- Finally, if the hunter ended up next to enemies, attack the weakest of those
        local attack_status = hunter_attack_weakest_adj_enemy(ai, hunter)

        -- If the enemy was killed, hunter returns home
        if hunter.valid and (attack_status == 'killed') then
            hunter_vars.goal_x, hunter_vars.goal_y = nil, nil
            hunter_vars.hunting_status = 'return'
            MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
            if cfg.show_messages then
                W.message { speaker = hunter.id, message = 'Now that I have eaten, I will go back home.' }
            end
        end

        -- At this point, issue a 'return', so that no other action takes place this turn
        return
    end

    -- If we got here, this means the hunter is either returning, or resting
    if (hunter_vars.hunting_status == 'return') then
        goto_x, goto_y = wesnoth.find_vacant_tile(cfg.home_x, cfg.home_y)
        --print('Go home:', home_x, home_y, goto_x, goto_y)

        local next_hop = AH.next_hop(hunter, goto_x, goto_y)
        if next_hop then
            --print(next_hop[1], next_hop[2])
            AH.movefull_stopunit(ai, hunter, next_hop)
            if (not hunter) or (not hunter.valid) then return end

            -- If there's an enemy on the 'home' hex and we got right next to it, attack that enemy
            if (H.distance_between(cfg.home_x, cfg.home_y, next_hop[1], next_hop[2]) == 1) then
                local enemy = wesnoth.get_unit(cfg.home_x, cfg.home_y)
                if enemy and wesnoth.is_enemy(enemy.side, hunter.side) then
                    if cfg.show_messages then
                        W.message { speaker = hunter.id, message = 'Get out of my home!' }
                    end
                    AH.checked_attack(ai, hunter, enemy)
                    if (not hunter) or (not hunter.valid) then return end
                end
            end
        end

        -- We also attack the weakest adjacent enemy, if still possible
        hunter_attack_weakest_adj_enemy(ai, hunter)
        if (not hunter) or (not hunter.valid) then return end

        -- If the hunter got home, start the resting counter
        if (hunter.x == cfg.home_x) and (hunter.y == cfg.home_y) then
            hunter_vars.hunting_status = 'resting'
            hunter_vars.resting_until = wesnoth.current.turn + (cfg.rest_turns or 3)
            MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
            if cfg.show_messages then
                W.message { speaker = hunter.id, message = 'I made it home - resting now until the end of Turn ' .. hunter_vars.resting_until .. ' or until fully healed.' }
            end
        end

        -- At this point, issue a 'return', so that no other action takes place this turn
        return
    end

    -- If we got here, the only remaining action is resting
    if (hunter_vars.hunting_status == 'resting') then
        -- So all we need to do is take moves away from the hunter
        AH.checked_stopunit_moves(ai, hunter)
        if (not hunter) or (not hunter.valid) then return end

        -- However, we do also attack the weakest adjacent enemy, if still possible
        hunter_attack_weakest_adj_enemy(ai, hunter)
        if (not hunter) or (not hunter.valid) then return end

        -- If this is the last turn of resting, we also remove the status and turn variable
        if (hunter.hitpoints >= hunter.max_hitpoints) and (hunter_vars.resting_until <= wesnoth.current.turn) then
            hunter_vars.hunting_status = nil
            hunter_vars.resting_until = nil
            MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
            if cfg.show_messages then
                W.message { speaker = hunter.id, message = 'I am done resting. It is time to go hunting again next turn.' }
            end
        end
        return
    end

    -- In principle we should never get here, but just in case: reset variable, so that hunter goes hunting on next turn
    hunter_vars.hunting_status = nil
    MAIUV.set_mai_unit_variables(hunter, cfg.ai_id, hunter_vars)
end

return ca_hunter
