class CURVER
inherit SAMPLER
creation make
feature
	x0, x1 : DOUBLE
	diff : DOUBLE
	original_sign : INTEGER

	make(start, finish : DOUBLE) is
	do
		x0 := start
		x1 := finish
		diff := x1 - x0
		original_sign := diff.sign
	end

	t : DOUBLE
	
	next_sample : DOUBLE is
	do
		t := t + (1.0 / 44100.0)
		--Result := 330.0 - (330.0 * (t.pow(0.1)))
		Result := x0 + diff * t.pow(0.1)
		if (x1 - Result).sign /= original_sign then
			Result := x1
		end
	end

	reset is
	do
		t := 0.0
	end
end
