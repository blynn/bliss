class RING_MOD
inherit SAMPLER
creation make
feature
	input1 : SAMPLER
	input2 : SAMPLER

	make(t1, t2 : SAMPLER) is
	do
		put_input1(t1)
		put_input2(t2)
	end

	put_input1(t : SAMPLER) is
	do
		input1 := t
	end

	put_input2(t : SAMPLER) is
	do
		input2 := t
	end

	next_sample : DOUBLE is
	do
		Result := input1.next_sample * input2.next_sample
	end
end
