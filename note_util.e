deferred class NOTE_UTIL
feature
    is_note(s : STRING; i : INTEGER) : BOOLEAN is
    require
	i <= s.upper
    local
	j : INTEGER
    do
	j := i
	if s.item(j) >= 'A' and then s.item(j) <= 'G' then
	    j := j + 1
	    if j <= s.upper and then s.item(j) = '#' then
		j := j + 1
	    end
	    if j <= s.upper and then s.item(j).is_digit then
		Result := True
	    end
	end
    end

    note_rank_array : ARRAY[INTEGER] is
    once
	Result := <<0,2,4,5,7,9,11>>
    end

    note_to_rank(s : STRING; i : INTEGER) : INTEGER is
    require
	is_note(s, i)
    local
	j : INTEGER
    do
	j := i
	Result := s.item(j).code - ('C').code
	if Result < 0 then
	    Result := Result + 7
	end
	Result := note_rank_array.item(Result + 1)
	j := j + 1
	if s.item(j) = '#' then
	    Result := Result + 1
	    j := j + 1
	end
	Result := Result + 12 * s.item(j).value
    end

    note_max : INTEGER is 127
    note_freq_table : ARRAY[DOUBLE] is
    local
	i : INTEGER
    once
	!!Result.make(0, note_max)
	from i := 0
	until i > note_max
	loop
	    Result.put(220.0 * (2.0).pow((i - 57) / 12), i)
	    i := i + 1
	end
    end

    scale_array : ARRAY[STRING] is
    once
	Result := <<"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B">>
    end

    to_note(c : CHARACTER) : STRING is
    local
	i : INTEGER
    do
	!!Result.make(3)
	i := c.code \\ 16
	Result.copy(scale_array @ i)
	i := c.code // 16
	Result.add_last(i.digit)
    end

    buzz_note_to_freq(c : CHARACTER) : DOUBLE is
    local
	i : INTEGER
    do
	i := c.code \\ 16 - 1
	i := i + (c.code // 16) * 12
	Result := note_freq_table.item(i)
    end
end
