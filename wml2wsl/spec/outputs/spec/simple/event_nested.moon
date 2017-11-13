Test{
    event: {
        name: "start"
        do: ->
            Event{
                name: "test"
                do: ->
                    VARIABLE("pass_test", 1)
            }
    }
}
