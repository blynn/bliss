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
    end
end
