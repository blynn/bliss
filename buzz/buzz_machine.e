class BUZZ_MACHINE
inherit CONV_TABLE
creation make
feature
    name : STRING
    type : INTEGER
    dll : STRING
    x, y : REAL

    attr_count : INTEGER
    sequence_count : INTEGER

    make(new_name : STRING) is
    do
	!!name.copy(new_name)
    end

    pattern : ARRAY[BUZZ_PATTERN]
    pattern_count : INTEGER
    put_pattern(p : BUZZ_PATTERN; i : INTEGER) is
    require
	i >= 0
	i < pattern_count
    do
	pattern.put(p, i)
    end
    put_pattern_count(i : INTEGER) is
    do
	pattern_count := i
	!!pattern.make(0, pattern_count - 1)
    end

    put_type(new_type : INTEGER) is
    do
	type := new_type
	if type = 0 then
	    global_parm_size := 5
	    track_parm_size := 0
	end
    end

    put_dll(new_dll : STRING) is
    local
	c : CONV
    do
	!!dll.copy(new_dll)
	if conv_table.has(dll) then
	    c := conv_table.at(dll)
	    global_parm_size := c.global_parm_count
	    track_parm_size := c.track_parm_count
	else
	    io.put_string("Unhandled machine: " + dll + "%N")
	    die_with_code(1)
	end
    end

    put_attr_count(i : INTEGER) is
    do
	attr_count := i
    end

    put_sequence_count(i : INTEGER) is
    do
	sequence_count := i
    end

    global_parm_size : INTEGER
    track_parm_size : INTEGER

    put_xy(new_x, new_y : REAL) is
    do
	x := new_x
	y := new_y
    end

    in_count : INTEGER
    increment_in_count is
    do
	in_count := in_count + 1
    end
end
