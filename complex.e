expanded class COMPLEX
feature
    real : DOUBLE
    imaginary : DOUBLE
    pi : DOUBLE is
    once
	Result := 4. * (1.).atan
    end

    r : DOUBLE is
    do
	Result := (real * real + imaginary * imaginary).sqrt
    end

    theta : DOUBLE is
    do
	if real = 0. then
	    Result := pi / 2.
	else
	    Result := (imaginary / real).atan
	end
	if imaginary < 0 then
	    Result := Result + pi
	end
    end

    from_double(a : DOUBLE) is
    do
	real := a
	imaginary := 0
    end

    mul_double(a : DOUBLE) : COMPLEX is
    do
	Result.put(real * a, imaginary * a)
    end

    div_double(a : DOUBLE) : COMPLEX is
    local
	t : DOUBLE
    do
	t := 1. / a
	Result := mul_double(t)
    end

    put(a, b : DOUBLE) is
    do
	real := a
	imaginary := b
    end

    infix "+" (other: COMPLEX): COMPLEX is
    do
	Result.put(real + other.real, imaginary + other.imaginary)
    end

    infix "-" (other: COMPLEX): COMPLEX is
    do
	Result.put(real - other.real, imaginary - other.imaginary)
    end

    infix "*" (other: COMPLEX): COMPLEX is
    do
	Result.put(real * other.real - imaginary * other.imaginary,
		real * other.imaginary + other.real * imaginary)
    end

    infix "/" (other: COMPLEX): COMPLEX is
    local
	d : DOUBLE
    do
	d := 1. / (other.real * other.real + other.imaginary * other.imaginary)
	Result.put(d * (real * other.real + imaginary * other.imaginary),
		d * (other.real * imaginary - real * other.imaginary))
    end

    to_string : STRING is
    do
	Result := real.to_string + " + " + imaginary.to_string + "i"
    end
end
