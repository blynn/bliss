class EVENT
creation make
feature
	x, y : INTEGER
	desc : STRING
	i1, i2 : INTEGER
	kmod : INTEGER

	make is
	do
	end

	put_x(x0 : INTEGER) is
	do
		x := x0
	end

	put_y(y0 : INTEGER) is
	do
		y := y0
	end

	put_desc(s : STRING) is
	do
		desc := s
	end

	desc_from_external(p : POINTER) is
	do
		!!desc.from_external_copy(p)
	end

	put_i1(i : INTEGER) is
	do
		i1 := i
	end

	put_i2(i : INTEGER) is
	do
		i2 := i
	end

	put_kmod(i : INTEGER) is
	do
		kmod := i
	end
	
	update is
	do
		put_x(get_current_x)
		put_y(get_current_y)
		put_i1(get_current_i1)
		put_i2(get_current_i2)
		put_kmod(get_current_kmod)
		desc_from_external(get_current_desc)
	end

	get_current_x : INTEGER is
	external "C" alias "export_current_x"
	end

	get_current_y : INTEGER is
	external "C" alias "export_current_y"
	end

	get_current_i1 : INTEGER is
	external "C" alias "export_current_i1"
	end

	get_current_i2 : INTEGER is
	external "C" alias "export_current_i2"
	end

	get_current_kmod : INTEGER is
	external "C" alias "export_current_kmod"
	end

	get_current_desc : POINTER is
	external "C" alias "export_current_desc"
	end
end
