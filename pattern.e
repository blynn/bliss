class PATTERN
inherit BOOMER
creation make
feature
    name : STRING
    put_name(s : STRING) is
    do
	!!name.copy(s)
    end

    commands : SPREADSHEET[STRING]

    machine : MACHINE

    make(init_name : STRING; init_machine : MACHINE) is
    require
	init_machine /= Void
	init_name /= Void
    do
	machine := init_machine
	!!commands.make
	put_name(init_name)
    ensure
	machine /= Void
	commands /= Void
    end

    boom(b : INTEGER) is
    require
	commands /= Void
    local
	it : ITERATOR[STRING]
    do
	if commands.has_row(b) then
	    it := commands.get_row(b).get_new_iterator
	    from it.start
	    until it.is_off
	    loop
		machine.process_command(it.item)
		it.next
	    end
	end
    end

    put(command : STRING; time, c : INTEGER) is
    do
	commands.put(command, time, c)
    end

    has(r, c : INTEGER) : BOOLEAN is
    do
	Result := commands.has(r, c)
    end

    at(r, c : INTEGER) : STRING is
    require
	has(r, c)
    do
	Result := commands.at(r, c)
    end

    remove(r, c : INTEGER) is
    require
	has(r, c)
    do
	commands.remove(r, c)
    end

    save(saver : SAVER) is
    local
	it : ITERATOR[INTEGER]
	it2 : ITERATOR[INTEGER]
    do
	saver.start_pattern(name)
	saver.put_machine_name(machine.name)
	it := commands.get_sorted_row_list.get_new_iterator
	from it.start
	until it.is_off
	loop
	    saver.start_row(it.item)
	    it2 := commands.get_sorted_col_list(it.item).get_new_iterator
	    from it2.start
	    until it2.is_off
	    loop
		saver.put_command(it2.item, commands.at(it.item, it2.item))
		it2.next
	    end
	    saver.end_row(it.item)
	    it.next
	end
	saver.end_pattern
    end

    instant_execute is
    local
	it, it2 : ITERATOR[INTEGER]
    do
	it := commands.get_sorted_row_list.get_new_iterator
	from it.start
	until it.is_off
	loop
	    it2 := commands.get_sorted_col_list(it.item).get_new_iterator
	    from it2.start
	    until it2.is_off
	    loop
		machine.process_command(commands.at(it.item, it2.item))
		it2.next
	    end
	    it.next
	end
    end
end
