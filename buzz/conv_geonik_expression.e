class CONV_GEONIK_EXPRESSION
inherit CONV
creation register
feature
    buzz_dll : STRING is "Geonik's Expression"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 4
    track_parm_count : INTEGER is 0

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
