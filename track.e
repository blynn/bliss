class TRACK
inherit BOOMER
creation make
feature
    machine : MACHINE
    commands : DICTIONARY[STRING, INTEGER]

    make(init_machine : MACHINE) is
    require
	init_machine /= Void
    do
	machine := init_machine
	!!commands.make
    end

    last_b : INTEGER

    rewind is
    do
	offset := 0
	playing_pattern := Void
    end

    boom(b : INTEGER) is
    do
	if commands.has(b) then
	    offset := b
	    process_command(commands.at(b))
	end
	if playing_pattern /= Void then
	    playing_pattern.boom(b - offset)
	end
    end

    process_command(s : STRING) is
    do
	if machine.pattern_table.has(s) then
	    playing_pattern := machine.pattern_table.at(s)
	else
	    playing_pattern := Void
	end
    end

    put(cmd : STRING; b : INTEGER) is
    do
	commands.put(cmd, b)
    end

    playing_pattern : PATTERN
    offset : INTEGER
end
