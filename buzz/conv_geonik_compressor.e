class CONV_GEONIK_COMPRESSOR
inherit CONV
creation register
feature
    buzz_dll : STRING is "Geonik's Compressor"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 7
    track_parm_count : INTEGER is 0

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
