import math

from mido import MidiFile

from pyknon.genmidi import Midi
from pyknon.music import NoteSeq, Note, Rest
from midi2audio import FluidSynth

def make_midi(midi_path, notes, bpm, ins, t):
    note_names = 'c c# d d# e f f# g g# a  a# b'.split()

    result = NoteSeq()
    melody_dur = 0
    for n in notes:
        if (n[1] < 0):
            duration = (1.0 / -n[1]) + (1.0 / -n[1] / 2)
        else:
            duration = 1.0 / n[1]
        print(duration)
        melody_dur += duration

        if n[0].lower() == 'r':
            result.append(Rest(dur=duration))
        else:
            pitch = n[0][:-1]
            octave = int(n[0][-1]) + 1
            pitch_number = note_names.index(pitch.lower())

            result.append(Note(pitch_number, octave=octave, dur=duration, volume=100))

    harmony_len = len(notes) + (4 - len(notes) % 4)
    duration = 0.25
    pitch = [[0, 4, 7], [9, 0, 3], [2, 5, 9], [7, 11, 2, 5]]
    octave = [[5, 5, 5], [5, 6, 6], [5, 5, 5], [5, 5, 6, 6]]

    guitar0 = NoteSeq()
    for n in range(harmony_len):
        index = int(math.floor(n%16/4))
        guitar0.append(Note(pitch[index][0], octave=octave[index][0], dur=duration, volume=50))

    guitar1 = NoteSeq()
    for n in range(harmony_len):
        index = int(math.floor(n%16/4))
        guitar1.append(Note(pitch[index][1], octave=octave[index][1], dur=duration, volume=50))

    guitar2 = NoteSeq()
    for n in range(harmony_len):
        index = int(math.floor(n%16/4))
        guitar2.append(Note(pitch[index][2], octave=octave[index][2], dur=duration, volume=50))

    guitar3 = NoteSeq()
    for n in range(harmony_len):
        index = int(math.floor(n%16/4))
        if(index == 3):
            guitar3.append(Note(pitch[index][3], octave=octave[index][3], dur=duration, volume=50))
        else:
            guitar3.append(Rest(dur=duration))
    
    cymbal = NoteSeq()
    for n in range(harmony_len):
        duration = 0.25
        pitch = 10
        octave = 3
        cymbal.append(Note(pitch, octave=octave, dur=duration, volume=50))

    kick = NoteSeq()
    for n in range(harmony_len):
        duration = 0.25
        pitch = 0
        octave = 3
        
        if(n%4==0):
            kick.append(Note(pitch, octave=octave, dur=duration, volume=50))
        else:
            kick.append(Rest(dur=duration))

    snare = NoteSeq()
    for n in range(harmony_len):
        duration = 0.25
        pitch = 4
        octave = 3
        
        if(n%4==2):
            snare.append(Note(pitch, octave=octave, dur=duration, volume=50))
        else:
            snare.append(Rest(dur=duration))
    

    midi = Midi(number_tracks=8, tempo=bpm, instrument=[5, 25, 25, 25, 25, 0, 0, 0])
    midi.seq_notes(result, track=0)
    midi.seq_notes(guitar0, track=1)
    midi.seq_notes(guitar1, track=2)
    midi.seq_notes(guitar2, track=3)
    midi.seq_notes(guitar3, track=4)
    midi.seq_notes(cymbal, track=5, channel = 9)
    midi.seq_notes(kick, track=6, channel = 9)
    midi.seq_notes(snare, track=7, channel = 9)
    midi.write(midi_path)

tempKeys = [
    ['g4', -4.0], ['a4', 8.0], ['g4', 4.0], ['g4', 4.0],
    ['c5', 4.0], ['c5', 8.0], ['b4', 8.0], ['a4', 2.0],
    ['g4', 4.0], ['a4', 8.0], ['g4', 8.0], ['f4', 8.0], ['d4', 8.0], ['f4', 8.0], ['a4', 8.0],
    ['g4', -2.0],
    ['g4', -4.0], ['a4', 8.0], ['g4', 4.0], ['g4', 4.0],
    ['c5', 4.0], ['c5', 8.0], ['b4', 8.0], ['a4', 2.0],
    ['g4', 8.0], ['a4', 8.0], ['b4', 8.0], ['c5', 8.0], ['d5', 8.0], ['d5', 8.0], ['c5', 8.0], ['b4', 8.0],
    ['c5', -2.0]
]

make_midi(midi_path = 'testout.mid', notes=tempKeys, bpm=120, ins=5, t=0)

fs = FluidSynth('FluidR3_GM.sf2', 44100)
fs.midi2audio('testout.mid', "testout.wav")


#fs.play_midi('output.mid')
#fs.midi_to_audio('test.mid', "output2.wav")

#cv1 = MidiFile('output-left hand.mid', clip=True)
#cv2 = MidiFile('output-right hand.mid', clip=True)

#cv2.tracks.append(cv1.tracks[0])

#cv2.save('output-double.mid')
