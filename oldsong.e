class SONG
inherit
    MACHINE_TABLE;
    BOOMER
creation make
feature
    new_machine(name : STRING; mclass : STRING) : MACHINE is
    require
	machine_class_table.has(mclass)
	not machine_table.has(name)
    local
	m : MACHINE
    do
	m := machine_class_table.at(mclass)
	Result := m.make_instance(name)
	machine_table.put(Result, name)
	machine_zorder.add_last(Result)
	machine_list.add_last(Result)
    end

    new_machine_autoname(mclass : STRING) : MACHINE is
    require
	machine_class_table.has(mclass)
    local
	s : STRING
	i : INTEGER
	m : MACHINE
    do
	m := machine_class_table.at(mclass)
	s := m.id
	if machine_table.has(s) then
	    from  i := 2
	    until not machine_table.has(s + i.to_string)
	    loop
		i := i + 1
	    end
	    s := s + i.to_string
	end
	Result := m.make_instance(s)
	machine_table.put(Result, s)
	machine_zorder.add_last(Result)
	machine_list.add_last(Result)
    end

    machine_table : DICTIONARY[MACHINE, STRING] is
    once
	!!Result.make
    end

    machine_list : LINKED_LIST[MACHINE] is
    once
	!!Result.make
    end

    machine_zorder : LINKED_LIST[MACHINE] is
    once
	!!Result.make
    end

    next_machine(m : MACHINE) : MACHINE is
    local
	i : INTEGER
    do
	i := machine_list.fast_index_of(m) + 1
	if i > machine_list.upper then
	    i := machine_list.lower
	end
	Result := machine_list @ i
    end

    patterns : SPREADSHEET[BOOMER]

    make is
    do
	!!patterns.make
    end

    rewind is
    do
	if patterns.has_row(0) then
	    pointer := patterns.get_row(0)
	    pointer_start := 0
	else
	    pointer := Void
	end
    end

    boom(b : INTEGER) is
    local
	it : ITERATOR[BOOMER]
    do
	if patterns.has_row(b) then
	    pointer := patterns.get_row(b)
	    pointer_start := b
	end
	if pointer /= Void then
	    it := pointer.get_new_iterator
	    from it.start
	    until it.is_off
	    loop
		it.item.boom(b - pointer_start)
		it.next
	    end
	end
    end

    put(boomer : BOOMER; time, c : INTEGER) is
    do
	patterns.put(boomer, time, c)
    end

feature {NONE}
    pointer : LINKED_LIST[BOOMER]
    pointer_start : INTEGER
end
