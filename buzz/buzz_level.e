class BUZZ_LEVEL
creation make
feature
    length : INTEGER
    root_freq : DOUBLE
    put_root_freq(d : DOUBLE) is
    do
	root_freq := d
    end

    make(n : INTEGER) is
    do
	length := n
    end

    data : ARRAY[CHARACTER]
    init_data(n : INTEGER) is
    do
	!!data.make(0, n - 1)
    end
end
