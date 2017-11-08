display = require"client.display"


love.load = () ->
    display.init!
    print"load complete"


love.update = ( dt ) ->
    love.event.quit! if love.keyboard.isDown( "escape" )
    display.scroll(dt)


love.draw = () ->
    fps = love.timer.getFPS!
    love.graphics.print("fps: #{fps}", 10, 10)

    display.draw_map!
