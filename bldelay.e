class BLDELAY
inherit EFFECT
creation register
feature
    id : STRING is "BLDelay"

    boom is
    do
    end

    buffer : ARRAY[DOUBLE]
    pbuf : INTEGER
    feedback : DOUBLE is 0.5
    wetout : DOUBLE is 0.375
    drythru : BOOLEAN
    length : INTEGER is 15000

    work : DOUBLE is
    local
	delay : DOUBLE
    do
	delay := buffer @ pbuf
	buffer.put(feedback * delay + mix_input, pbuf)
	pbuf := pbuf + 1
	if pbuf > length then
	    pbuf := 0
	end
	if drythru then
	    Result := wetout * delay 
	else
	    Result := mix_input + wetout * delay 
	end
    end

    buf_size : INTEGER is 32768
    init is
    do
	!!buffer.make(0, buf_size)
    end

    process_command(s : STRING) is
    do
    end
end
