class NOP_EFFECT
inherit EFFECT
creation register
feature
    id : STRING is "NOP"

    boom is
    do
    end

    next_sample : DOUBLE is
    local
	it : ITERATOR_ON_LINKED_LIST[SAMPLER]
    do
	!!it.make(in_connection)
	from
	until it.is_off
	loop
	    Result := Result + it.item.next_sample
	    it.next
	end
    end

    init is
    do
    end

    process_command(s : STRING) is
    do
    end
end
