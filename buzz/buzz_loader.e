class BUZZ_LOADER
inherit NOTE_UTIL
creation make
feature
    --in : INPUT_STREAM
    in : PORTABLE_FILE_READ

    make is
    do
	!!last_string.make(16)
    end

    load(f : STRING) : BUZZ_SONG is
    local
	sfr : PORTABLE_FILE_READ
    do
	!!sfr.connect_to(f)
	if sfr.is_connected then
	    in := sfr
	    !!buzz.make
	    parse
	    buzz.dump
	    sfr.disconnect
	    Result := buzz
	end
    end

    buzz : BUZZ_SONG

    --TODO: error checking!
    parse is
    local
	e : BUZZ_DIR_ENTRY
    do
	offset := 0
	parse_header
	from
	until not buzz.entry_has_offset(offset)
	loop
	io.put_string("cur off : " + offset.to_string + "%N")
	    e := buzz.entry_at_offset(offset)
	    if e.name.is_equal("MACH") then
		parse_machine_section
	    elseif e.name.is_equal("CONN") then
		parse_connection_section
	    elseif e.name.is_equal("SEQU") then
		parse_sequence_section
	    elseif e.name.is_equal("PATT") then
		parse_pattern_section
	    elseif e.name.is_equal("WAVT") then
		parse_wavetable_section
	    elseif e.name.is_equal("WAVE") then
		parse_wavedata_section
	    else
		read_string(e.size)
	    end
	end
	get_char
	if not in.end_of_input then
	    io.put_string("didn't reach end of file%N")
	end
    end

    parse_header is
    local
	i, off, size : INTEGER
	e : BUZZ_DIR_ENTRY
    do
	read_string(4)
	if last_string.is_equal("Buzz") then
	    read_dword
	    buzz.put_dir_count(last_integer)
	    from i := 0
	    until i = buzz.dir_count
	    loop
		read_string(4)
		read_dword
		off := last_integer
		read_dword
		size := last_integer
		!!e.make(last_string, off, size)
		buzz.put_entry(e, i)
		i := i + 1
	    end
	    from
	    until i = buzz.dir_max
	    loop
		read_string(12)
		i := i + 1
	    end
	end
    end
    
    parse_machine_section is
    local
	i : INTEGER
    do
	io.put_string("parsing MACH%N")
	read_word
	buzz.put_machine_count(last_integer)
	from i := 0
	until i = buzz.machine_count
	loop
	    buzz.put_machine(parse_machine, i)
	    i := i + 1
	end
    end

    parse_machine : BUZZ_MACHINE is
    local
	i : INTEGER
	x, y : REAL
    do
	read_asciiz
	io.put_string(last_string + "%N")
	!!Result.make(last_string)
	read_byte
	io.put_string(last_integer.to_string + "%N")
	Result.put_type(last_integer)
	if last_integer > 0 then
	    read_asciiz
	    Result.put_dll(last_string)
	end
	io.put_string("DLL: %"" + last_string + "%"%N")

	read_float
	x := last_real

	read_float
	y := last_real

	Result.put_xy(x, y)

	read_dword
	io.put_string("msd size: " + last_integer.to_string + "%N")
	read_string(last_integer)

	read_word
	Result.put_attr_count(last_integer)
	io.put_string("attr count: " + last_integer.to_string + "%N")
	from i := 0
	until i = Result.attr_count
	loop
	    read_asciiz
	    io.put_string("attr: " + last_string + " = ")
	    read_dword
	    io.put_string(last_integer.to_string + "%N")
	    i := i + 1
	end
	io.put_string("gsize: " + Result.global_parm_size.to_string + "%N")
	read_string(Result.global_parm_size)
	read_word
	Result.put_sequence_count(last_integer)
	from i := 0
	until i = Result.sequence_count
	loop
	    read_string(Result.track_parm_size)
	    i := i + 1
	end
    end

    parse_connection_section is
    local
	i : INTEGER
	src, dst, amp, pan : INTEGER
	bc : BUZZ_CONNECTION
    do
	read_word
	buzz.put_connection_count(last_integer)
	from i := 0
	until i = buzz.connection_count
	loop
	    read_word
	    src := last_integer
	    read_word
	    dst := last_integer
	    read_word
	    amp := last_integer
	    read_word
	    pan := last_integer
	    !!bc.make(src, dst, amp, pan)
	    buzz.put_connection(bc, i)
	    i := i + 1
	end
    end

    parse_sequence_section is
    local
	i, j, k : INTEGER
	seq : BUZZ_SEQUENCE
	bppos, bpe : INTEGER
    do
	read_dword
	i := last_integer
	read_dword
	j := last_integer
	read_dword
	k := last_integer
	buzz.put_song_info(i, j, k)
	read_word
	buzz.put_sequence_count(last_integer)
	from i := 0
	until i = buzz.sequence_count
	loop
	    read_word
	    !!seq.make(last_integer)
	    read_dword
	    seq.put_event_count(last_integer)
	    read_byte
	    bppos := last_integer
	    read_byte
	    bpe := last_integer
	    from j := 0
	    until j = seq.event_count
	    loop
		read_bytes(bppos)
		seq.put_eventpos(last_integer, j)
		read_bytes(bpe)
		seq.put_event(last_integer, j)
		j := j + 1
	    end
	    buzz.put_sequence(seq, i)
	    i := i + 1
	end
    end

    parse_pattern_section is
    local
	i, j : INTEGER
	track_count : INTEGER
	m : BUZZ_MACHINE
	p : BUZZ_PATTERN
    do
	from i := 0
	until i = buzz.machine_count
	loop
	    m := buzz.machine.item(i)
	    read_word
	    m.put_pattern_count(last_integer)
	    read_word
	    track_count := last_integer
	    from j := 0
	    until j = m.pattern_count
	    loop
		read_asciiz
		!!p.make(last_string, track_count)
		read_word
		p.put_row_count(last_integer)
		parse_pattern_data(p, m)
		m.put_pattern(p, j)
		j := j + 1
	    end
	    i := i + 1
	end
    end
    
    parse_pattern_data(p : BUZZ_PATTERN; m : BUZZ_MACHINE) is
    local
	i, j : INTEGER
	in_count, glen, tlen : INTEGER
    do
	tlen := m.track_parm_size
	glen := m.global_parm_size
	in_count := m.in_count
	from j := 0
	until j = in_count
	loop
	    read_word
