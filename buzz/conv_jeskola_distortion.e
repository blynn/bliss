class CONV_JESKOLA_DISTORTION
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Distortion"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 9
    track_parm_count : INTEGER is 0

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
