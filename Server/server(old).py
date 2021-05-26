import socket
import threading
import os
import sys
import tomita
import tomita.legacy.pysynth as py
import tomita.legacy.pysynth_c as pc
import tomita.legacy.pysynth_e as pe
from src.MarkovMusic import MusicMatrix
from random import *

host = 'localhost'
port = 3000
lst = []

originalKeys = [
    [1, 'A0'], [2, 'A#0'], [3, 'B0'],
    [4, 'C1'], [5, 'C#1'], [6, 'D1'], [7, 'D#1'], [8, 'E1'], [9, 'F1'],
    [10, 'F#1'], [11, 'G1'], [12, 'G#1'], [13, 'A1'], [14, 'A#1'], [15, 'B1'],
    [16, 'C2'], [17, 'C#2'], [18, 'D2'], [19, 'D#2'], [20, 'E2'], [21, 'F2'],
    [22, 'F#2'], [23, 'G2'], [24, 'G#2'], [25, 'A2'], [26, 'A#2'], [27, 'B2'],
    [28, 'C3'], [29, 'C#3'], [30, 'D3'], [31, 'D#3'], [32, 'E3'], [33, 'F3'],
    [34, 'F#3'], [35, 'G3'], [36, 'G#3'], [37, 'A3'], [38, 'A#3'], [39, 'B3'],
    [40, 'C4'], [41, 'C#4'], [42, 'D4'], [43, 'D#4'], [44, 'E4'], [45, 'F4'],
    [46, 'F#4'], [47, 'G4'], [48, 'G#4'], [49, 'A4'], [50, 'A#4'], [51, 'B4'],
    [52, 'C5'], [53, 'C#5'], [54, 'D5'], [55, 'D#5'], [56, 'E5'], [57, 'F5'],
    [58, 'F#5'], [59, 'G5'], [60, 'G#5'], [61, 'A5'], [62, 'A#5'], [63, 'B5'],
    [64, 'C6'], [65, 'C#6'], [66, 'D6'], [67, 'D#6'], [68, 'E6'], [69, 'F6'],
    [70, 'F#6'], [71, 'G6'], [72, 'G#6'], [73, 'A6'], [74, 'A#6'], [75, 'B6'],
    [76, 'C7'], [77, 'C#7'], [78, 'D7'], [79, 'D#7'], [80, 'E7'], [81, 'F7'],
    [82, 'F#7'], [83, 'G7'], [84, 'G#7'], [85, 'A7'], [86, 'A#7'], [87, 'B7'],
    [88, 'C8']
]
tempKeys = [
    ['r', -1], ['f3', 0], ['g3', 1], ['a3', 2], ['b3', 3], ['c4', 4], ['d4', 5], ['e4', 6],
    ['f4', 7], ['g4', 8], ['a4', 9], ['b4', 10], ['c5', 11], ['d5', 12], ['e5', 13],
    ['f5', 14], ['g5', 15], ['a5', 16], ['b5', 17], ['c6', 18], ['d6', 19], ['e6', 20]
]
tempKeys2 = [
    ['r', -1], ['f3', 0], ['g3', 1], ['a3', 2], ['a#3', 3], ['c4', 4], ['d4', 5], ['e4', 6],
    ['f4', 7], ['g4', 8], ['a4', 9], ['a#4', 10], ['c5', 11], ['d5', 12], ['e5', 13],
    ['f5', 14], ['g5', 15], ['a5', 16], ['a#5', 17], ['c6', 18], ['d6', 19], ['e6', 20]
]
tempKeys3 = [
    ['r', -1], ['f#3', 0], ['g3', 1], ['a3', 2], ['b3', 3], ['c4', 4], ['d4', 5], ['e4', 6],
    ['f#4', 7], ['g4', 8], ['a4', 9], ['b4', 10], ['c5', 11], ['d5', 12], ['e5', 13],
    ['f#5', 14], ['g5', 15], ['a5', 16], ['b5', 17], ['c6', 18], ['d6', 19], ['e6', 20]
]
tempKeys4 = [
    ['r', -1], ['f3', 0], ['g3', 1], ['g#3', 2], ['a#3', 3], ['c4', 4], ['d4', 5], ['d#4', 6],
    ['f4', 7], ['g4', 8], ['g#4', 9], ['a#4', 10], ['c5', 11], ['d5', 12], ['d#5', 13],
    ['f5', 14], ['g5', 15], ['g#5', 16], ['a#5', 17], ['c6', 18], ['d6', 19], ['d#6', 20]
]
tempKeys5 = [
    ['r', -1], ['f3', 0], ['g3', 1], ['a3', 2], ['a#3', 3], ['c4', 4], ['d4', 5], ['d#4', 6],
    ['f4', 7], ['g4', 8], ['a4', 9], ['a#4', 10], ['c5', 11], ['d5', 12], ['d#5', 13],
    ['f5', 14], ['g5', 15], ['a5', 16], ['a#5', 17], ['c6', 18], ['d6', 19], ['d#6', 20]
]

