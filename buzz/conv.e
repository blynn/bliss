deferred class CONV
inherit NOTE_UTIL
feature
    buzz_dll : STRING is
    deferred
    end

    bliss_id : STRING is
    deferred
    end

    global_parm_count : INTEGER is
    deferred
    end

    track_parm_count : INTEGER is
    deferred
    end

    register(table : DICTIONARY[CONV, STRING]) is
    do
	table.put(Current, buzz_dll)
    end

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    deferred
    end

    hex255table : ARRAY[STRING] is
    local
	i, j : INTEGER
	s : STRING
    once
	!!Result.make(0, 255)
	from i := 0
	until i = 16
	loop
	    from j := 0
	    until j = 16
	    loop
		!!s.make(2)
		s.add_last(i.hexadecimal_digit)
		s.add_last(j.hexadecimal_digit)
		Result.put(s, i * 16 + j)
		j := j + 1
	    end
	    i := i + 1
	end
    end
end
