class SONG
inherit
    MACHINE_TABLE
creation make
feature
    master : MACHINE

    play_flag : BOOLEAN

    play is
    do
	play_flag := True
    end

    pause is
    do
	play_flag := False
    end

    new_machine(name : STRING; mclass : STRING) : MACHINE is
    require
	machine_class_table.has(mclass)
	not machine_table.has(name)
    local
	m : MACHINE
    do
	m := machine_class_table.at(mclass)
	Result := add_machine(m, name);
    end

    new_machine_autoname(mclass : STRING) : MACHINE is
    require
	machine_class_table.has(mclass)
    local
	s : STRING
	i : INTEGER
	m : MACHINE
    do
	m := machine_class_table.at(mclass)
	s := m.id
	if machine_table.has(s) then
	    from  i := 2
	    until not machine_table.has(s + i.to_string)
	    loop
		i := i + 1
	    end
	    s := s + i.to_string
	end
	Result := add_machine(m, s)
    end

    add_track(m : MACHINE) is
    local
	t : TRACK
    do
	!!t.make(m)
	tracks.add_last(t)
    end

    machine_table : DICTIONARY[MACHINE, STRING]
    machine_list : LINKED_LIST[MACHINE]
    machine_zorder : LINKED_LIST[MACHINE]
    connection_list : LINKED_LIST[EDGE]

    next_machine(m : MACHINE) : MACHINE is
    local
	i : INTEGER
    do
	i := machine_list.fast_index_of(m) + 1
	if i > machine_list.upper then
	    i := machine_list.lower
	end
	Result := machine_list @ i
    end

    tracks : LINKED_LIST[TRACK]

    wave_table : DICTIONARY[WAVE, INTEGER]

    put_wave(w : WAVE; i : INTEGER) is
    do
	wave_table.put(w, i)
    end

    layoutx : INTEGER is 638
    layouty : INTEGER is 438

    make is
    do
	!!machine_zorder.make
	!!machine_table.make
	!!machine_list.make
	!!connection_list.make
	!!tracks.make
	!!wave_table.make
	master := new_machine("Master", "Master")
	master.move_to((layoutx - master.width) // 2, (layouty - master.height) // 2)
	end_beat := 16
    end

    import_buzz(buzz : BUZZ_SONG) is
    local
	bc : BUZZ_CONVERTER
    do
	!!bc.make
	bc.convert(Current, buzz)
    end

    connect(src, dst : MACHINE) is
    require
	src.is_source
	dst.is_sink
    local
	edge : EDGE
    do
	if src = dst then
	    io.put_string("cycle detected%N")
	elseif exists_edge(src, dst) then
	    io.put_string("duplicate edge detected%N")
	else
	    !!edge.make(src, dst)
	    dst.add_inedge(edge)
	    if is_cyclic then
		dst.remove_inedge(edge)
		io.put_string("cycle detected%N")
	    else
		connection_list.add_last(edge)
	    end
	    toposort
	end
    end

    exists_edge(src, dst : MACHINE) : BOOLEAN is
    local
	it : ITERATOR[EDGE]
    do
	it := dst.in_connection.get_new_iterator
	from it.start
	until it.is_off or else Result = True
	loop
	    if it.item.src = src then
		Result := True
	    else
		it.next
	    end
	end
    end

    delete_edge(e : EDGE) is
    local
	i : INTEGER
    do
	i := connection_list.fast_index_of(e)
	connection_list.remove(i)
	e.dst.remove_inedge(e)
	toposort
    end

    move_to(m : MACHINE; new_x, new_y : INTEGER) is
    do
	m.move_to(new_x, new_y)
    end

    rewind is
    local
	it : ITERATOR[TRACK]
    do
	beat_count := 0
	it := tracks.get_new_iterator
	from it.start
	until it.is_off
	loop
	    it.item.rewind
	    it.next
	end
	setup_machines
    end

    setup_machines is
    local
	it : ITERATOR[MACHINE]
    do
	it := machine_list.get_new_iterator
	from it.start
	until it.is_off
	loop
	    it.item.setup
	    it.next
	end
    end

    beat_count : INTEGER
    end_beat : INTEGER
    loop_begin : INTEGER
    loop_end : INTEGER

    put_end_beat(eb : INTEGER) is
    do
	end_beat := eb
    end

    put_loop_begin(i : INTEGER) is
    do
	loop_begin := i
    end

    put_loop_end(i : INTEGER) is
    do
	loop_end := i
    end

    boom is
    local
	it : ITERATOR[TRACK]
	itm : ITERATOR[MACHINE]
    do
	if beat_count >= end_beat then
	    beat_count := 0
	end
	itm := machine_list.get_new_iterator
	from itm.start
	until itm.is_off
	loop
	    itm.item.boom
	    itm.next
	end

	it := tracks.get_new_iterator
	from it.start
	until it.is_off
	loop
	    it.item.boom(beat_count)
	    it.next
	end
	beat_count := beat_count + 1
    end

    get_playing_rows(pat : PATTERN) : LINKED_LIST[INTEGER] is
    local
	it : ITERATOR[TRACK]
    do
	!!Result.make
	it := tracks.get_new_iterator
	from it.start
	until it.is_off
	loop
	    if it.item.playing_pattern = pat then
		Result.add_last(beat_count - it.item.offset)
	    end
	    it.next
	end
    end

    has(b, i : INTEGER) : BOOLEAN is
    require
	i >= tracks.lower
	i <= tracks.upper
    do
	Result := (tracks @ i).commands.has(b)
    end

    at(b, i : INTEGER) : STRING is
    require
	i >= tracks.lower
	i <= tracks.upper
	has(b, i)
    do
	Result := (tracks @ i).commands.at(b)
    end

    put(cmd : STRING; b, i : INTEGER) is
    require
	i >= tracks.lower
	i <= tracks.upper
    do
	(tracks @ i).put(cmd, b)
    end

    remove(b, i : INTEGER) is
    require
	i >= tracks.lower
	i <= tracks.upper
	has(i, b)
    do
	(tracks @ i).commands.remove(b)
    end

    save(saver : SAVER) is
    local
	it : ITERATOR[MACHINE]
	m : MACHINE
    do
	it := machine_zorder.get_new_iterator
	from it.start
	until it.is_off
	loop
	    m := it.item
	    saver.start_machine(m.name)
	    saver.put_machine_class(m.id)
	    saver.put_machine_pos(m.posx, m.posy)
	    saver.end_machine
	    it.next
	end

	it := machine_zorder.get_new_iterator
	saver.start_zorder
	from it.start
	until it.is_off
	loop
	    saver.put_machine_name(it.item.name)
	    it.next
	end
	saver.end_zorder
    end

	next_sample : DOUBLE is
	do
	    if play_flag then
		if new_beat then
		    boom
		end
		increment_sample_count
		unvisit_all_machines
		master.compute_next_frame
		Result := master.last_frame
		if Result > 1.0 then
		    Result := 1.0
		elseif Result < -1.0 then
		    Result := -1.0
		end
	    end
	end
	
	unvisit_all_machines is
	local
	    it : ITERATOR[MACHINE]
	do
	    it := machine_list.get_new_iterator
	    from it.start
	    until it.is_off
	    loop
		it.item.clear_visited
		it.next
	    end
	end

	bpm : INTEGER
	tpb : INTEGER
	sample_mod_tick : INTEGER
	sample_count : INTEGER
	samples_per_tick : INTEGER

	jump_to(b : INTEGER) is
	do
	    sample_mod_tick := 0
	    beat_count := b
	end
	
	new_beat : BOOLEAN is
	do
		Result := sample_mod_tick = 0
	end

	put_bpm_tpb(b, t : INTEGER) is
	do
		bpm := b
		tpb := t
		calculate_samples_per_tick
	end

	increment_sample_count is
	do
		sample_count := sample_count + 1
		sample_mod_tick := sample_mod_tick + 1
		if sample_mod_tick = samples_per_tick then
			sample_mod_tick := 0
		end
	end

	calculate_samples_per_tick is
	do
		sample_mod_tick := 0
		samples_per_tick := (60 * 44100 / (bpm * tpb)).rounded
	end

feature {NONE}
    add_machine(m : MACHINE; name : STRING) : MACHINE is
    do
	Result := m.make_instance(name)
	machine_table.put(Result, name)
	machine_zorder.add_last(Result)
	machine_list.add_last(Result)
    end

    machine_topo_order : LINKED_LIST[MACHINE]

    toposort is
    local
	sl : LINKED_LIST[MACHINE]
	m : MACHINE
	it : ITERATOR[EDGE]
    do
	!!sl.make
	sl.add_last(master)
	!!machine_topo_order.make
	from
	until sl.is_empty
	loop
	    m := sl.last
	    sl.remove_last
	    machine_topo_order.add_first(m)
	    it := m.in_connection.get_new_iterator
	    from it.start
	    until it.is_off
	    loop
		if not machine_topo_order.fast_has(it.item.src) then
		    sl.add_last(it.item.src)
		end
		it.next
	    end
	end
    end

    is_cyclic : BOOLEAN is
    do
    end
end
