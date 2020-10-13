package com.example.practiceopencv;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.SoundPool;

import java.util.HashMap;

public class MusicPlayer {
    private static SoundPool soundPool;
    private static HashMap<Integer, Integer> soundPoolMap;
    private static int[] pianoSounds = {
            R.raw.sound1, R.raw.sound2, R.raw.sound3, R.raw.sound4,
            R.raw.sound5, R.raw.sound6, R.raw.sound7, R.raw.sound8};

    public static int[] initSound(Context context) {
        AudioAttributes attributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                .build();
        soundPool = new SoundPool.Builder()
                .setAudioAttributes(attributes)
                .build();
        soundPoolMap = new HashMap(pianoSounds.length);

        for(int i = 0; i < pianoSounds.length; i++) {
            soundPoolMap.put(pianoSounds[i], soundPool.load(context, pianoSounds[i], 0));
        }

        return pianoSounds;
    }

    public static void play(final int rawID) {
        if(soundPoolMap.containsKey(rawID)) {
            soundPool.play(soundPoolMap.get(rawID), 1, 1, 0, 0, 1f);
        }
    }
}
