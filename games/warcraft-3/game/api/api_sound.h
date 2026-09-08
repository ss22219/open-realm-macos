extern LPPLAYER currentplayer;

static DWORD JassCreateSoundHandle(LPJASS j, LPCSTR fileName, BOOL looping, BOOL is3D,
                                   BOOL stopwhenoutofrange, LONG fadeInRate, LONG fadeOutRate,
                                   int soundIndex) {
    gsound_t *sound = jass_newhandle(j, sizeof(*sound), "sound");
    if (!sound) return 0;
    strlcpy(sound->fileName, fileName ? fileName : "", sizeof(sound->fileName));
    sound->looping = looping;
    sound->is3D = is3D;
    sound->stopwhenoutofrange = stopwhenoutofrange;
    sound->fadeInRate = fadeInRate;
    sound->fadeOutRate = fadeOutRate;
    sound->soundIndex = soundIndex;
    sound->volume = 1.0f;
    sound->pitch = 1.0f;
    sound->minDistance = 0.0f;
    sound->maxDistance = 100000.0f;
    sound->distanceCutoff = 100000.0f;
    sound->coneInside = 360.0f;
    sound->coneOutside = 360.0f;
    sound->coneOutsideVolume = 127.0f;
    G_JassSoundRuntimeInit(sound);
    return 1;
}

DWORD CreateSound(LPJASS j) {
    LPCSTR fileName = jass_checkstring(j, 1);
    BOOL looping = jass_checkboolean(j, 2);
    BOOL is3D = jass_checkboolean(j, 3);
    BOOL stopwhenoutofrange = jass_checkboolean(j, 4);
    LONG fadeInRate = jass_checkinteger(j, 5);
    LONG fadeOutRate = jass_checkinteger(j, 6);
    LPCSTR eaxSetting = jass_checkstring(j, 7);
    (void)eaxSetting;
    return JassCreateSoundHandle(j, fileName, looping, is3D, stopwhenoutofrange,
                                 fadeInRate, fadeOutRate, gi.SoundIndex(fileName));
}
/* Label constructors resolve WC3 sound-data rows into the same game-owned sound
 * descriptor as CreateSound. Playback is sent through the entity/server sound
 * path; JASS never owns or calls the client mixer directly. */
