class BUZZ_CONNECTION
creation make
feature
    src : INTEGER
    dst : INTEGER
    amp : INTEGER
    pan : INTEGER

    make(s, d, a, p : INTEGER) is
    do
	src := s
	dst := d
	amp := a
	pan := p
    end
end
