class CONV_GEONIK_PRIMIFUN
inherit CONV
creation register
feature
    buzz_dll : STRING is "Geonik's PrimiFun"
    bliss_id : STRING is "ZERO"
    global_parm_count : INTEGER is 0
    track_parm_count : INTEGER is 9

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
