class CONV_JESKOLA_DELAY
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Delay"
    bliss_id : STRING is "BLDelay"
    global_parm_count : INTEGER is 1
    track_parm_count : INTEGER is 5

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
