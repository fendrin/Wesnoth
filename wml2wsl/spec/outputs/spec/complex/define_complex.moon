SCATTER_UNITS = (NUMBER, TYPES, PADDING_RADIUS, FILTER, UNIT_WML) ->
    -- Scatters the given kind of units randomly on a given area on the map.
    --
    -- An example which scatters some loyal elves on forest hexes in
    -- x,y=10-30,20-40, at a minimum of three hexes apart from each other and
    -- never on top of or adjacent to any already existing units:
    --! {SCATTER_UNITS 20 "Elvish Fighter,Elvish Archer,Elvish Shaman" 3 (
    --!     terrain=Gs^Fp
    --!     x=10-30
    --!     y=20-40
    --!
    --!     [not]
    --!         [filter]
    --!         [/filter]
    --!     [/not]
    --!
    --!     [not]
    --!         [filter_adjacent_location]
    --!             [filter]
    --!             [/filter]
    --!         [/filter_adjacent_location]
    --!     [/not]
    --! ) (
    --!     side=2
    --!     generate_name=yes
    --!     random_traits=yes
    --!
    --!     [modifications]
    --!         {TRAIT_LOYAL}
    --!     [/modifications]
    --! )}
    Set_Variables{
        name: "unit_type_table"

        split: {
            list: TYPES
            key: "type"
            separator: ','
        }
    }

    VARIABLE("unit_type_table_i", 0)

    Random_Placement{
        num_items: NUMBER
        variable: "random_placement_location"
        allow_less: true
        min_distance: PADDING_RADIUS
        filter_location: {
            <FILTER
            include_borders: false
        }
        command: ->
            Unit{
                type: unit_type_table[unit_type_table_i].type
                x: random_placement_location.x, y: random_placement_location.y
                <UNIT_WML
            }
            VARIABLE("unit_type_table_i", "$(($unit_type_table_i + 1) % $unit_type_table.length)")

    }

    CLEAR_VARIABLE("unit_type_table,unit_type_table_i,random_placement_location")

