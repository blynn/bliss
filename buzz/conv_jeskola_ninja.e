class CONV_JESKOLA_NINJA
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola NiNjA dElaY"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 1
    track_parm_count : INTEGER is 10

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
