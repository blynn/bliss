class SPREADSHEET[E]
creation make
feature
    make is
    do
	!!table.make
    end

    table : DICTIONARY[DICTIONARY[E, INTEGER], INTEGER]

    has_row(r : INTEGER) : BOOLEAN is
    do
	if table.has(r) then
	    Result := True
	end
    end

    get_row(r : INTEGER) : LINKED_LIST[E] is
    require
	has_row(r)
    local
	it : ITERATOR[INTEGER]
	t : DICTIONARY[E, INTEGER]
    do
	!!Result.make
	t := table.at(r)
	it := get_sorted_col_list(r).get_new_iterator
	from it.start
	until it.is_off
	loop
	    Result.add_last(t.at(it.item))
	    it.next
	end
    end

    has(r, c : INTEGER) : BOOLEAN is
    do
	if table.has(r) then
	    Result := table.at(r).has(c)
	end
    end

    at(r, c : INTEGER) : E is
    require
	has(r, c)
    do
	Result := table.at(r).at(c)
    end

    put(e : E; r, c : INTEGER) is
    local
	t : DICTIONARY[E, INTEGER]
    do
	if table.has(r) then
	    t := table.at(r)
	else
	    !!t.make
	    table.put(t, r)
	end
	t.put(e, c)
    end

    remove(r, c : INTEGER) is
    require
	has(r, c)
    do
	table.at(r).remove(c)
	if table.at(r).is_empty then
	    table.remove(r)
	end
    end

    sorter : COLLECTION_SORTER[INTEGER]

    get_sorted_row_list : LINKED_LIST[INTEGER] is
    local
	it : ITERATOR[INTEGER]
    do
	!!Result.make
	it := table.get_new_iterator_on_keys
	from it.start
	until it.is_off
	loop
	    Result.add_last(it.item)
	    it.next
	end
	sorter.sort(Result)
    end

    get_sorted_col_list(r : INTEGER) : LINKED_LIST[INTEGER] is
    require
	has_row(r)
    local
	it : ITERATOR[INTEGER]
	t : DICTIONARY[E, INTEGER]
    do
	!!Result.make
	t := table.at(r)
	it := t.get_new_iterator_on_keys
	from it.start
	until it.is_off
	loop
	    Result.add_last(it.item)
	    it.next
	end
	sorter.sort(Result)
    end
end
