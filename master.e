class MASTER
inherit MACHINE
creation register
feature
    id : STRING is "Master"

    boom is
    do
    end

    input_list : LINKED_LIST[SAMPLER] is
    do
	Result := in_connection
    end

    next_sample : DOUBLE is
    local
	it : ITERATOR_ON_LINKED_LIST[SAMPLER]
    do
	!!it.make(input_list)
	from
	until it.is_off
	loop
	    Result := Result + it.item.next_sample
	    it.next
	end
    end

    process_command(s : STRING) is
    do
    end

    is_sink : BOOLEAN is True
    is_source : BOOLEAN is False

    init is
    do
    end

    bgcolor : COLOR is
    do
	Result := red
    end
end
