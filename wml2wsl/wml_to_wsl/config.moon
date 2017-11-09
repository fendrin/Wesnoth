INDENT = 4
indent_space = ""
for i = 1, INDENT
    indent_space ..= " "

actionWrappers = {
    "command"
    "define", "do"
    "else"
    "ifDef"
    "then"
    "toplevel"
}
actionTags = {
    "allow_recruit", "allow_extra_recruit", "allow_end_turn", "allow_undo", "animate_unit"
    "break", "binary_path"
    "chat", "clear_variable", "capture_village", "clear_menu_item", "color_adjust"
    "delay", "disallow_recruit", "disallow_extra_recruit", "do_command", "disallow_end_turn"
    "end_turn", "endlevel", "effect"
    "fire_event", "for", "find_path"
    "gold"
    "harm_unit", "heal_unit", "hide_unit"
    "if", "insert_tag", "inspect", "item"
    --j
    "kill"
    "lua", "lift_fog", "lock_view"
    "modify_ai", "modify_side", "modify_turns", "move_unit_fake", "music", "modify_unit", "message", "my_lua_action", "move_unit"
    --n
    "objectives", "object", "on_undo", "on_redo", "open_help"
    "petrify", "place_shroud", "put_to_recall_list"
    --q
    "redraw", "remove_shroud", "recall", "role", "random_placement", "replace_schedule", "return", "recruit", "remove_event", "remove_object", "remove_shroud", "reset_fog", "remove_time_area", "replace_map", "remove_item"
    "set_variable", "set_variables", "switch", "store_locations", "store_side", "store_unit", "set_menu_item", "select_unit", "sound_source", "store_gold", "store_reachable_locations", "store_map_dimensions", "store_starting_location", "store_time_of_day", "store_turns", "store_unit_type", "store_unit_type_ids", "store_villages", "store_items", "store_relative_direction", "set_recruit", "set_extra_recruit", "sound", "scroll_to_unit", "show_objectives"
    "test_key", "test_key2", "test_key3", "test_key4", "test_lead_space", "textdomain", "test_condition", "teleport", "time_area", "terrain_mask", "terrain", "transform_unit", "tunnel"
    "umc", "unit", "unstore_unit", "unit_worth", "unpetrify", "unhide_unit", "unit_overlay"
    "variables", "volume"
    "while", "wml_message"
    --x
    --y
    "zoom"
}
ambigTags = {
    "event"
    "unit"
    "command"
    "story"
}
toplevelTags = {
    "binary_path"
    "campaign", "core"
    "era"
    "fonts"
    "game_config"
    "help"
    "language", "locale"
    "multiplayer"
    "scenario"
    "terrain_type", "terrain_graphics", "test", "textdomain", "theme", "tutorial"
    "units"
}
actionMacros = {
    "ASSERT", "ASSERT_YES_9_5", "ASSERT_NO_9_5", "AI_ASPECT_UNIT_TEST", "AI_UNIT_TEST", "ALICE_MOVES_TO_BOB", "ALICE_1_HP", "ASPECT_NOTIFICATION", "ADD_SYMBOL", "ANSWER", "ACTIVATE_CALADAN"
    "BEAST", "BOB_1_HP", "BOTH_0_DEF", "BOTH_MAX_XP"
    "CHAT_IF"
    --D
    --E
    "FAIL", "FOR_LOOP_ARRAY_TEST", "FOR_LOOP_TEST_STEP"
    "GENERIC_UNIT_TEST"
    --H
    --I
    --J
    "KILL_SIDE"
    --L
    "MESSAGE", "MINIMUM", "MOVE_MALFORMED_SCEN", "MOVE_SKIP_SIGHTED_SCEN", "MP_UNIT_TEST"
    --N
    --O
    "PROMOTE_ADVISOR"
    --Q
    "RANDOM", "RECALL_ADVISOR", "RECALL_MAGE", "RETURN", "ROTATE_HEX_WITH_TERRAIN", "RECRUIT", "RECRUIT_TEST", "RING_OF_STRENGTH_APPEARS", "REMOVE_KEEP", "REPEAT"
    "SCATTER_UNITS", "SUCCEED", "SIMPLE", "SUCCEED", "SPLIT", "STR_FEEDING", "STR_FEEDING_EFFECT", "STR_FEEDING_DESCRIPTION", "STORM_TRIDENT", "FLAMING_SWORD", "ANKH_NECKLACE", "SPECIAL_NOTES_STUN", "SUMMON_FIRE_GUARDIAN"
    "TEST_HAS_ALLY_SCEN", "TEST_TRANSACTION", "TEST_TRANSACTION_PARAM", "TEST_FILTER_VISION_SCEN", "TEST_CHECK_VICTORY_SCEN", "TEST_KILL_SIDES_TWO_THREE", "TURN_UNIT", "TEST_SIGHTED_EVENTS_SCEN", "TEST_GRUNT_DAMAGE", "TEST_FEEDING", "TEST_BREAK_REPLAY", "TEST_VISION_CHAT"
    "UNIT"
    "VARIABLE"
    --W
    --X
    --Y
    --Z
}
actionTagMap = {}
for tag in *actionTags
    actionTagMap[tag] = true
ambigMap = {}
for tag in *ambigTags
    ambigMap[tag] = true
actionMacroMap = {}
for tag in *actionMacros
    actionMacroMap[tag] = true
toplevelMap = {}
for tag in *toplevelTags
    toplevelMap[tag] = true
actionWrappersMap = {}
for tag in *actionWrappers
    actionWrappersMap[tag] = true
ambigTagMap = {}
for tag in *ambigTags
    ambigTagMap[tag] = true
{
    :actionTagMap
    :actionWrappersMap
    :ambigTagMap
    :actionMacroMap
    :toplevelMap
    :indent_space
    :INDENT
}
