class CONV_JESKOLA_EQ3
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola EQ-3"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 11
    track_parm_count : INTEGER is 0

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
