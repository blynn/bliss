class BUZZ_SEQUENCE
creation make
feature
    machine_index : INTEGER

    eventpos : ARRAY[INTEGER]
    event : ARRAY[INTEGER]
    event_count : INTEGER

    put_eventpos(p, i : INTEGER) is
    do
	eventpos.put(p, i)
    end

    put_event(e, i : INTEGER) is
    do
	event.put(e, i)
    end

    put_event_count(i : INTEGER) is
    do
	event_count := i
	!!eventpos.make(0, event_count)
	!!event.make(0, event_count)
    end

    make(i : INTEGER) is
    do
	machine_index := i
    end
end
