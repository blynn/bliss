class BUZZ_CONVERTER
inherit CONV_TABLE
creation make
feature
    make is
    do
    end

    convert(song : SONG; buzz : BUZZ_SONG) is
    local
	i, j : INTEGER
	bm : BUZZ_MACHINE
	m : MACHINE
	x, y : INTEGER
	bc : BUZZ_CONNECTION
	seq : BUZZ_SEQUENCE
	m_array : ARRAY[MACHINE]
	t : TRACK
	c : CONV
    do
	--convert machines
	!!m_array.make(0, buzz.machine_count - 1)
	from i := 0
	until i = buzz.machine_count
	loop
	    bm := buzz.machine.item(i)
	    if bm.type = 0 then
		m := song.master
	    else
		c := conv_table.at(bm.dll)
		m := song.new_machine(bm.name, c.bliss_id)
	    end
	    m_array.put(m, i)
	    x := (song.layoutx * (bm.x + 1) / 2).rounded
	    y := (song.layouty * (bm.y + 1) / 2).rounded
	    song.move_to(m, x, y)
	    i := i + 1
	end

	--convert connections
	from i := 0
	until i = buzz.connection_count
	loop
	    bc := buzz.connection.item(i)
	    song.connect(m_array.item(bc.src), m_array.item(bc.dst))
	    i := i + 1
	end

	--convert machine patterns
	from i := 1
	    --start from 1: master = 0 has to be handled separately
	until i = buzz.machine_count
	loop
	    bm := buzz.machine.item(i)
	    c := conv_table.at(bm.dll)
	    c.convert_patterns(m_array.item(i), bm)
	    i := i + 1
	end

	--convert sequence
	song.put_end_beat(buzz.song_end)
	song.put_loop_begin(buzz.loop_begin)
	song.put_loop_end(buzz.loop_end)

	from i := 0
	until i = buzz.sequence_count
	loop
	    seq := buzz.sequence.item(i)
	    !!t.make(m_array.item(seq.machine_index))
	    bm := buzz.machine.item(seq.machine_index)
	    song.tracks.add_last(t)
	    from j := 0
	    until j = seq.event_count
	    loop
		x := seq.event.item(j) - 16
		t.put(bm.pattern.item(x).name, seq.eventpos.item(j))
		j := j + 1
	    end
	    i := i + 1
	end

	convert_waves(song, buzz)
    end

    convert_waves(song : SONG; buzz : BUZZ_SONG) is
    local
	i : INTEGER
	bw : BUZZ_WAVETABLE_ENTRY
	bl : BUZZ_LEVEL
	w : WAVE
    do
	--convert waves
	from i := 0
	until i = buzz.wave_count
	loop
	    bw := buzz.wavetable.item(i)
	    bl := bw.level_table.first
	    !!w.make(bw.name)
	    w.put_volume(bw.volume)
	    w.set_data(bl.data)
	    w.put_length(bl.length)
	    w.put_root_freq(bl.root_freq)
	    song.put_wave(w, i + 1)
io.put_string("converting " + w.name + ": put at " + (i + 1).to_string + "%N")
io.put_string("index = " + bw.index.to_string + "%N")
io.put_string("volume = " + bw.volume.to_string + "%N")
	    i := i + 1
	end
    end
end
