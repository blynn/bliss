deferred class GENERATOR
inherit MACHINE
feature
    is_sink : BOOLEAN is False
    is_source : BOOLEAN is True

    bgcolor : COLOR is
    do
	Result := blue
    end

    --init_inmachine(m : MACHINE) is
    --do
    --end
end
