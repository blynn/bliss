class EDGE
inherit SAMPLER
creation make
feature
    src, dst : MACHINE
    amp : DOUBLE is 1.0
    pan : INTEGER

    make(m1, m2 : MACHINE) is
    require
	m1.is_source
	m2.is_sink
    do
	src := m1
	dst := m2
    end

    next_sample : DOUBLE is
    do
	Result := amp * src.next_sample
    end

    x, y : INTEGER
    put_xy(new_x, new_y : INTEGER) is
    do
	x := new_x
	y := new_y
    end
end
