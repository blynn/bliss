deferred class EFFECT
inherit MACHINE
feature
    is_sink : BOOLEAN is True
    is_source : BOOLEAN is True

    bgcolor : COLOR is
    do
	Result := green
    end
end
