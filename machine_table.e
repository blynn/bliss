deferred class MACHINE_TABLE
feature
    machine_class_table : DICTIONARY[MACHINE, STRING] is
    local
	m : MACHINE
    once
	!!Result.make
	!BLDRUM!m.register(Result)
	!BLBASS2!m.register(Result)
	!MASTER!m.register(Result)
	!NOP_EFFECT!m.register(Result)
	!BLTRACKER!m.register(Result)
    end
end
