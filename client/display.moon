-- magic numbers
font_size = 12
scroll_speed = 900
tile_w = 52
tile_h = 72
BORDER_SIZE = 1


map_x = 0
map_y = 0

local map_w, map_h
local map_display_w, map_display_h

map_data="
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Ww, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Wo, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ds, Ds, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Gs, Gs, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ds, Gs, Ds, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Gs, Gs, Gs, Ww, Ww, Ww, Wo, Ww, Ww, Ww, Wo, Ww, Ds, Ds, Ww, Gs, Gs, Ds, Ds, Ww, Ww, Ds, Ds, Ds, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ds, Gs^Vh, Gs, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ds, Gs, Ww, Gs, Rd, Rd, Ds, Ds, Ds, Ds, Gs, Ds, Ds, Ds, Ww, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ds, Gs, Gs, Ww, Ww, Ww, Ww, Gg, Gg, Ww, Ds, Gs, Gs^Vht, Ww, Gg, Rd, Gs, Gs, Ds, Gs, Gs, Gs^Es, Gs, Ds, Ds, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ww, Ds, Rd, Gs, Ds, Ww, Ds, Gs, Gs, Gg, Ww, Ds, Gs, Ww, Ww, Gg, Gg, Gs^Vht, Gs, Rd, Rd, Rd, Rd, Gs, Gs, Ds, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Gs, Rd, Gs, Gs, Ds, Gs, Gs^Fds, Gs, Gg, Ww, Gs, Gs, Ww, Gg, Gg, Gs, Rd, Rd, Rd, Gs^Es, Gs, Re^Fds, Gs^Fds, Gs^Fds, Ds, Ww^Bw/, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Gs, Gs, Gs, Gs^Vht, Rd, Gs, Gs^Fds, Gs^Fds, Gg, Ww, Gs, Ww, Ww, Gg, Gg, Gs, Rd, Gs, Gs, Gs, Ce, Ce, Ce, Rr, Rr, Ds, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Gs^Vh, Gs, Gs, Gs, Rd, Rd, Gs, Gs, Gg^Vh, Ww, Gs, Ww, Gg, Gs, Gs, Gs, Rr, Gs, Re^Vhc, Gs, Ce, 1 Ke, Ce, Re^Fds, Gs, Ds, Ds, Ww, Ww, Wo, Wo
Wo, Wo, Ww, Ww, Ww, Ds, Gs, Gs^Fds, Gs^Fds, Rd, Gs, Rd^Es, Gs, Gg, Ww, Ww, Ww, Gg, Gg, Gs^Es, Rr, Rr, Rr, Rr, Rr, Ce, Ce, Ce, Re^Fds, Gs^Fds, Ds, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ds, Gs, Gs, Gs, Gs, Gs^Vht, Gs, Gg, Gg, Gg, Ww, Ww, Gs, Gs^Vht, Rr, Gs, Gs, Gs^Fds, Gs, Re^Fds, Re^Fds, Re^Fds, Re^Vhc, Gs^Fds, Gs^Fds, Ds, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ww, Ds, Gs, Rd, Gs^Es, Gs, Gs^Fds, Gs, Rd, Gs^Fds, Gs^Fds, Ww, Gs, Gs, Gs, Gs, Gs, Gs^Vh, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Ds, Ds, Ww, Ww, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ds, Gs^Vht, Rd, Gs^Fds, Gs, Gs, Gs, Gs^Fds, Gs, Rr, Rd, Ww, Gs, Hh, Hh, Hh, Gs, Gs^Fds, Gs^Fds, Gs^Fds, Gs, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gg, Ds, Ww, Ww, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Gs, Gs, Gs, Gs, Gs^Fds, Rd, Rd, Rd, Rr, Gs^Vht, Gs, Ww, Ww, Mm, Mm, Mm, Hh^Fds, Hh^Fds, Gs^Fds, Gs^Fds, Gs^Vh, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gg, Ds, Ds, Ww, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Gs, Gs, Gs, Gs^Fds, Gs, Rd, Gs, Gs, Gs, Gs, Mm, Mm, Ww, Mm, Mm, Mm, Mm, Hh^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gs^Fds, Gs, Gg, Gg, Ds, Ds, Ww, Ww, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Gs^Vh, Gs, Gs, Gs, Gs, Rr, Gg^Vh, Hh, Hh, Mm, Mm, Mm, Ww, Ww, Mm, Mm, Mm, Hh, Gs^Fds, Gg^Vh, Gs^Fds, Gs^Fds, Gs^Vht, Gg, Gg, Ds, Ds, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ds, Ds, Gs, Gs, Hh, Hh, Rr, Hh, Hh, Hh, Mm, Mm, Ww, Ww, Ww, Ww, Mm, Mm, Gg, Gg, Gg, Gs, Gg, Gg, Gg, Gg, Ds, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ds, Gs, Rd, Rd, Gs, Gs, Rd, Hh, Hh, Hh, Mm, Mm, Ww, Ww, Ww, Ww, Ww, Mm, Mm, Hh, Gg, Gg, Rr, Gg, Gg, Gs^Fds, Gg, Ds, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ds, Ds, Gs, Gs, Gs^Es, Gs, Hh, Hh, Hh, Mm, Mm, Ww, Ww, Ww, Ww, Ww, Mm, Mm, Hh, Hh, Gg, Rr, Gg, Gg, Gs^Fds, Gs^Fds, Ds, Ww, Ww, Wo, Wo
Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ds, Gs^Vh, Gs, Rr, Gs, Hh, Hh, Hh, Mm, Mm, Ww, Ww, Ww, Ww, Mm, Mm, Mm, Hh, Gg, Gg, Rr, Rr, Gg, Gs^Fds, Gg^Vh, Ds, Ww, Wo, Wo
Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ds, Ds, Gs, Rr, Gs^Vht, Gs, Hh, Hh, Mm, Mm, Ww, Ww, Ww, Mm, Mm, Mm, Hh, Gg, Gg, Rr, Gg, Rr, Rr, Gg, Gg^Vht, Ds, Ww, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ds, Gs, Rr, Rr, Gs, Hh, Mm, Mm, Ww, Ww, Ww, Mm, Mm, Gg^Vh, Gg, Gg^Es, Rr, Gg, Rr, Gg, Rr, Rr, Ds, Ds, Ww, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ds, Gs, Gs, Rd, Gs, Hh, Hh, Mm, Mm, Mm, Mm, Mm, Mm, Gg, Gg, Gg, Gg, Gg^Vht, Gg, Gg, Rr, Ds, Ww, Ww, Ww, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ds, Gs, Gs, Rr, Gs, Gs, Hh, Hh, Mm, Hh^Vhh, Mm, Mm, Mm, Gs, Gg, Gg, Gg, Rd, Gg, Gg, Ds, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ds, Gs, Rr, Gs, Gs, Gs, Hh, Hh, Mm, Mm, Mm, Hh, Gs, Gg, Gs, Gs, Gs, Rd, Rd, Ds, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Ds, Gs^Vht, Gs, Gs, Gs, Gs^Vh, Gs, Gs, Hh, Hh, Hh, Gs, Gg, Gg, Gs, Gs, Gg, Gs, Gg, Ds, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Ww, Ww, Ds, Ds, Ww, Ds, Gs, Gs, Rd, Gs, Gs, Gs, Gs, Gs, Gs^Es, Rd, Rd, Rd, Rd, Rd, Rd, Gg^Es, Rd, Gg, Ds, Ww, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ds, Gs^Es, Gd, Gs, Ds, Ds, Gs, Gs, Rd, Gs, Gs, Gs, Gs, Gs, Gs, Gs, Gs, Gd^Fds, Gs, Gs, Gs, Rd, Gs, Gg, Gg, Ds, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Gd, Re, Re^Es, Re, Gd, Gs, Rr, Rd, Gs, Gs, Rd, Gs, Gs, Gd^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gs^Vht, Gs, Rd, Gg, Gg^Vh, Gg, Gg, Ds, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Ch, Ch, Ch, Rr^Vhc, Gs^Es, Re, Gs, Rd, Gs^Es, Gs, Rd, Gs, Gll^Fds, Gd^Fds, Gd^Fds, Gd^Fds, Gll^Fds, Gd^Fds, Rd, Rd, Rd, Rd, Rd, Gg, Ds, Ww, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Ch, 2 Kh, Ch, Rr, Re, Re, Rr^Vhc, Gs^Es, Rd, Rd, Rd, Gs, Gll^Fds, Gll^Fds, Gs^Vht, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gd^Fds, Rd^Es, Rd, Gs, Gg, Ds, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Ww, Ww, Ds, Ch, Ch, Ch, Re, Re, Gs^Es, Gs, Re, Gs, Ww, Ww, Gs^Vh, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Rd, Gd^Fds, Rd, Rd, Gs, Gs, Ds, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Ww, Gd, Gd^Es, Gd^Vhc, Re, Re, Rr^Vhc, Re, Re, Rr, Re, Gs, Gg, Ww, Gs, Gd^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gll^Fds, Gd^Fds, Rd, Rd^Es, Rd, Rd, Rd, Gs, Gs, Ds, Ww, Ww, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Gd, Re, Gs, Re, Gs^Es, Gd, Gs, Re, Gs, Gs, Gg, Ww, Gs, Gll^Fds, Gll^Fds, Gll^Fds, Gd^Fds, Ds, Ds, Ds, Rd, Rd, Gs, Gs, Gs, Ds, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Ww, Ww, Ds, Ds, Ww, Gs, Gs, Gs^Vhc, Gs, Gg, Gg, Gg, Ww, Gs, Gd^Fds, Gll^Fds, Gd^Fds, Ds, Ds, Ww, Ds, Gs, Rd, Gs, Rd, Gs, Ww, Ww, Ww, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Ds, Gg, Ds, Ds, Ds, Ww, Ww, Gs, Gd^Fds, Ds, Ww, Ww, Ww, Ww, Ww, Gs, Gs, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Ds, Ds, Ww, Ds, Ww, Ww, Ww, Gs, Ww, Ww, Ww, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Wo, Ww, Ww, Ww, Ww, Ww, Wo, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Ww, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Wo, Wo, Wo, Ww, Ww, Ww, Wo, Wo, Wo, Wo, Wo, Wo, Ww, Ww, Wo, Ww, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo
Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo, Wo"

