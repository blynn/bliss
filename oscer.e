class OSCER
inherit
	SAMPLER
creation make
feature
	reset is
	do
		t := 0.0
	end
	
	t : DOUBLE
	freq : DOUBLE
	periodicf : PERIODIC_FUNCTION

	put_freq(f : DOUBLE) is
	do
		freq := f
	end

	make(pf : PERIODIC_FUNCTION) is
	do
		periodicf := pf
	end
	
	next_sample : DOUBLE is
	do
		next_t
		Result := periodicf.evaluate(t)
	end

	next_t is
	do
		t := fast_next(t, freq)
	end

	fast_next(a : DOUBLE; b : DOUBLE) : DOUBLE is
	external "C"
	end
end
