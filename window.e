deferred class WINDOW
inherit
    SDL_CONSTANT;
    COLOR_TABLE
feature
    offsetx : INTEGER
    offsety : INTEGER
    width : INTEGER
    height : INTEGER

    put_geometry(x, y, w, h : INTEGER) is
    do
	offsetx := x
	offsety := y
	width := w
	height := h
    end

    handle_event(e : EVENT) is
    do
	e.put_x(e.x - offsetx)
	e.put_y(e.y - offsety)
	process_event(e)
    end

    process_event(e : EVENT) is
    deferred
    end

    update is
    deferred
    end

    write(x, y : INTEGER; s : STRING) is
    local
	img : IMAGE
    do
	!!img.make
	img.render_string(s)
	img.blit(offsetx + x, offsety + y)
	img.free
    end

    fill_rect(x, y, w, h : INTEGER; c : COLOR) is
    do
	ext_fill_rect(x + offsetx, y + offsety, w, h, c.to_integer)
    end

    ext_fill_rect(x, y, w, h, c : INTEGER) is
    external "C"
    end

    blit(img : IMAGE; x, y : INTEGER) is
    do
	img.blit(x + offsetx, y + offsety)
    end

    ext_blit_surface(p : POINTER; x, y : INTEGER) is
    external "C"
    end

    ext_get_mouse_state is
    external "C"
    end

    last_mouse_x : INTEGER is
    do
	Result := get_last_mouse_x - offsetx
    end

    last_mouse_y : INTEGER is
    do
	Result := get_last_mouse_y - offsety
    end

    get_last_mouse_x : INTEGER is
    external "C"
    end

    get_last_mouse_y : INTEGER is
    external "C"
    end

    aalinecolor(x1, y1, x2, y2 : INTEGER; c : COLOR) is
    do
	ext_aalinecolor(offsetx + x1, offsety + y1, offsetx + x2, offsety + y2, c.to_gfx_integer)
    end

    ext_aalinecolor(x1, y1, x2, y2, c : INTEGER) is
    external "C"
    end

    filledcirclecolor(x1, y1, r : INTEGER; c : COLOR) is
    do
	ext_filledcirclecolor(offsetx + x1, offsety + y1, r, c.to_gfx_integer)
    end

    ext_filledcirclecolor(x, y, r, c : INTEGER) is
    external "C"
    end
end
