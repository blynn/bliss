class FREQ_MOD
inherit SAMPLER
creation make
feature
	input : OSCER
	modulator : SAMPLER

	make is
	do
	end

	put_input(o : OSCER) is
	do
		input := o
	end

	put_modulator(t : SAMPLER) is
	do
		modulator := t
	end

	next_sample : DOUBLE is
	do
		input.put_freq(modulator.next_sample)
		Result := input.next_sample
	end
end
