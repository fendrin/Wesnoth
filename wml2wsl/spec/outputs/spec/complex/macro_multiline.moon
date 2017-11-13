SCATTER_UNITS(20, "Elvish Fighter,Elvish Archer,Elvish Shaman", 3, {
        terrain: "Gs^Fp"
        x: "10-30"
        y: "20-40"

        not: {
            filter: {
            }
        }

        not: {
            filter_adjacent_location: {
                filter: {
                }
            }
        }
    },{
        side: 2
        generate_name: true
        random_traits: true

        modifications: {
            <TRAIT_LOYAL!
        }
})
