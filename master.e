class MASTER
inherit MACHINE
creation register
feature
    init is
    do
    end

    id : STRING is "Master"

    boom is
    do
    end

    work : DOUBLE is
    do
	Result := mix_input
    end

    process_command(s : STRING) is
    do
    end

    is_sink : BOOLEAN is True
    is_source : BOOLEAN is False

    bgcolor : COLOR is
    do
	Result := red
    end
end
