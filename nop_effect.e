class NOP_EFFECT
inherit EFFECT
creation register
feature
    id : STRING is "NOP"

    boom is
    do
    end

    work : DOUBLE is
    do
	Result := mix_input
    end

    init is
    do
    end

    process_command(s : STRING) is
    do
    end
end