DWORD CreateSoundFilenameWithLabel(LPJASS j) {
    LPCSTR fileName = jass_checkstring(j, 1);
    BOOL looping = jass_checkboolean(j, 2);
    BOOL is3D = jass_checkboolean(j, 3);
    BOOL stopwhenoutofrange = jass_checkboolean(j, 4);
    LONG fadeInRate = jass_checkinteger(j, 5);
    LONG fadeOutRate = jass_checkinteger(j, 6);
    LPCSTR label = jass_checkstring(j, 7);
    int soundIndex = label && *label ? G_JassSoundIndexFromLabel(label) : gi.SoundIndex(fileName);
    return JassCreateSoundHandle(j, fileName, looping, is3D, stopwhenoutofrange,
                                 fadeInRate, fadeOutRate, soundIndex)
        ? 1 : jass_pushnullhandle(j, "sound");
}
DWORD CreateSoundFromLabel(LPJASS j) {
    LPCSTR label = jass_checkstring(j, 1);
    BOOL looping = jass_checkboolean(j, 2);
    BOOL is3D = jass_checkboolean(j, 3);
    BOOL stopwhenoutofrange = jass_checkboolean(j, 4);
    LONG fadeInRate = jass_checkinteger(j, 5);
    LONG fadeOutRate = jass_checkinteger(j, 6);
    int soundIndex = G_JassSoundIndexFromLabel(label);
    return JassCreateSoundHandle(j, label, looping, is3D, stopwhenoutofrange,
                                 fadeInRate, fadeOutRate, soundIndex)
        ? 1 : jass_pushnullhandle(j, "sound");
}
DWORD CreateMIDISound(LPJASS j) {
    LPCSTR label = jass_checkstring(j, 1);
    LONG fadeInRate = jass_checkinteger(j, 2);
    LONG fadeOutRate = jass_checkinteger(j, 3);
    int soundIndex = G_JassSoundIndexFromLabel(label);
    return JassCreateSoundHandle(j, label, false, false, true, fadeInRate, fadeOutRate, soundIndex)
        ? 1 : jass_pushnullhandle(j, "sound");
}
DWORD SetSoundParamsFromLabel(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    LPCSTR label = jass_checkstring(j, 2);
    if (sound) sound->soundIndex = G_JassSoundIndexFromLabel(label);
    return 0;
}
DWORD SetSoundDistanceCutoff(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) sound->distanceCutoff = MAX(0.0f, jass_checknumber(j, 2));
    return 0;
}
DWORD SetSoundChannel(LPJASS j) {
    (void)jass_checkhandle(j, 1, "sound");
    (void)jass_checkinteger(j, 2);
    return 0;
}
DWORD SetSoundVolume(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    LONG volume = jass_checkinteger(j, 2);
    if (sound) {
        sound->volume = (FLOAT)MAX(0, MIN(volume, 127)) / 127.0f;
        G_JassSoundSetVolume(sound, sound->volume);
    }
    return 0;
}
DWORD SetSoundPitch(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) sound->pitch = MAX(0.0f, jass_checknumber(j, 2));
    return 0;
}
DWORD SetSoundDistances(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) {
        sound->minDistance = MAX(0.0f, jass_checknumber(j, 2));
        sound->maxDistance = MAX(sound->minDistance, jass_checknumber(j, 3));
    }
    return 0;
}
DWORD SetSoundConeAngles(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) {
        sound->coneInside = jass_checknumber(j, 2);
        sound->coneOutside = jass_checknumber(j, 3);
        sound->coneOutsideVolume = jass_checkinteger(j, 4);
    }
    return 0;
}
DWORD SetSoundConeOrientation(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) sound->coneOrientation = MAKE(VECTOR3, jass_checknumber(j, 2),
                                              jass_checknumber(j, 3), jass_checknumber(j, 4));
    return 0;
}
DWORD SetSoundPosition(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    FLOAT x = jass_checknumber(j, 2);
    FLOAT y = jass_checknumber(j, 3);
    FLOAT z = jass_checknumber(j, 4);
    if (sound) G_JassSoundSetPosition(sound, &MAKE(VECTOR3, x, y, z));
    return 0;
}
DWORD SetSoundVelocity(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) {
        sound->velocity = MAKE(VECTOR3, jass_checknumber(j, 2), jass_checknumber(j, 3), jass_checknumber(j, 4));
        sound->hasVelocity = true;
    }
    return 0;
}
DWORD AttachSoundToUnit(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    LPEDICT whichUnit = jass_checkhandle(j, 2, "unit");
    if (sound) G_JassSoundAttach(sound, whichUnit);
    return 0;
}
/* StartSound snapshots the current transient sound-handle presentation state
 * into one generic sound packet. Continuous attachment tracking and stop/fade
 * playback lifetime remain separate mixer work. */
