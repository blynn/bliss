class CONV_JESKOLA_SHAPER
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Wave Shaper"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 0
    track_parm_count : INTEGER is 3

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
