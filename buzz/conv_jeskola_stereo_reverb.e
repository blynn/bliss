class CONV_JESKOLA_STEREO_REVERB
inherit CONV
creation register
feature
    buzz_dll : STRING is "Jeskola Stereo Reverb"
    bliss_id : STRING is "NOP"
    global_parm_count : INTEGER is 10
    track_parm_count : INTEGER is 0

    convert_patterns(m : MACHINE; bm : BUZZ_MACHINE) is
    do
    end
end
