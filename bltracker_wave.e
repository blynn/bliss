class BLTRACKER_WAVE
inherit SONG_TABLE
creation make
feature
    make is
    do
    end

    precision : INTEGER is 1024

    put_vol(d : DOUBLE) is
    do
	vol := d
    end

    d_cache : DOUBLE
    put_freq(d : DOUBLE) is
    do
	d_cache := d
    end

    vol : DOUBLE

    freq : INTEGER

    wave : WAVE

    wave_index : INTEGER

    wave_length : INTEGER

    modulus : INTEGER

    playing : BOOLEAN

    put_wave_index(i : INTEGER) is
    do
	wave_index := i
	wave := song.wave_table.at(wave_index)
	wave_length := wave.length
	modulus := wave_length * precision
	freq := (precision * d_cache / wave.root_freq).rounded
--io.put_string("freq = " + freq.to_string + "%N")
    end

    play_freq(d : DOUBLE) is
    do
	playing := True
	put_freq(d)
	put_vol(1.0)
	volslide := 0.0
	x := 0
    end

    x : INTEGER

    next_sample : DOUBLE is
    do
	if playing then
	    Result := wave.get_data_double(x // precision) * vol
	    vol := vol + volslide
	    if vol < 0.0 then vol := 0.0 end
	    x := x + freq
	    if x >= modulus then
		playing := False
	    end
	end
    end

    volslide : DOUBLE
    volslidedown(i : INTEGER) is
    do
	volslide := -i / (256 * 512)
    end
end