DWORD StartSound(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    jassSoundPlayback_t playback;
    FLOAT attenuation;

    if (!sound || !sound->soundIndex) return 0;
    sound->stopped = false;
    sound->playing = true;
    G_JassSoundPlayback(sound, &playback);
    attenuation = sound->is3D ? 1.0f : 0.0f;

    if (currentplayer) {
        LPEDICT recipient = PLAYER_ENT(currentplayer);
        if (!recipient) return 0;
        if (playback.positioned)
            gi.PositionedSound(&playback.origin, recipient, CHAN_OWNER | CHAN_RELIABLE, sound->soundIndex,
                               playback.volume, attenuation, 0.0f);
        else
            gi.Sound(recipient, CHAN_OWNER | CHAN_RELIABLE, sound->soundIndex,
                     playback.volume, attenuation, 0.0f);
        return 0;
    }

    if (playback.positioned)
        gi.PositionedSound(&playback.origin, playback.emitter, CHAN_RELIABLE, sound->soundIndex,
                           playback.volume, attenuation, 0.0f);
    else
        gi.Sound(NULL, CHAN_RELIABLE, sound->soundIndex, playback.volume, attenuation, 0.0f);
    return 0;
}
DWORD StopSound(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    (void)jass_checkboolean(j, 2);
    (void)jass_checkboolean(j, 3);
    if (sound) { sound->stopped = true; sound->playing = false; }
    return 0;
}
DWORD KillSoundWhenDone(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    if (sound) sound->killWhenDone = true;
    return 0;
}
DWORD SetMusicVolume(LPJASS j) {
    G_MusicSetVolume(jass_checkinteger(j, 1));
    return 0;
}
DWORD SetMusicPlayPosition(LPJASS j) {
    G_MusicSetPosition(jass_checkinteger(j, 1));
    return 0;
}
DWORD SetThematicMusicVolume(LPJASS j) {
    G_MusicSetThematicVolume(jass_checkinteger(j, 1));
    return 0;
}
DWORD SetThematicMusicPlayPosition(LPJASS j) {
    G_MusicSetThematicPosition(jass_checkinteger(j, 1));
    return 0;
}
DWORD PlayMusic(LPJASS j) {
    G_MusicPlay(jass_checkstring(j, 1), 0, 0);
    return 0;
}
DWORD PlayMusicEx(LPJASS j) {
    LPCSTR musicName = jass_checkstring(j, 1);
    LONG frommsecs = jass_checkinteger(j, 2);
    LONG fadeinmsecs = jass_checkinteger(j, 3);
    G_MusicPlay(musicName, MAX(0, frommsecs), MAX(0, fadeinmsecs));
    return 0;
}
DWORD SetMapMusic(LPJASS j) {
    LPCSTR musicName = jass_checkstring(j, 1);
    BOOL random = jass_checkboolean(j, 2);
    LONG index = jass_checkinteger(j, 3);
    G_MusicSetMap(musicName, random, index);
    return 0;
}
DWORD ClearMapMusic(LPJASS j) {
    G_MusicClearMap();
    return 0;
}
DWORD PlayThematicMusic(LPJASS j) {
    G_MusicPlayThematic(jass_checkstring(j, 1), 0);
    return 0;
}
DWORD PlayThematicMusicEx(LPJASS j) {
    LPCSTR musicFileName = jass_checkstring(j, 1);
    LONG frommsecs = jass_checkinteger(j, 2);
    G_MusicPlayThematic(musicFileName, MAX(0, frommsecs));
    return 0;
}
DWORD EndThematicMusic(LPJASS j) {
    G_MusicEndThematic();
    return 0;
}
DWORD StopMusic(LPJASS j) {
    G_MusicStop(jass_checkboolean(j, 1));
    return 0;
}
DWORD ResumeMusic(LPJASS j) {
    G_MusicResume();
    return 0;
}
DWORD SetSoundDuration(LPJASS j) {
    gsound_t *soundHandle = jass_checkhandle(j, 1, "sound");
    if (soundHandle) soundHandle->duration = MAX(0, jass_checkinteger(j, 2));
    return 0;
}
DWORD GetSoundDuration(LPJASS j) {
    gsound_t *soundHandle = jass_checkhandle(j, 1, "sound");
    return jass_pushinteger(j, soundHandle ? soundHandle->duration : -1);
}
DWORD GetSoundFileDuration(LPJASS j) {
    //LPCSTR musicFileName = jass_checkstring(j, 1);
    return jass_pushinteger(j, 0);
}
DWORD VolumeGroupSetVolume(LPJASS j) {
    //HANDLE vgroup = jass_checkhandle(j, 1, "volumegroup");
    //FLOAT scale = jass_checknumber(j, 2);
    return 0;
}
DWORD VolumeGroupReset(LPJASS j) {
    return 0;
}
DWORD GetSoundIsPlaying(LPJASS j) {
    gsound_t *sound = jass_checkhandle(j, 1, "sound");
    return jass_pushboolean(j, sound && sound->playing && !sound->stopped);
}
DWORD GetSoundIsLoading(LPJASS j) {
    //HANDLE soundHandle = jass_checkhandle(j, 1, "sound");
    return jass_pushboolean(j, 0);
}
DWORD RegisterStackedSound(LPJASS j) {
    (void)jass_checkhandle(j, 1, "sound");
    (void)jass_checkboolean(j, 2);
    (void)jass_checknumber(j, 3);
    (void)jass_checknumber(j, 4);
    return 0;
}
DWORD UnregisterStackedSound(LPJASS j) {
    (void)jass_checkhandle(j, 1, "sound");
    (void)jass_checkboolean(j, 2);
    (void)jass_checknumber(j, 3);
    (void)jass_checknumber(j, 4);
    return 0;
}
