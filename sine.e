class SINE
inherit PERIODIC_FUNCTION
creation make
feature
	pi2 : DOUBLE is 6.283185307

	make is
	do
	end

	evaluate(t : DOUBLE) : DOUBLE is
	do
		Result := (t * pi2).sin
	end
end
