class CONV_JESKOLA_TRILOK
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Trilok"
    bliss_id : STRING is "ZERO"
    global_parm_count : INTEGER is 4
    track_parm_count : INTEGER is 0

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