--io.put_string("in: " + last_integer.to_string + "%N")
	    from i := 0
	    until i = p.row_count
	    loop
		read_word
--io.put_string("amp: " + last_integer.to_hexadecimal + "%N")
		read_word
--io.put_string("pan: " + last_integer.to_hexadecimal + "%N")
		i := i + 1
	    end
	    j := j + 1
	end

	from i := 0
	until i = p.row_count
	loop
	    read_string(glen)
	    p.put_global_data(last_string, i)
	    i := i + 1
	end

	from j := 0
	until j = p.track_count
	loop
	    from i := 0
	    until i = p.row_count
	    loop
		read_string(tlen)
		p.put_data(last_string, j, i)
		i := i + 1
	    end
	    j := j + 1
	end
    end

    parse_wavetable_section is
    local
	i, j : INTEGER
	e : BUZZ_WAVETABLE_ENTRY
	bl : BUZZ_LEVEL
    do
	read_word
	buzz.put_wave_count(last_integer)
	from i := 0
	until i = buzz.wave_count
	loop
	    read_word
io.put_string("index: ")
io.put_integer(last_integer)
io.put_string("%N")
if last_integer /= i then
    io.put_string("index /= i!%N")
    die_with_code(1)
end
	    !!e.make(last_integer)
	    read_asciiz
	    e.put_filename(last_string)
	    read_asciiz
	    e.put_name(last_string)
	    read_float
	    e.put_volume(last_real.to_double)
	    read_byte

	    read_byte
	    e.put_level_count(last_integer)
	    from j := 0
	    until j = e.level_count
	    loop
		read_dword
		!!bl.make(last_integer)
io.put_string("size: " + last_integer.to_string + "%N")
		read_dword
		read_dword
		read_dword
		get_char
		bl.put_root_freq(buzz_note_to_freq(last_char))
		e.level_table.put(bl, j)
		j := j + 1
	    end
	    buzz.put_wavetable_entry(e, e.index)
	    i := i + 1
	end
    end

    parse_wavedata_section is
    local
	i, j, k : INTEGER
	n : INTEGER
	e : BUZZ_WAVETABLE_ENTRY
	bl : BUZZ_LEVEL
	total : INTEGER
    do
	read_word
	n := last_integer
	if n > buzz.wave_count then
	    io.put_string("n > buzz.wave_count!%N")
	    die_with_code(1)
	end
	from i := 0
	until i = n
	loop
	    read_word
	    e := buzz.wavetable.item(last_integer)
	    read_byte
	    if last_integer /= 0 then
		die_with_code(1)
		io.put_string("can't handle this wav format%N")
	    end
	    read_dword
io.put_string("total: " + last_integer.to_string + "%N")
	    total := last_integer
	    from j := 0
	    until j = e.level_count
	    loop
		bl := e.level_table @ j
		bl.init_data(bl.length * 2)
		from k := 0
		until k = bl.length
		loop
		    get_char
		    bl.data.put(last_char, 2 * k)
		    get_char
		    bl.data.put(last_char, 2 * k + 1)
		    k := k + 1
		end
		j := j + 1
	    end
	    i := i + 1
	end
    end

    offset : INTEGER
    last_string : STRING
    last_integer : INTEGER
    last_real : REAL

    last_char : CHARACTER is
    do
	Result := in.last_character
    end

    read_string(n : INTEGER) is
    local
	i : INTEGER
    do
	last_string.clear
	from i := 0
	until i = n
	loop
	    get_char
	    last_string.add_last(last_char)
	    i := i + 1
	end
    end

    read_byte is
    do
	get_char
	last_integer := last_char.code
    end

    read_bytes(n : INTEGER) is
    local
	i : INTEGER
	pwr256 : INTEGER
    do
	pwr256 := 1
	last_integer := 0
	from i := 0
	until i = n
	loop
	    get_char
	    last_integer := last_integer + last_char.code * pwr256
	    pwr256 := pwr256 * 256
	    i := i + 1
	end
    end

    read_dword is
    do
	get_char
	last_integer := last_char.code
	get_char
	last_integer := last_integer + last_char.code * 256
	get_char
	last_integer := last_integer + last_char.code * 256 * 256
	get_char
	last_integer := last_integer + last_char.code * 256 * 256 * 256
    end

    read_word is
    do
	get_char
	last_integer := last_char.code
	get_char
	last_integer := last_integer + last_char.code * 256
    end

    read_float is
    do
	read_dword
	last_real := ext_int_to_float(last_integer)
    end

    ext_int_to_float(i : INTEGER) : REAL is
    external "C"
    end

    read_asciiz is
    do
	last_string.clear
	from get_char
	until last_char.code = 0
	loop
	    last_string.add_last(last_char)
	    get_char
	end
    end

    get_char is
    do
	in.read_character
	offset := offset + 1
    end
end
