deferred class COLOR_TABLE
feature
    black : COLOR is
    once
	!!Result.make(0, 0, 0)
    end

    white : COLOR is
    once
	!!Result.make(255, 255, 255)
    end

    red : COLOR is
    once
	!!Result.make(127, 0, 0)
    end

    blue : COLOR is
    once
	!!Result.make(0, 0, 127)
    end

    green : COLOR is
    once
	!!Result.make(0, 127, 0)
    end
end
