class BUZZ_PATTERN
creation make
feature
    name : STRING

    make(new_name : STRING; i : INTEGER) is
    do
	!!name.copy(new_name)
	track_count := i
    end

    global_data : ARRAY[STRING]
    data : ARRAY2[STRING]
    row_count : INTEGER
    put_row_count(i : INTEGER) is
    do
	row_count := i
	if track_count > 0 then
	    !!data.make(0, track_count - 1, 0, row_count - 1)
	end
	!!global_data.make(0, row_count - 1)
    end
    track_count : INTEGER

    put_global_data(gd : STRING; i : INTEGER) is
    local
	s : STRING
    do
	!!s.copy(gd)
	global_data.put(s, i)
    end

    put_data(d : STRING; i, j : INTEGER) is
    local
	s : STRING
    do
	!!s.copy(d)
	data.put(s, i, j)
    end
end
