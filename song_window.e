class SONG_WINDOW
inherit
    WINDOW;
    SONG_TABLE
creation make
feature
    tablex : INTEGER is 30
    tabley : INTEGER is 40
    labelx : INTEGER is 0
    labely : INTEGER is 20
    cellx : INTEGER is 60
    celly : INTEGER is 20

    rowstep : INTEGER is 16

    originr, originc : INTEGER
    cursorr, cursorc : INTEGER
    maxr, maxc : INTEGER

    make(x, y, w, h : INTEGER) is
    do
	put_geometry(x, y, w, h)
	maxc := width // cellx - 1
	maxr := height // celly - 1
	originc := song.tracks.lower
    end

    process_event(e : EVENT) is
    do
	inspect e.type
	when sdl_keydown then
	    process_key(e)
	when sdl_mousebuttondown then
	    process_buttondown(e)
	else
	end
    end

    process_buttondown(e : EVENT) is
    local
	r : INTEGER
    do
	inspect e.i1
	when 1 then
	    if e.x < tablex and then e.y > tabley then
		r := rowstep * (originr + ((e.y - tabley) // celly))
		if r > song.end_beat then
		    r := song.end_beat
		end
		io.put_string("jump to " + r.to_string + "%N")
		song.jump_to(r)
	    end
	end
    end

    process_key(e : EVENT) is
    do
	if is_kmod(e.kmod, kmod_ctrl) then
	    inspect e.i1
	    when sdlk_e then
		song.put_end_beat(abs_row(cursorr));
	    else
		if not song.tracks.is_empty then
		    process_edit_key(e.i1)
		end
	    end
	else
	    if not song.tracks.is_empty then
		process_edit_key(e.i1)
	    end
	end
    end

    process_edit_key(k : INTEGER) is
    do
	inspect k
	when sdlk_right then
	    if abs_col(cursorc) < song.tracks.upper then
		if cursorc < maxc - 1 then
		    cursorc := cursorc + 1
		else
		    originc := originc + 1
		end
	    end
	when sdlk_left then
	    if abs_col(cursorc) > song.tracks.lower then
		if cursorc > 0 then
		    cursorc := cursorc - 1
		else
		    if originc > 0 then
			originc := originc - 1
		    end
		end
	    end
	when sdlk_pagedown then
	    move_cursor_down(16)
	when sdlk_pageup then
	    move_cursor_up(16)
	when sdlk_down then
	    move_cursor_down(1)
	when sdlk_up then
	    move_cursor_up(1)
	when sdlk_0 then
	    song.put("0", abs_row(cursorr), abs_col(cursorc))
	when sdlk_1 then
	    song.put("1", abs_row(cursorr), abs_col(cursorc))
	when sdlk_period then
	    if song.has(abs_row(cursorr), abs_col(cursorc)) then
		song.remove(abs_row(cursorr), abs_col(cursorc))
	    end
	else
	end
    end
    
    move_cursor_up(i : INTEGER) is
    do
	if cursorr >= i then
	    cursorr := cursorr - i
	else
	    if originr >= i then
		originr := originr - i
	    else
		originr := 0
	    end
	end
    end

    move_cursor_down(i : INTEGER) is
    do
	if cursorr < maxr - i then
	    cursorr := cursorr + i
	else
	    originr := originr + i
	end
    end

    update is
    do
	draw_song
    end

    draw_song is
    local
	r, c : INTEGER
	x : INTEGER
    do
	if not song.tracks.is_empty then
	    --draw cursor
	    fill_rect(tablex + cursorc * cellx, tabley + cursorr * celly, cellx, celly, blue)
	    --draw col labels
	    from c := 0
	    until c = maxc
	    loop
		if legal_col(abs_col(c)) then
		    draw_col_label(c, song.tracks.item(abs_col(c)).machine.name)
		end
		c := c + 1
	    end

	    --draw vertical rules
	    from c := 0
	    until c = maxc
	    loop
		x := abs_col(c)
		if legal_col(abs_col(c)) then
		    x := tablex + (c + 1) * cellx - 1
		    aalinecolor(x, labely, x, height - 1, white)
		end
		c := c + 1
	    end
	end

	--draw rules
	aalinecolor(0, tabley - 1, width - 1, tabley - 1, white)
	aalinecolor(tablex - 1, labely, tablex - 1, height - 1, white)

	--draw row labels
	from r := 0
	until r = maxr
	loop
	    draw_row_label(r, abs_row(r).to_string)
	    r := r + 1
	end

	--draw cells
	from r := 0
	until r = maxr
	loop
	    from c := 0
	    until c = maxc
	    loop
		if legal_col(abs_col(c)) then
		    if song.has(abs_row(r), abs_col(c)) then
			write(tablex + c * cellx, tabley + r * celly,
				song.at(abs_row(r), abs_col(c)))
		    end
		end
		c := c + 1
	    end
	    r := r + 1
	end

	--draw end_beat line
	r := song.end_beat * celly // rowstep - originr * celly + tabley
	if r >= tabley and then r < height then
	    aalinecolor(0, r, width - 1, r, white)
	end

	--draw song_play line
	r := song.beat_count * celly // rowstep - originr * celly + tabley
	r := r + celly * song.sample_mod_tick // (song.samples_per_tick * rowstep)
	if r >= tabley and then r < height then
	    aalinecolor(0, r, width - 1, r, white)
	end
    end

    legal_col(c : INTEGER) : BOOLEAN is
    do
	if c >= song.tracks.lower and then
		c <= song.tracks.upper then
	    Result := True
	end
    end

    draw_row_label(r : INTEGER; s : STRING) is
    do
	write(labelx, tabley + r * celly, s)
    end

    draw_col_label(c : INTEGER; s : STRING) is
    do
	write(tablex + c * cellx, labely, s)
    end

    abs_row(r : INTEGER) : INTEGER is
    do
	Result := rowstep * (originr + r)
    end

    abs_col(c : INTEGER) : INTEGER is
    do
	Result := originc + c
    end
end
