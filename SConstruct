import os

platform = 'linux'
program = 'bliss'
program_version = '0.1.3'

sample_files = Split('demo.bl shepard.bl stomper.bl pluck.bl dist.bl')

units = Split("""
    out.c
    dummy.c
    funk.c
    osc.c noise.c shepard.c random_wave.c stomperosc.c
    adsr.c stomperenv.c seg.c
    butterlpf.c butterhpf.c
    lp4pole.c
    onezero.c onepole.c twopole.c
    delay.c clipper.c
    """)
adts = Split("""
    darray.c htable.c graph.c
    """)
gfx = Split("""
    SDL_gfxPrimitives.c
    colour.c
    widget.c menu.c checkbox.c button.c label.c textbox.c window.c
    """)
audio = Split("""
    audio.c midi.c orch.c ins.c voice.c note.c gen.c track.c
    """)
bliss = Split("""
    about.c file_window.c compan.c aux.c canvas.c config.c utable.c
    file.c gui.c bliss.c
    """)

env = Environment(CCFLAGS = Split('-O2 -pipe -Wall -fomit-frame-pointer'))

env.Command('midi.c', platform + '/midi.c', 'ln -s $SOURCE $TARGET')
env.Command('version.h', 'version.h.in',
	'sed s/@VERSION@/' + program_version + '/ < $SOURCE > $TARGET')
env.ParseConfig('sdl-config --cflags')
env.ParseConfig('sdl-config --libs')

#env.Program('diffy', adts + gfx + ['diffy.c'])
env.Program('bliss', adts + gfx + audio + units + bliss)