song = [['c4', 4], ['c4', 4], ['c4', 4], ['d4', 8], ['e4', 4], ['e4', 4], ['d4', 8], ['e4', 4], ['f4', 8], ['g4', 2], ['c4', 8], ['c4', 8], ['c4', 8], ['g4', 8], ['g4', 8], ['g4', 8], ['e4', 8], ['e4', 8], ['e4', 8], ['c4', 8], ['c4', 8], ['c4', 8], ['g4', 4], ['f4', 8], ['e4', 4], ['d4', 8], ['c4', 2]]

def binder(c, addr):
    try:
        # Data Recieve
        data = c.recv(1024)
        data = data.decode('utf-8', 'replace')
        print(data)
        lst = data.split(sep='/')
    except:
        print("Client Down")
    finally:
        global songNum
        song1 = []
        song2 = []
        select = 0
        keys = []
        key = 0
        keyCnt = 0
        notes1 = []
        notes2 = []
        beats = []
        index = 0
        print("Client End")

        # Select Number
        select = int(lst[len(lst) - 1])
        print("select : " + str(select))
        
        # Music Key
        key = int(lst[len(lst) - 2]) // 10
        keyCnt = int(lst[len(lst) - 2]) % 10
        print("key : " + str(key))
        print("keyCnt : " + str(keyCnt))

        # Temp Array Set
        if key == 0:
            keys = tempKeys
        elif key == 1:
            if keyCnt == 1:
                keys = tempKeys2
            elif keyCnt == 2:
                keys = tempKeys5
            elif keyCnt == 3:
                keys = tempKeys4
        else:
            if keyCnt == 1:
                keys = tempKeys3

        # Data Processing
        for i in range(1, len(lst) - 2):
            print(lst[i])
        for i in range(1, len(lst) - 2):
            sub = lst[i].find(',')
            for j in range(0, len(tempKeys)):
                if int(lst[i][0:sub]) == keys[j][1]:
                    index = j
            notes1.append(keys[index][0])
            if keys[index][0] == 'r':
                notes2.append(keys[index][0])
            else:
                notes2.append(keys[index + 4][0])
            beats.append(lst[i][sub + 1:])
            song1.append([notes1[i - 1], int(beats[i - 1])])
            song2.append([notes2[i - 1], int(beats[i - 1])])

        # Make Wav File
        print("original song")
        print(song1)
        randomNum = randint(10000000, 99999999)
        songName = "Temp/0" + str(randomNum) + ".wav"
        if select == 0:
            py.make_wav(song1, fn = songName)
        elif select == 1:
            songName1 = "Temp/01" + str(randomNum) + ".wav"
            songName2 = "Temp/02" + str(randomNum) + ".wav"
            pe.make_wav(song1, fn = songName1)
            pc.make_wav(song2, fn = songName2)
            py.mix_files(songName1, songName2, songName)
        else:
            matrix = MusicMatrix(song1)
            chain = [song1[0][0], song1[0][1]]
            random_song = []
            for i in range(0, len(song1)):
                chain = matrix.next_note(chain)
                random_song.append(chain)
            markov_song = list(song1) + list(random_song)
            
            matrix = MusicMatrix(markov_song)
            chain = [song1[0][0], song1[0][1]]
            result_song = []
            for i in range(0, len(song1)):
                chain = matrix.next_note(chain)
                result_song.append(chain)
            print("random song")
            print(result_song)
            pe.make_wav(result_song, fn = songName)

        # Send Wav File
        with open(songName, 'rb') as f:
            try:
                data2 = f.read(1024)
                while data2:
                    c.send(data2)
                    data2 = f.read(1024)
                    print("Send...")
                print("Send Success")
                c.close()
            except:
                print("Send Error")


# Socket Set & Start
s = socket.socket(socket.AF_INET)
s.bind((host, port))
s.listen()

try:
    while(True):
        c, addr = s.accept()
        th = threading.Thread(target = binder, args = (c, addr));
        th.start()
except:
    print("Server Down")
finally:
    print("Server End")
    s.close()
