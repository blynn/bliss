class BLISS
inherit
    SONG_TABLE;
    SDL_CONSTANT;
    COLOR_TABLE
creation make
feature
    --status_bar : STATUS_BAR
    pattern_window : PATTERN_WINDOW
    machine_window : MACHINE_WINDOW
    song_window : SONG_WINDOW

    import_buzz_window : IMPORT_BUZZ_WINDOW

    modal : WINDOW
    put_modal(w : WINDOW) is
    do
	modal := w
    end

    load_buzz_dialog is
    do
	modal := import_buzz_window
    end

    comp_test is
    local
	a, b : COMPLEX
    do
	a.put(5, 2)
	b.put(3, 3)
	io.put_string((a-b).to_string + "%N")
	io.put_string((a*b).to_string + "%N")
	io.put_string((a/b).to_string + "%N")
	io.put_string((b*(a/b)).to_string + "%N")
	io.put_string(b.theta.to_string + "%N")
	io.put_string(b.div_double(3.).r.to_string + "%N")
	io.put_string(b.theta.to_string + "%N")
    end

    make is
    do
	vis_upper := 256
	!!vis.make(1, vis_upper)
	!!vis_snap.make(1, vis_upper)
	vis_i := 1

	ext_init
	!!last_event.make

	song.put_bpm_tpb(150, 4)

	!!machine_window.make(1, 21, 638, 438)
	!!pattern_window.make(1, 21, 638, 438)
	!!song_window.make(1, 21, 638, 438)
	!!import_buzz_window.make(Current)
	--!!status_bar.make(0, 460, 640, 480)

	pattern_window.put_machine(song.master)

	current_window := machine_window

	pause_audio(0)

	main_loop
    end

    end_program is
    do
	pause_audio(1)
	die_with_code(0)
    end

    vis_snap : ARRAY[DOUBLE]
    vis : ARRAY[DOUBLE]
    vis_upper : INTEGER
    vis_i : INTEGER

    vis_draw is
    local
	i : INTEGER
	x, y : INTEGER
    do
	from i := 1
	until i > vis_upper
	loop
	    y := 100 - (vis_snap.item(i) * 50).rounded
	    x := i + 100
	    pixelcolor(x, y, white)
	    i := i + 1
	end
    end

    buffer_audio is
    local
	d : DOUBLE
    do
	from
	until not audio_buffer_ready
	loop
	    d := song.next_sample
	    if vis_i > vis_upper then
		from vis_i := 1
		until vis_i > vis_upper
		loop
		    vis_snap.put(vis @ vis_i, vis_i)
		    vis_i := vis_i + 1
		end
		vis_i := 1
	    end
	    vis.put(d, vis_i)
	    vis_i := vis_i + 1
	    next_frame(d, d)
	end
    end

    last_event : EVENT

    current_window : WINDOW

    main_loop is
    do
	from
	until False
	loop
	    draw_screen
	    ext_update_screen
	    lock_audio
	    buffer_audio
	    unlock_audio
	    if poll_event then
		process_last_event
	    end
	end
    end

    poll_event : BOOLEAN is
    do
	Result := ext_poll_event
	if Result then
	    last_event.update
	end
    end

    screenx : INTEGER is 640
    screeny : INTEGER is 480

    draw_screen is
    do
	blank_screen
	vis_draw
	current_window.update
	if modal /= Void then
	    modal.update
	end
    end

    blank_screen is
    do
	fill_rect(0, 0, screenx, screeny, black)
    end

    process_last_event is
    do
	io.put_string(last_event.desc + ": ")
	io.put_integer(last_event.i1)
	io.put_new_line
	if modal /= Void then
	    modal.handle_event(last_event)
	else
	
	if last_event.desc.is_equal("keydown") then
if is_kmod(last_event.kmod, kmod_ctrl) then
    inspect last_event.i1
    when sdlk_s then
	song.save(test_saver)
    when sdlk_l then
	load_buzz_dialog
    when sdlk_n then
	song.make
    else
	current_window.handle_event(last_event)
    end
else
	    inspect last_event.i1
	    when sdlk_f2 then
		current_window := pattern_window
	    when sdlk_f3 then
		current_window := machine_window
	    when sdlk_f4 then
		current_window := song_window
	    when sdlk_escape then
		end_program
	    when sdlk_f5 then
		song.play
	    when sdlk_f6 then
		song.rewind
	    when sdlk_f8 then
		song.pause
	    else
		current_window.handle_event(last_event)
	    end
end
	else
	    current_window.handle_event(last_event)
	end

	end
    end

    test_saver : SAVER is
    once
	!!Result.make
    end

    ext_init is
    external "C" alias "init"
    end

    next_frame(l : DOUBLE; r : DOUBLE) is
    external "C"
    end

    audio_buffer_ready : BOOLEAN is
    external "C" alias "buffer_ready"
    end

    lock_audio is
    external "C" alias "SDL_LockAudio"
    end

    unlock_audio is
    external "C" alias "SDL_UnlockAudio"
    end

    pause_audio(i : INTEGER) is
    external "C" alias "SDL_PauseAudio"
    end

    ext_poll_event : BOOLEAN is
    external "C" alias "ext_poll_event"
    end

    ext_start_drawing is
    external "C"
    end

    ext_update_screen is
    external "C"
    end

    xoffset : INTEGER
    yoffset : INTEGER
    write(x, y : INTEGER; s : STRING) is
    do
	render_text(xoffset + x, yoffset + y, s.to_external)
    end

    render_text(x, y : INTEGER; s : POINTER) is
    external "C"
    end

    fill_rect(x, y, w, h : INTEGER; c : COLOR) is
    do
	ext_fill_rect(x, y, w, h, c.to_integer)
    end

    ext_fill_rect(x, y, w, h, c : INTEGER) is
    external "C"
    end

    pixelcolor(x, y : INTEGER; c : COLOR) is
    do
	ext_pixelcolor(x, y, c.to_gfx_integer)
    end

    ext_pixelcolor(x, y, c : INTEGER) is
    external "C"
    end

    aalinecolor(x1, y1, x2, y2 : INTEGER; c : COLOR) is
    do
	ext_aalinecolor(x1, y1, x2, y2, c.to_gfx_integer)
    end

    ext_aalinecolor(x1, y1, x2, y2, c : INTEGER) is
    external "C"
    end
end