parse_map_string = (map_string, border_size=BORDER_SIZE) ->
    assert(map_string)
    map = {}
    y = 1 - border_size
    local x
    for line in string.gmatch(map_string, "[^\r\n]+")
        x = 1 - border_size
        for terrain_string in string.gmatch(line, "([^,]+)")
            if map[x] == nil
                map[x] = {}
            map[x][y] = terrain_string
            x += 1
        y += 1
    with map
        .width  = x - 1 - border_size
        .height = y - 1 - border_size
        .border_size = border_size
    return map

-- map variables
map = parse_map_string(map_data)
map_w = #map[1] -- Obtains the width of the first row of the map
map_h = #map -- Obtains the height of the map


init = () ->
    love.graphics.setFont(love.graphics.newFont(font_size))
    screen_width, screen_height = love.graphics.getDimensions!
    map_display_w = screen_width  / tile_w
    map_display_h = screen_height / tile_h


draw_map = () ->

    screen_width, screen_height = love.graphics.getDimensions!
    map_display_w = screen_width  / tile_w
    map_display_h = screen_height / tile_h

    map_display_buffer = 2 -- We have to buffer one tile before and behind our viewpoint.
                           -- Otherwise, the tiles will just pop into view, and we don't want that.
	offset_x = map_x % tile_w
	offset_y = map_y % tile_h
	firstTile_x = math.floor(map_x / tile_w)
	firstTile_y = math.floor(map_y / tile_h)

    for y=1, (map_display_h + map_display_buffer)
		for x=1, (map_display_w + map_display_buffer)
			-- Note that this condition block allows us to go beyond the edge of the map.
			if y+firstTile_y >= 1 and y+firstTile_y <= map_h and
                x+firstTile_x >= 1 and x+firstTile_x <= map_w
                    x_coordinate = x+firstTile_x
                    y_coordinate = y+firstTile_y
                    x_pos = ((x-1)*tile_w) - offset_x - tile_w/2
                    y_pos = ((y-1)*tile_h) - offset_y - tile_h/2
                    y_pos += tile_h/2 if (x+firstTile_x) % 2 == 0
                    with love.graphics
                    -- love.graphics.draw(tile[map[y_coordinate][x_coordinate]], x_pos, y_pos)
                        .printf(map[y_coordinate][x_coordinate], x_pos, y_pos, tile_w, "center")
                        .printf("#{x_coordinate} / #{y_coordinate}",
                            x_pos + tile_w/4, y_pos + tile_h - font_size, tile_w, "center")


scroll = (dt) ->
    speed = scroll_speed * dt

    key_map =
        scroll_up:   "w"
        scroll_down: "s"
        scroll_east: "a"
        scroll_west: "d"
        quit_game:   "escape"

	-- get input
    with love.keyboard
        map_y = map_y - speed if .isDown(key_map.scroll_up)
        map_y = map_y + speed if .isDown(key_map.scroll_down)
        map_x = map_x - speed if .isDown(key_map.scroll_east)
        map_x = map_x + speed if .isDown(key_map.scroll_west)

	-- check boundaries. remove this section if you don't wish to be constrained to the map.
    map_x = 0 if map_x < 0
    map_y = 0 if map_y < 0

    if map_x > map_w * tile_w - map_display_w * tile_w - 1
        map_x = map_w * tile_w - map_display_w * tile_w - 1

    if map_y > map_h * tile_h - map_display_h * tile_h - 1
        map_y = map_h * tile_h - map_display_h * tile_h - 1


{
    :scroll
    :draw_map
    :init
}
