class PATTERN_WINDOW
inherit
    WINDOW;
    SONG_TABLE
creation make
feature
    tablex : INTEGER is 30
    tabley : INTEGER is 40
    labelx : INTEGER is 0
    labely : INTEGER is 20
    cellx : INTEGER is 50
    celly : INTEGER is 20

    originr, originc : INTEGER
    cursorr, cursorc : INTEGER
    maxr, maxc : INTEGER

    machine : MACHINE
    put_machine(m : MACHINE) is
    do
	machine := m
	if machine.pattern_table.is_empty then
	    pattern := Void
	else
	    pattern := machine.pattern_list.first
	end
    end

    pattern : PATTERN

    edit(p : PATTERN) is
    require
	p /= Void
    do
	pattern := p
	machine := p.machine
    end

    make(x, y, w, h : INTEGER) is
    do
	put_geometry(x, y, w, h)
	maxc := width // cellx - 1
	maxr := height // celly - 1
    end

    process_event(e : EVENT) is
    do
	if e.type = sdl_keydown then
	    process_key(e)
	end
    end

    process_key(e : EVENT) is
    do
	if is_kmod(e.kmod, kmod_ctrl) then
	    inspect e.i1
	    when sdlk_m then
		put_machine(song.next_machine(machine))
	    when sdlk_p then
		if pattern /= Void then
		    pattern := machine.next_pattern(pattern)
		end
	    when sdlk_0 then
		pattern := machine.setup_pattern
	    when sdlk_return then
		pattern := machine.new_pattern_autoname
	    else
	    end
	else
	    if pattern /= Void then
		process_edit_key(e.i1)
	    end
	end
    end

    process_edit_key(k : INTEGER) is
    require
	pattern /= Void
    do
	inspect k
	when sdlk_right then
	    if cursorc < maxc - 1 then
		cursorc := cursorc + 1
	    else
		originc := originc + 1
	    end
	when sdlk_left then
	    if cursorc > 0 then
		cursorc := cursorc - 1
	    else
		if originc > 0 then
		    originc := originc - 1
		end
	    end
	when sdlk_down then
	    move_cursor_down(1)
	when sdlk_up then
	    move_cursor_up(1)
	else
	    process_text_edit_key(k)
	end
    end

    process_text_edit_key(k : INTEGER) is
    do
	if k >= sdlk_0 and then k <= sdlk_9 then
	    text_append(k.to_character)
	elseif k >= sdlk_a and then k <= sdlk_z then
	    text_append((k - 32).to_character)
	elseif k = sdlk_backspace then
	    text_remove_last
	end
    end

    text_append(c : CHARACTER) is
    local
	s : STRING
    do
	if pattern.has(cursorr + originr, cursorc + originc) then
	    s := pattern.at(cursorr + originr, cursorc + originc)
	else
	    !!s.make(10)
	end
	s.add_last(c)
	pattern.put(s, cursorr + originr, cursorc + originc)
    end

    text_remove_last is
    local
	s : STRING
    do
	if pattern.has(cursorr + originr, cursorc + originc) then
	    s := pattern.at(cursorr + originr, cursorc + originc)
	    s.remove_last(1)
	    if s.is_empty then
		pattern.remove(cursorr + originr, cursorc + originc)
	    end
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
	write(0, 0, "Machine: " + machine.name)
	if pattern /= Void then
	    write(300, 0, "Pattern: " + pattern.name)
	    draw_spreadsheet
	else
	    write(width // 2 - 100, height // 2, "Press Ctrl-Enter to create a new pattern.")
	end
    end

    draw_spreadsheet is
    local
	r, c : INTEGER
	it : ITERATOR[INTEGER]
    do
	--draw cursor
	fill_rect(tablex + cursorc * cellx, tabley + cursorr * celly, cellx, celly, blue)

	--draw rules
	aalinecolor(0, tabley, width - 1, tabley, white)
	aalinecolor(tablex, labely, tablex, height - 1, white)

	--draw row labels
	from r := 0
	until r = maxr
	loop
	    draw_row_label(r, (originr + r).to_string)
	    r := r + 1
	end

	--draw col labels
	from c := 0
	until c = maxc
	loop
	    draw_col_label(c, (originc + c).to_string)
	    c := c + 1
	end

	--draw cells
	from r := 0
	until r = maxr
	loop
	    from c := 0
	    until c = maxc
	    loop
		if pattern.has(originr + r, originc + c) then
		    write(tablex + c * cellx, tabley + r * celly,
			    pattern.at(originr + r, originc + c))
		end
		c := c + 1
	    end
	    r := r + 1
	end

	--draw playing rows
	it := song.get_playing_rows(pattern).get_new_iterator
	from it.start
	until it.is_off
	loop
	    r := tabley + (it.item - originr) * celly
	    if r >= tabley and then r < height then
		aalinecolor(0, r, width - 1, r, white)
	    end
	    it.next
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
end
