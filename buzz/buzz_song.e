class BUZZ_SONG
creation make
feature
    make is
    do
    end

    song_end, loop_begin, loop_end : INTEGER

    put_song_info(a, b, c : INTEGER) is
    do
	song_end := a
	loop_begin := b
	loop_end := c
    end

    dir : ARRAY[BUZZ_DIR_ENTRY]
    dir_table : DICTIONARY[BUZZ_DIR_ENTRY, INTEGER]
    dir_count : INTEGER
    dir_max : INTEGER is 31

    wavetable : ARRAY[BUZZ_WAVETABLE_ENTRY]
    wave_count : INTEGER

    put_wave_count(i : INTEGER) is
    do
	wave_count := i
	!!wavetable.make(0, wave_count - 1)
    end
    put_wavetable_entry(e : BUZZ_WAVETABLE_ENTRY; i : INTEGER) is
    do
	wavetable.put(e, i)
    end

    machine : ARRAY[BUZZ_MACHINE]
    machine_count : INTEGER

    connection : ARRAY[BUZZ_CONNECTION]
    connection_count : INTEGER

    sequence : ARRAY[BUZZ_SEQUENCE]
    sequence_count : INTEGER

    put_sequence(seq : BUZZ_SEQUENCE; i : INTEGER) is
    do
	sequence.put(seq, i)
    end

    put_sequence_count(i : INTEGER) is
    do
	sequence_count := i
	!!sequence.make(0, sequence_count - 1)
    end

    put_dir_count(i : INTEGER) is
    require
	i > 0
	i <= dir_max
    do
	dir_count := i
	!!dir.make(0, dir_count - 1)
	!!dir_table.make
    end

    put_entry(e : BUZZ_DIR_ENTRY; i : INTEGER) is
    do
	dir.put(e, i)
	dir_table.put(e, e.offset)
    end

    put_machine_count(i : INTEGER) is
    require
	i > 0
    do
	machine_count := i
	!!machine.make(0, machine_count - 1)
    end

    put_machine(m : BUZZ_MACHINE; i : INTEGER) is
    require
	i >= 0
	i < machine_count
    do
	machine.put(m, i)
    end

    put_connection_count(i : INTEGER) is
    require
	i > 0
    do
	connection_count := i
	!!connection.make(0, connection_count)
    end

    put_connection(c : BUZZ_CONNECTION; i : INTEGER) is
    require
	i >= 0
	i < connection_count
    do
	connection.put(c, i)
	machine.item(c.dst).increment_in_count
    end

    entry_has_offset(off : INTEGER) : BOOLEAN is
    do
	Result := dir_table.has(off)
    end
    entry_at_offset(off : INTEGER) : BUZZ_DIR_ENTRY is
    require
	dir_table.has(off)
    do
	Result := dir_table.at(off)
    end

    dump is
    local
	i : INTEGER
	e : BUZZ_DIR_ENTRY
	m : BUZZ_MACHINE
	c : BUZZ_CONNECTION
    do
	from i := 0
	until i = dir_count
	loop
	    io.put_string(i.to_string + ": ")
	    e := dir @ i
	    io.put_string(e.name + ", offset = ")
	    io.put_integer(e.offset)
	    io.put_string(", size = ")
	    io.put_integer(e.size)
	    io.put_string("%N")
	    i := i + 1
	end

	from i := 0
	until i = machine_count
	loop
	    m := machine @ i
	    io.put_string("Machine: " + m.name + "%N")
	    io.put_string("x, y = " + m.x.to_string)
	    io.put_string(", " + m.y.to_string + "%N")
	    i := i + 1
	end

	from i := 0
	until i = connection_count
	loop
	    c := connection @ i
	    io.put_string("connection from ")
	    io.put_integer(c.src)
	    io.put_string(" to ")
	    io.put_integer(c.dst)
	    io.put_new_line
	    i := i + 1
	end
    end
end
