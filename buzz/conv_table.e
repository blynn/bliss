deferred class CONV_TABLE
feature
    conv_table : DICTIONARY[CONV, STRING] is
    local
	c : CONV
    once
	!!Result.make
	!CONV_JESKOLA_BASS2!c.register(Result)
	!CONV_JESKOLA_EQ3!c.register(Result)
	!CONV_JESKOLA_REVERB!c.register(Result)
	!CONV_JESKOLA_TRACKER!c.register(Result)
	!CONV_JESKOLA_DELAY!c.register(Result)
	!CONV_JESKOLA_DISTORTION!c.register(Result)
	!CONV_JESKOLA_TRILOK!c.register(Result)
	!CONV_JESKOLA_SHAPER!c.register(Result)
	!CONV_JESKOLA_FLANGER!c.register(Result)
	!CONV_JESKOLA_NINJA!c.register(Result)
	!CONV_JESKOLA_NOISE!c.register(Result)
	!CONV_JESKOLA_FILTER!c.register(Result)
	!CONV_JESKOLA_STEREO_REVERB!c.register(Result)
	!CONV_GEONIK_COMPRESSOR!c.register(Result)
	!CONV_GEONIK_PRIMIFUN!c.register(Result)
	!CONV_GEONIK_EXPRESSION!c.register(Result)
    end
end
