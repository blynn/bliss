class CONV_JESKOLA_TRACKER
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Tracker"
    bliss_id : STRING is "BLTracker"
    global_parm_count : INTEGER is 1
    track_parm_count : INTEGER is 5

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    local
	i : INTEGER
	r, t : INTEGER
	bp : BUZZ_PATTERN
	p : PATTERN
	s : STRING
	exists_event : BOOLEAN
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
		    exists_event := False
		    s := bp.data.item(t, r)
		    if s.item(1).code /= 0 then
			p.put(to_note(s.item(1)), r, t * 6 + 1)
			exists_event := True
		    end
		    if s.item(2).code /= 0 then
			p.put("S" + hex255table.item(s.item(2).code), r, t * 6 + 2)
			exists_event := True
		    end
		    if s.item(3).code /= 255 then
			p.put("V" + hex255table.item(s.item(3).code), r, t * 6 + 3)
			exists_event := True
		    end
		    if s.item(4).code /= 0 then
			if s.item(4).code = 1 then
			    p.put("vd" + hex255table.item(s.item(5).code), r, t * 6 + 4)
			end
			exists_event := True
		    end
		    if exists_event then
			p.put("T" + t.to_string, r, t * 6)
		    end
		    t := t + 1
		end
		r := r + 1
	    end
	    i := i + 1
	end
    end
end
