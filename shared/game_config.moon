irdya_version = "0.0.1"
game_title = "Wesnoth"
game_id    = "wesnoth"

return ( config ) ->

    with config
        .identity = game_id
        .version  = "0.10.2" -- version of love2d engine
        .console  = true
        with .window
            .title      = "#{game_title} | Irdya (v#{irdya_version})"
            -- .icon       = "binary/icons/#{game_id}-icon.png"
            .msaa       = 4
            .width      = 1920
            .minwidth   = 800
            .height     = 1080
            .minheight  = 480
            .borderless = false
            .resizable  = true
            .fullscreen = false
            .vsync      = false
        with .modules
            .physics = false
            .math    = false
            .thread  = false
            .audio   = false
            -- .sound   = false
            .video   = false
