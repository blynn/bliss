class SAVER
creation make
feature
    make is
    do
	os := io
    end

    os : OUTPUT_STREAM

    tabs : INTEGER

    start_pattern(id : STRING) is
    do
	put_line("");
	put_line("start pattern")
	tabs := tabs + 1
	put_line("name %"" + id + "%"")
    end

    end_pattern is
    do
	tabs := tabs - 1
	put_line("end")
    end

    start_row(r : INTEGER) is
    do
	put_line("row " + r.to_string)
	tabs := tabs + 1
    end

    end_row(r : INTEGER) is
    do
	tabs := tabs - 1
	put_line("end")
    end

    start_zorder is
    do
	put_line("");
	put_line("start zorder")
	tabs := tabs + 1
    end

    end_zorder is
    do
	tabs := tabs - 1
	put_line("end")
    end

    start_machine(name : STRING) is
    do
	put_line("");
	put_line("start machine")
	tabs := tabs + 1
	put_line("name %"" + name + "%"")
    end

    end_machine is
    do
	tabs := tabs - 1
	put_line("end")
    end

    put_machine_class(id : STRING) is
    do
	put_line("class %"" + id + "%"")
    end

    put_machine_pos(x, y : INTEGER) is
    do
	put_line("x " + x.to_string)
	put_line("y " + y.to_string)
    end

    put_command(c : INTEGER; command : STRING) is
    do
	put_line(c.to_string + " " + command)
    end

    put_machine_name(id : STRING) is
    do
	put_line("machine %"" + id +"%"")
    end

    put_line(s : STRING) is
    local
	i : INTEGER
    do
	from i := tabs
	until i = 0
	loop
	    os.put_string("    ")
	    i := i - 1
	end
	os.put_string(s)
	os.put_new_line
    end
end
