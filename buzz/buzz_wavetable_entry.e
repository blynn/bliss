class BUZZ_WAVETABLE_ENTRY
creation make
feature
    index : INTEGER
    make(i : INTEGER) is
    do
	index := i
    end

    volume : DOUBLE
    put_volume(d : DOUBLE) is
    do
	volume := d
    end

    filename : STRING
    put_filename(fname : STRING) is
    do
	!!filename.copy(fname)
    end

    name : STRING
    put_name(n : STRING) is
    do
	!!name.copy(n)
    end

    level_table : ARRAY[BUZZ_LEVEL]
    level_count : INTEGER
    put_level_count(i : INTEGER) is
    do
	level_count := i
	!!level_table.make(0, level_count - 1)
    end
end
