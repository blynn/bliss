class NOISE
inherit SAMPLER
creation make
feature
	t : DOUBLE
	t_max : DOUBLE
	sr : STD_RAND
	current_value : DOUBLE

	put_freq(f : DOUBLE) is
	do
		t_max := 44100 / f
	end

	make is
	do
		!!sr.make
		next_value
	end
	
	next_sample : DOUBLE is
	do
		next_t
		Result := current_value
	end

	next_t is
	do
		if t_max > 0.0 then
			t := t + 1.0
			if t > t_max then
				next_value
				t := t - t_max
			end
		end
	end

	next_value is
	local
		i : INTEGER
		n : INTEGER
	do
		n := 10
		current_value := 0
		from i := 0
		until i = n
		loop
			sr.next
			current_value := current_value + sr.last_double
			i := i + 1
		end
		current_value := (2 * current_value) / n - 1.0
	end
end
