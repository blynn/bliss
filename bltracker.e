class BLTRACKER
inherit GENERATOR
creation register
feature
    id : STRING is "BLTracker"

    boom is
    do
    end

    sample : INTEGER is
    do
	Result := song.sample_mod_tick
    end
    sample_rate : INTEGER is
    do
	Result := song.samples_per_tick
    end

    max_track : INTEGER is 8

    track_array : NATIVE_ARRAY[BLTRACKER_WAVE]
    track : INTEGER
    put_track(i : INTEGER) is
    do
	if i >= 0 and then i < max_track then
	    track := i
	end
    end

    precision : INTEGER is 1024

    work : DOUBLE is
    local
	i : INTEGER
    do
	from i := 0
	until i = max_track
	loop
	    Result := Result + track_array.item(i).next_sample
	    i := i + 1
	end
    end

    process_command(s : STRING) is
    local
	i : INTEGER
    do
io.put_string("command = " + s + "%N")
	inspect s.item(1)
	when 'V' then
	    i := s.item(2).hexadecimal_value
	    if s.item(3).is_hexadecimal_digit then
		i := i * 16 + s.item(3).hexadecimal_value
	    end
	    track_array.item(track).put_vol(i / 128)
	when 'S' then
	    i := s.item(2).hexadecimal_value * 16 + s.item(3).hexadecimal_value
	    track_array.item(track).put_wave_index(i)
	when 'T' then
	    --i := s.item(2).hexadecimal_value * 16 + s.item(3).hexadecimal_value
	    i := s.item(2).hexadecimal_value
	    put_track(i)
	when 'v' then
	    i := s.item(3).hexadecimal_value * 16 + s.item(4).hexadecimal_value
	    track_array.item(track).volslidedown(i)
	else
	    if is_note(s, 1) then
		track_array.item(track).play_freq(note_freq_table.item(note_to_rank(s, 1)))
	    end
	end
    end

    init is
    local
	t : BLTRACKER_WAVE
	i : INTEGER
    do
	track_array := track_array.calloc(max_track)
	from i := 0
	until i = max_track
	loop
	    !!t.make
	    track_array.put(t, i)
	    i := i + 1
	end
    end
end
