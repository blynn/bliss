class CONV_JESKOLA_NOISE
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Noise Generator"
    bliss_id : STRING is "ZERO"
    global_parm_count : INTEGER is 0
    track_parm_count : INTEGER is 10

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
