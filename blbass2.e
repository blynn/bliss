class BLBASS2
inherit GENERATOR
creation register
feature
    id : STRING is "BLBass2"

    boom is
    do
    end

    resolution : INTEGER is 1024

    pi2 : DOUBLE is
    once
	Result := 8.0 * (1.0).atan
    end

    sin_table : ARRAY[DOUBLE] is
    local
	i : INTEGER
    once
	!!Result.make(0, resolution - 1)
	from i := 0
	until i = resolution
	loop
	    Result.put(0.5 * ((i / resolution) * pi2).sin, i)
	    i := i + 1
	end
    end

    saw_table : ARRAY[DOUBLE] is
    local
	i : INTEGER
    once
	!!Result.make(0, resolution - 1)
	from i := 0
	until i = resolution
	loop
	    Result.put(0.5 - (i / resolution), i)
	    i := i + 1
	end
    end

    fade_resolution : INTEGER is 128
    fade_table : ARRAY[DOUBLE] is
    local
	i : INTEGER
    once
	!!Result.make(0, fade_resolution)
	from i := 0
	until i = fade_resolution
	loop
	    Result.put((2.0).pow(i / fade_resolution) - 1, i)
	    i := i + 1
	end
    end
    fade_in : BOOLEAN
    fade_out : BOOLEAN

    ttl : INTEGER
    put_length(l : INTEGER) is
    do
	ttl := l
	fade_in := True
	if ttl = 1 then
	    fade_out := True
	else
	    fade_out := False
	end
    end

    x : INTEGER

    precision : INTEGER is 32

    modulus : INTEGER is
    once
	Result := 44100 * precision 
    end

    fade_time : INTEGER is
    do
	Result := sample_rate // 16
    end

    freq : INTEGER
    vol : DOUBLE

    sample : INTEGER is
    do
	Result := song.sample_mod_tick
    end
    sample_rate : INTEGER is
    do
	Result := song.samples_per_tick
    end

    put_vol(d : DOUBLE) is
    do
	vol := d
    end

    put_freq(d : DOUBLE) is
    do
	freq := (d * precision).rounded
    end

    wave_table : ARRAY[DOUBLE]

    next_sample : DOUBLE is
    do
	if ttl > 0 then
	    Result := wave_table.item(x * resolution // modulus) * vol
	    x := x + freq
	    if x >= modulus then
		x := x - modulus
		check
		    x < modulus
		end
	    end
	    if sample = 0 then
		ttl := ttl - 1
		if ttl = 1 then
		    fade_out := True
		end
	    end
	    if fade_in then
		if sample < fade_time then
		    Result := Result * fade_table.item(sample * fade_resolution // fade_time)
		end
	    else
		fade_in := False
	    end
	    if fade_out then
		if sample > sample_rate - fade_time then
		    Result := Result * fade_table.item((sample_rate - sample) * fade_resolution // fade_time)
		end
	    end
	end
    end

    process_command(s : STRING) is
    local
	i : INTEGER
    do
io.put_string("command = " + s + "%N")
	if s.item(1) = 'L' then
	    i := s.item(2).value
	    if s.count > 2 and then s.item(3).is_digit then
		i := i * 10 + s.item(3).value
	    end
	    put_length(i)
	elseif s.item(1) = 'V' then
	    i := s.item(2).hexadecimal_value
	    if s.item(3).is_hexadecimal_digit then
		i := i * 16 + s.item(3).hexadecimal_value
	    end
	    put_vol(i / 128)
	elseif is_note(s, 1) then
	    play_freq(note_freq_table.item(note_to_rank(s, 1)))
	end
    end

    init is
    do
	wave_table := saw_table
    end

    play_freq(d : DOUBLE) is
    do
	put_freq(d)
	put_length(1)
	put_vol(1.0)
	x := 0
    end
end
