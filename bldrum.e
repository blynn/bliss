class BLDRUM
inherit GENERATOR
creation register
feature
    id : STRING is "BLDrum"

    next_sample : DOUBLE is
    local
	it : ITERATOR[SAMPLER]
    do
	it := drum_table.get_new_iterator_on_items
	from it.start
	until it.is_off
	loop
	    Result := Result + it.item.next_sample
	    it.next
	end
    end

    process_command(s : STRING) is
    do
io.put_string("command = " + s + "%N")
	if is_note(s, 1) then
io.put_string("command is noteworthy%N")
io.put_string(note_to_rank(s, 1).to_string + "%N")
	    play_freq(note_freq_table.item(note_to_rank(s, 1)))
	elseif s.is_equal("KD") then
	    kick_drum
	elseif s.is_equal("SD") then
	    snare_drum
	end
    end

    boom is
    do
	if ttl > 0 then
	    ttl := ttl - 1
	    if ttl = 0 then
		drum_table.remove("sine")
	    end
	end
    end

    ttl : INTEGER
    play_freq(d : DOUBLE) is
    local
	o : OSCER
	s : SINE
    do
	!!s.make
	!!o.make(s)
	o.put_freq(d)
	ttl := 3
	drum_table.put(o, "sine")
    end

    drum_table : DICTIONARY[SAMPLER, STRING]

    init is
    do
	!!drum_table.make
    end

    kick_drum is
    local
	s : SINE
	o : OSCER
	c : CURVER
	fm : FREQ_MOD
	rm : RING_MOD
    do
	!!s.make
	!!o.make(s)
	!!fm.make
	fm.put_input(o)
	!!c.make(330.0, 0.0)
	fm.put_modulator(c)
	!!c.make(1.0, 0.0)
	!!rm.make(fm, c)
	drum_table.put(rm, "kick")
    end

    snare_drum is
    local
	n : NOISE
	c : CURVER
	rm : RING_MOD
    do
	!!n.make
	n.put_freq(20000)
	!!c.make(1.0, 0.0)
	!!rm.make(n, c)
	drum_table.put(rm, "snare")
    end
end
