class IMAGE
creation make
feature
    make is
    do
    end

    width : INTEGER is
    require
	is_connected
    do
	Result := get_img_width(ptr)
    end

    height : INTEGER is
    require
	is_connected
    do
	Result := get_img_height(ptr)
    end

    is_connected : BOOLEAN

    --load(filename : STRING) is
    --require
	--not is_connected --to avoid memory leak
    --do
	--ptr := load_image(filename.to_external)
	--if ptr.is_not_null then
	    --is_connected := True
	--end
    --end

    new(w, h : INTEGER) is
    require
	not is_connected --to avoid memory leak
    do
	ptr := ext_new_surface(w, h)
	if ptr.is_not_null then
	    is_connected := True
	end
    end

    free is
    do
	if ptr.is_not_null then
	    ptr := free_img(ptr)
	    is_connected := False
	end
    end

    partial_blit(rx, ry, rw, rh, x, y : INTEGER) is
    require
	is_connected
    do
	ext_partial_blit_img(ptr, rx, ry, rw, rh, x, y)
    end

    blit(x, y : INTEGER) is
    require
	is_connected
	ptr.is_not_null
    do
	ext_blit_img(ptr, x, y)
    end

    blit_to(img : IMAGE; x, y : INTEGER) is
    require
	is_connected
	img.is_connected
	ptr.is_not_null
    do
	ext_blit_img_to(ptr, img.ptr, x, y)
    end

    --render_string(s : STRING; font : TTF_FONT) is
    render_string(s : STRING) is
    require
	not is_connected --otherwise it's a memory leak
	--font.is_connected
    do
	--ptr := render_text(s.to_external, font.to_external)
	ptr := render_text(s.to_external)
	if ptr.is_not_null then
	    is_connected := True
	end
    end

    draw_text(x, y : INTEGER; s : STRING) is
    require
	is_connected
	--font.is_connected
    local
	text : IMAGE
    do
	!!text.make
	text.render_string(s)
	text.blit_to(Current, x, y)
	text.free
    end

    set_alpha(a : INTEGER) is
    require
	is_connected
    do
	img_set_alpha(ptr, a)
    end

    box(x, y, w, h : INTEGER; c : COLOR) is
    do
	ext_box(ptr, x, y, w, h, c.to_gfx_integer)
    end

    rotozoom(img : IMAGE; angle, zoom : DOUBLE) is
    do
	ptr := ext_rotozoom(img.ptr, angle, zoom, 1)
	if ptr.is_not_null then
	    is_connected := True
	end
    end

    new_arrow is
    do
	ptr := draw_arrow_kludge
	if ptr.is_not_null then
	    is_connected := True
	end
    end

    ptr : POINTER

feature {NONE}

    --load_image(filename : POINTER) : POINTER is
    --external "C"
    --end

    free_img(i : POINTER) : POINTER is
    external "C"
    end

    --render_text(string : POINTER; font : POINTER) : POINTER is
    render_text(string : POINTER) : POINTER is
    external "C"
    end

    get_img_width(i : POINTER) : INTEGER is
    external "C"
    end

    get_img_height(i : POINTER) : INTEGER is
    external "C"
    end

    img_set_alpha(i : POINTER; a : INTEGER) is
    external "C"
    end

    ext_partial_blit_img(i : POINTER; rx, ry, rw, rh, x, y : INTEGER) is
    external "C"
    end

    ext_blit_img(i : POINTER; x, y : INTEGER) is
    external "C"
    end

    ext_blit_img_to(src, dst : POINTER; x, y : INTEGER) is
    external "C"
    end

    ext_box(p : POINTER; x, y, x2, y2, c : INTEGER) is
    external "C"
    end

    ext_new_surface(w, h : INTEGER) : POINTER is
    external "C"
    end

    draw_arrow_kludge : POINTER is
    external "C"
    end

    ext_rotozoom(img : POINTER; angle, zoom : DOUBLE; smoothing : INTEGER) : POINTER is
    external "C" alias "rotozoomSurface"
    end
end
