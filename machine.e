deferred class MACHINE
inherit
    COLOR_TABLE;
    SONG_TABLE;
    NOTE_UTIL
feature
    id : STRING is
    deferred
    end

    boom is
    deferred
    end

    init_inmachine(m : MACHINE) is
    do --deferred
    end

    name : STRING
    put_name(s : STRING) is
    do
	!!name.copy(s)
    end

    process_command(s : STRING) is
    deferred
    end

    is_source : BOOLEAN is
    deferred
    end

    is_sink : BOOLEAN is
    deferred
    end

    in_connection : LINKED_LIST[EDGE]

    pattern_table : DICTIONARY[PATTERN, STRING]
    pattern_list : LINKED_LIST[PATTERN]
    setup_pattern : PATTERN

    add_inedge(e : EDGE) is
    do
	in_connection.add_last(e)
    end

    remove_inedge(e : EDGE) is
    require
	in_connection.fast_has(e)
    do
	in_connection.remove(in_connection.fast_index_of(e))
    end

    bgcolor : COLOR is
    deferred
    end

    posx, posy : INTEGER
    width, height : INTEGER

    new_pattern(s : STRING) : PATTERN is
    require
	not pattern_table.has(s)
    do
	!!Result.make(s, Current)
	pattern_table.put(Result, s)
	pattern_list.add_last(Result)
    end

    new_pattern_autoname : PATTERN is
    local
	i : INTEGER
    do
	from
	until not pattern_table.has(i.to_string)
	loop
	    i := i + 1
	end
	Result := new_pattern(i.to_string)
    end

    next_pattern(pat : PATTERN) : PATTERN is
    local
	i : INTEGER
    do
	i := pattern_list.fast_index_of(pat) + 1
	if i > pattern_list.upper then
	    i := pattern_list.lower
	end
	Result := pattern_list @ i
    end

    free is
    do
	image.free
    end

    image : IMAGE

    centerx : INTEGER is
    do
	Result := posx + width // 2
    end

    centery : INTEGER is
    do
	Result := posy + height // 2
    end

    put_heading(s : STRING) is
    do
	image.draw_text(4, 6, s)
    end

    show_info is
    local
	it : ITERATOR[STRING]
    do
	it := pattern_table.get_new_iterator_on_keys
	from it.start
	until it.is_off
	loop
	    io.put_string(it.item + "%N")
	    it.next
	end
    end

    setup is
    do
	setup_pattern.instant_execute
    end

    old_history : ARRAY[DOUBLE]
    old_history_size : INTEGER
    old_history_current : INTEGER

    old_process_history_size_request(m : MACHINE; n : INTEGER) is
    do
io.put_string("ph " + m.name + " " + n.to_string + "%N")
	if n > history_size then
	    history_size := n
io.put_string("hs " + history_size.to_string + "%N")
	    !!history.make(0, n-1)
	end
    end

    mix_input : DOUBLE
    last_frame : DOUBLE

    visited : BOOLEAN
    clear_visited is
    do
	visited := False
    end

    compute_next_frame is
    local
	it : ITERATOR[EDGE]
    do
	if not visited then
	    mix_input := 0
	    it := in_connection.get_new_iterator
	    from it.start
	    until it.is_off
	    loop
		it.item.src.compute_next_frame
		mix_input := mix_input + it.item.amp * it.item.src.last_frame
		it.next
	    end
	    last_frame := work
	    visited := True
	end
    end

    old_add_history(d : DOUBLE) is
    require
	history_size > 0
    do
	history.put(d, history_current)
	history_current := history_current + 1
	if history_current = history_size then
	    history_current := 0
	end
    end

    old_last_frame : DOUBLE is
    require
	history_size > 0
    local
	i : INTEGER
    do
	i := history_current - 1
	if i < 0 then
	    i := history_size - 1
	end
	Result := history @ i
    end

    work : DOUBLE is
    deferred
    end

    old_request_history_size(m : MACHINE; n : INTEGER) is
    do
	m.process_history_size_request(Current, n)
    end

feature {MACHINE_TABLE}
    make_instance(s : STRING) : like Current is
    do
	Result := clone(Current)
	Result.put_name(s)
	Result.init_connection_lists
	Result.init_pattern_table
	Result.init_picture
	Result.init
	--Result.request_history_size(Result, 1)
    end

feature {MACHINE}
    init is
    deferred
    end

    init_picture is
    do
	posx := 0
	posy := 0
	width := 70
	height := 25
	!!image.make
	image.new(width, height)
	image.box(0, 0, width - 1, height - 1, white)
	image.box(2, 2, width - 3, height - 3, bgcolor)
	put_heading(name)
    end

    init_connection_lists is
    do
	!!in_connection.make
    end

    init_pattern_table is
    do
	!!pattern_table.make
	!!pattern_list.make
	!!setup_pattern.make("SETUP", Current)
    end

feature {SONG}
    move_to(new_x, new_y : INTEGER) is
    do
	posx := new_x
	posy := new_y
    end

    move(rx, ry : INTEGER) is
    do
	posx := posx + rx
	posy := posy + ry
    end

    register(table : DICTIONARY[MACHINE, STRING]) is
    do
	table.put(Current, id)
    end
end
