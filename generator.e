deferred class GENERATOR
inherit MACHINE
feature
    is_sink : BOOLEAN is False
    is_source : BOOLEAN is True

    bgcolor : COLOR is
    do
	Result := blue
    end
end
