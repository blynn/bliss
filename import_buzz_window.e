class IMPORT_BUZZ_WINDOW
inherit
    WINDOW;
    SONG_TABLE
creation make
feature
    manager : BLISS

    filename : STRING

    text_remove_last is
    do
	if not filename.is_empty then
	    filename.remove_last(1)
	end
    end

    text_append(c : CHARACTER) is
    do
	filename.add_last(c)
    end

    make(wm : BLISS) is
    do
	manager := wm
	put_geometry(20, 20, 600, 50)
	!!filename.make(128)
    end

    import_buzz_file is
    local
	bl : BUZZ_LOADER
	b : BUZZ_SONG
    do
	!!bl.make
	b := bl.load(filename)
	if b = Void then
	    io.put_string("error loading file%N")
	else
	    song.import_buzz(b)
	end
	manager.put_modal(Void)
    end

    process_event(e : EVENT) is
    do
	if e.type = sdl_keydown then
	    process_key(e)
	end
    end

    process_key(e : EVENT) is
    local
	k : INTEGER
    do
	k := e.i1
	if k >= sdlk_a and then k <= sdlk_z then
	    if is_kmod(e.kmod, kmod_shift) then
		text_append((k - 32).to_character)
	    else
		text_append(k.to_character)
	    end
	elseif k = sdlk_backspace then
	    text_remove_last
	elseif k = sdlk_escape then
	    manager.put_modal(Void)
	elseif k = sdlk_return then
	    import_buzz_file
	else --if k >= sdlk_0 and then k <= sdlk_9 then
	    text_append(k.to_character)
	end
    end

    update is
    do
	fill_rect(0, 0, width, height, white)
	fill_rect(2, 2, width - 4, height - 4, black)
	write(10, 10, "Import Buzz file: " + filename)
    end
end
