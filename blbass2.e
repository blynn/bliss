class BLBASS2
inherit GENERATOR
creation register
feature
    pi : DOUBLE is
    once
	Result := 4.0 * (1.0).atan
    end

    boom is
    do
    end

    id : STRING is "BLBass2"

    max_resonance : INTEGER is 1000
    recipq : DOUBLE

    put_resonance(i : INTEGER) is
    require
	i <= max_resonance
    do
	recipq := 1. - (i / 256)
	--recipq := 1. / (i + 1)
    end

    pole_count : INTEGER is 2

    pole_sum : ARRAY[DOUBLE] is
    local
	i : INTEGER
    once
	!!Result.make(1, pole_count)
	from i := 1
	until i > pole_count
	loop
	    Result.put(2. * ((2 * i - 1) * pi / (4 * pole_count)).cos, i)
	    i := i + 1
	end
    end

    fs : DOUBLE is 44100.0

    gain : DOUBLE
    coeffin : ARRAY2[DOUBLE]
    coeffout : ARRAY2[DOUBLE]

    cutoff : DOUBLE
    put_cutoff(i : INTEGER) is
    do
	cutoff := (i + 1) / 129
    end

    compute_taps is
    local
	i : INTEGER
    do
	gain := 1.
	from i := 1
	until i > pole_count
	loop
	    compute_tap(i)
	    i := i + 1
	end
    end

    compute_tap(i : INTEGER) is
    local
	b1 : DOUBLE
	b2 : DOUBLE
	ad, bd : DOUBLE
    do
	b2 :=   1. / (pi * cutoff).tan
	b1 := pole_sum.item(i) * recipq * b2
	b2 := b2 * b2

	ad := 1.
	bd := b2 + b1 + 1.

	gain := gain * ad / bd

	coeffout.put((2. - 2. * b2) / bd, i, 1)
	coeffout.put((b2 - b1 + 1.) / bd, i, 2)
	coeffin.put(2., i, 1)
	coeffin.put(1., i, 2)
    if False then
	b2 := 1. / ((2. * fs) * (pi * cutoff).tan)
	b1 := pole_sum.item(i) * recipq * b2
	b2 := b2 * b2

	ad := 1.
	bd := 4. * b2 * fs * fs + 2. * b1 * fs + 1.

	gain := gain * ad / bd
	coeffout.put((2. - 8. * b2 * fs * fs) / bd, i, 1)
	coeffout.put((4. * b2 * fs * fs - 2. * b1 * fs + 1.) / bd, i, 2)
	coeffin.put(2., i, 1)
	coeffin.put(1., i, 2)
    end
    end

    historyin : ARRAY2[DOUBLE]
    historyout : ARRAY2[DOUBLE]

    lpfilter(input : DOUBLE) : DOUBLE is
    local
	i : INTEGER
	temp_history : DOUBLE
    do
	Result := input * gain
	from i := 1
	until i > pole_count
	loop
	    Result := Result - historyout.item(i, 1) * coeffout.item(i, 1)
	    temp_history := Result - historyout.item(i, 2) * coeffout.item(i, 2)
	    --WRONG?
	    Result := temp_history + historyout.item(i, 1) * coeffin.item(i, 1)
	    Result := Result + historyout.item(i, 2) * coeffin.item(i, 2)
	    historyout.put(historyout.item(i, 1), i, 2)
	    historyout.put(temp_history, i, 1)
	    i := i + 1
	end
    end

    resolution : INTEGER is 1024

    sin_table : ARRAY[DOUBLE] is
    local
	i : INTEGER
    once
	!!Result.make(0, resolution - 1)
	from i := 0
	until i = resolution
	loop
	    Result.put(0.5 * ((i / resolution) * pi * 2.0).sin, i)
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

    y1 : DOUBLE

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

    work : DOUBLE is
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

	    Result := lpfilter(Result)
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
	elseif s.item(1) = 'c' then
	    i := s.item(2).hexadecimal_value
	    if s.item(3).is_hexadecimal_digit then
		i := i * 16 + s.item(3).hexadecimal_value
	    end
	    put_cutoff(i)
	    compute_taps
	elseif s.item(1) = 'r' then
	    i := s.item(2).hexadecimal_value
	    if s.item(3).is_hexadecimal_digit then
		i := i * 16 + s.item(3).hexadecimal_value
	    end
	    put_resonance(i)
	    compute_taps
	elseif is_note(s, 1) then
	    play_freq(note_freq_table.item(note_to_rank(s, 1)))
	end
    end

    init is
    do
	wave_table := saw_table
	!!coeffin.make(1, pole_count, 1, 2)
	!!coeffout.make(1, pole_count, 1, 2)
	!!historyin.make(1, pole_count, 1, 2)
	!!historyout.make(1, pole_count, 1, 2)
	put_cutoff(128)
	put_resonance(0)
	compute_taps
    end

    play_freq(d : DOUBLE) is
    do
	put_freq(d)
	put_length(1)
	put_vol(1.0)
	x := 0
    end
end
