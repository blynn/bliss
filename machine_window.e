class MACHINE_WINDOW
inherit
    WINDOW;
    SONG_TABLE
creation make
feature
    make(x, y, w, h : INTEGER) is
    do
	put_geometry(x, y, w, h)
    end

    process_event(e : EVENT) is
    do
	inspect e.type
	when sdl_mousebuttondown then
	    process_buttondown(e)
	when sdl_mousebuttonup then
	    process_buttonup(e)
	when sdl_keydown then
	    process_key(e.i1)
	else
	end
    end

    process_buttondown(e : EVENT) is
    do
	inspect e.i1
	when 1 then
	    if is_kmod(e.kmod, kmod_shift) then
		select_machine_for_connect(e.x, e.y)
	    else
		select_machine_for_move(e.x, e.y)
		if selection = Void then
		    select_edge(e.x, e.y)   
		end
	    end
	else
	end
    end

    process_buttonup(e : EVENT) is
    do
	inspect e.i1
	when 1 then
	    if moving_flag then
		place_selection(e.x, e.y)
	    elseif connecting_flag then
		connect_selection(e.x, e.y)
	    end
	else
	end
    end

    select_edge(x, y : INTEGER) is
    local
	it : ITERATOR[EDGE]
	r : INTEGER
	d : INTEGER
	done : BOOLEAN
    do
	it := song.connection_list.get_new_iterator
	it.start
	if it.is_off then
	    done := True
	end
	from
	until done
	loop
	    d := it.item.x - x
	    r := d * d
	    d := it.item.y - y
	    r := d * d + r
	    if r < 10 * 10 then
		io.put_string(it.item.src.name + " --> " + it.item.dst.name + "%N")
		song.delete_edge(it.item)
		done := True
	    else
		it.next
		if it.is_off then
		    done := True
		end
	    end
	end
    end

    connecting_flag : BOOLEAN
    moving_flag : BOOLEAN
    selection : MACHINE
    movestartx : INTEGER
    movestarty : INTEGER

    machine_zorder : LINKED_LIST[MACHINE] is
    do
	Result := song.machine_zorder
    end

    select_machine(x, y : INTEGER) : INTEGER is
    local
	i : INTEGER
	m : MACHINE
    do
	from i := machine_zorder.upper
	until i < machine_zorder.lower
	loop
	    m := machine_zorder.item(i)
	    if x >= m.posx and then x <= m.posx + m.width and then
		    y >= m.posy and then y <= m.posy + m.height then
		Result := i
		i := machine_zorder.lower - 1
	    else
		i := i - 1
	    end
	end
    end

    select_machine_for_move(x, y : INTEGER) is
    local
	i : INTEGER
    do
	i := select_machine(x, y)
	if i >= machine_zorder.lower then
	    selection := machine_zorder @ i
	    machine_zorder.remove(i)
	    machine_zorder.add_last(selection)
	    moving_flag := True
	    movestartx := x - selection.posx
	    movestarty := y - selection.posy
	else
	    selection := Void
	end
    end

    select_machine_for_connect(x, y : INTEGER) is
    local
	i : INTEGER
    do
	i := select_machine(x, y)
	if i >= machine_zorder.lower then
	    selection := machine_zorder @ i
	    connecting_flag := True
	end
    end

    place_selection(x, y : INTEGER) is
    require
	moving_flag
    do
	moving_flag := False
	clipped_move(selection, x - movestartx, y - movestarty)
    end

    connect_selection(x, y : INTEGER) is
    require
	connecting_flag
    local
	i : INTEGER
	m : MACHINE
    do
	connecting_flag := False
	i := select_machine(x, y)
	if i >= machine_zorder.lower then
	    m := machine_zorder @ i
	    attempt_connect(selection, m)
	end
    end
    
    attempt_connect(source, sink : MACHINE) is
    do
	if sink.is_sink and then source.is_source then
	    --TODO: check source, sink aren't connected
	    song.connect(source, sink)
	end
    end

    clipped_move(m : MACHINE; new_x, new_y : INTEGER) is
    local
	x, y : INTEGER
    do
	x := new_x
	y := new_y
	if x < 0 then
	    x := 0
	elseif x > width - m.width then
	    x := width - m.width
	end
	if y < 0 then
	    y := 0
	elseif y > height - m.height then
	    y := height - m.height
	end
	song.move_to(m, x, y)
    end

    process_key(k : INTEGER) is
    local
	m : MACHINE
    do
	inspect k
	when sdlk_b then
	    m := song.new_machine_autoname("BLBass2")
	    song.add_track(m)
	    ext_get_mouse_state
	    clipped_move(m, last_mouse_x - m.width // 2, last_mouse_y - m.height // 2)
	when sdlk_n then
	    m := song.new_machine_autoname("BLDrum")
	    song.add_track(m)
	    ext_get_mouse_state
	    clipped_move(m, last_mouse_x - m.width // 2, last_mouse_y - m.height // 2)
	when sdlk_o then
	    m := song.new_machine_autoname("NOP")
	    song.add_track(m)
	    ext_get_mouse_state
	    clipped_move(m, last_mouse_x - m.width // 2, last_mouse_y - m.height // 2)
	else
	end
    end

    update is
    local
	it : ITERATOR[MACHINE]
	it2 : ITERATOR[EDGE]
	m : MACHINE
    do
	if moving_flag then
	    ext_get_mouse_state
	    clipped_move(selection, last_mouse_x - movestartx, last_mouse_y - movestarty)
	elseif connecting_flag then
	    ext_get_mouse_state
	    aalinecolor(selection.centerx, selection.centery, last_mouse_x, last_mouse_y, white)
	end

	--draw connections
	it2 := song.connection_list.get_new_iterator
	from it2.start
	until it2.is_off
	loop
	    draw_connection(it2.item)
	    it2.next
	end

	--draw machines
	it := machine_zorder.get_new_iterator
	from it.start
	until it.is_off
	loop
	    m := it.item
	    blit(m.image, m.posx, m.posy)
	    it.next
	end
    end

    draw_connection(e : EDGE) is
    local
	x1, y1, x2, y2 : INTEGER
	dx, dy : INTEGER
	img : IMAGE
	angle : DOUBLE
	m1, m2 : MACHINE
    do
	m1 := e.src
	m2 := e.dst
	x1 := m1.centerx
	y1 := m1.centery
	x2 := m2.centerx
	y2 := m2.centery
	aalinecolor(x1, y1, x2, y2, white)

	dy := y2 - y1
	dx := x2 - x1
	if dx = 0 then
	    if dy < 0 then
		angle := 90
	    else
		angle := -90
	    end
	else
	    angle := (dy / dx).atan
	    angle := angle * 180 / 3.14159265358979323846
	    if dx < 0 then
		angle := angle + 180
	    end
	    angle := -angle --computer screen: y increases downwards
	end
	x1 := (x1 + x2) // 2
	y1 := (y1 + y2) // 2
	e.put_xy(x1, y1)
	!!img.make
	img.rotozoom(arrow_image, angle, 1.0)
	img.set_colorkey(0)
	x2 := x1 - img.width // 2
	y2 := y1 - img.height // 2
	blit(img, x2, y2)
	img.free
    end

    arrow_image : IMAGE is
    once
	!!Result.make
	Result.new_arrow
    end
end
