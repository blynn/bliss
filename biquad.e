class BIQUAD
creation make
feature
    make is
    do
	!!outcoeff.make(1, 2)
	!!incoeff.make(0, 2)
    end

    incoeff : ARRAY[DOUBLE]
    outcoeff : ARRAY[DOUBLE]
    inhistory1 : DOUBLE
    inhistory2 : DOUBLE
    outhistory1 : DOUBLE
    outhistory2 : DOUBLE

    clear_history is
    do
	inhistory1 := 0
	inhistory2 := 0
	outhistory1 := 0
	outhistory2 := 0
    end

    put_outcoeff(d : DOUBLE; i : INTEGER) is
    require
	i >= 1
	i <= 2
    do
	outcoeff.put(d, i)
--io.put_string("output coeff " + i.to_string + " is " + d.to_string + "%N")
    end

    put_incoeff(d : DOUBLE; i : INTEGER) is
    require
	i >= 0
	i <= 2
    do
	incoeff.put(d, i)
--io.put_string(" input coeff " + i.to_string + " is " + d.to_string + "%N")
    end

    filter(input : DOUBLE) : DOUBLE is
    do
	Result := incoeff.item(0) * input
	Result := Result + incoeff.item(1) * inhistory1
	Result := Result + incoeff.item(2) * inhistory2
	Result := Result + outcoeff.item(1) * outhistory1
	Result := Result + outcoeff.item(2) * outhistory2
	inhistory2 := inhistory1
	inhistory1 := input
	outhistory2 := outhistory1
	outhistory1 := Result
    end
end
