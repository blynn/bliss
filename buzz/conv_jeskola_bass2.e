class CONV_JESKOLA_BASS2
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Bass 2"
    bliss_id : STRING is "BLBass2"
    global_parm_count : INTEGER is 0
    track_parm_count : INTEGER is 9

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    local
	i : INTEGER
	r, t : INTEGER
	bp : BUZZ_PATTERN
	p : PATTERN
	s : STRING
    do
	from i := 0
	until i = bm.pattern_count
	loop
	    bp := bm.pattern.item(i)
	    p := m.new_pattern(bp.name)
	    from r := 0
	    until r = bp.row_count
	    loop
		from t := 0
		until t = bp.track_count
		loop
		    s := bp.data.item(t, r)
		    if s.item(6).code /= 0 then
			p.put(to_note(s.item(6)), r, 6)
		    end
		    if s.item(7).code /= 255 then
			p.put("V" + hex255table.item(s.item(7).code), r, 7)
		    end
		    if s.item(8).code /= 0 then
			p.put("L" + s.item(8).code.to_string, r, 8)
		    end
		    t := t + 1
		end
		r := r + 1
	    end
	    i := i + 1
	end
    end
end
