class WAVE
creation make
feature
    root_freq : DOUBLE
    --TODO: store it's reciprocal
    put_root_freq(d : DOUBLE) is
    do
	root_freq := d
    end

    volume : DOUBLE
    put_volume(d : DOUBLE) is
    do
	volume := d
    end

    length : INTEGER

    put_length(l : INTEGER) is
    do
	length := l
    end

    data : ARRAY[CHARACTER]

    set_data(d : like data) is
    do
	data := d
    end

    name : STRING

    put_name(n : STRING) is
    do
	!!name.copy(n)
    end

    make(n : STRING) is
    do
	put_name(n)
    end

    get_data_double(i : INTEGER) : DOUBLE is
    local
	n : INTEGER
    do
	n := data.item(2 * i).code + 256 * data.item(2 * i + 1).code
	if n >= 32768 then
	    n := n - 65536
	end
	Result := n / 65536 * volume
    end
end
