class PORTABLE_FILE_READ
creation connect_to
feature
    is_connected : BOOLEAN

    connect_to(f : STRING) is
    do
	ptr := wrap_fopen(f.to_external, ("rb").to_external)
	if ptr.is_not_null then
	    is_connected := True
	end
    end

    disconnect is
    require
	is_connected
    do
	wrap_fclose(ptr)
    end

    last_character : CHARACTER

    read_character is
    require
	not end_of_input
    do
	last_character := wrap_fgetc(ptr)
    end

    ptr : POINTER

    end_of_input : BOOLEAN is
    require
	is_connected
    do
	Result := wrap_feof(ptr)
    end

    wrap_fopen(f, m : POINTER) : POINTER is
    external "C"
    end

    wrap_fclose(fp : POINTER) is
    external "C"
    end

    wrap_feof(fp : POINTER) : BOOLEAN is
    external "C"
    end

    wrap_fgetc(fp : POINTER) : CHARACTER is
    external "C"
    end
end
