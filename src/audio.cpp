#include <audio.hpp>
#include <globals.hpp>
#include <raylib.h>
#include <string>

AudioManager audio;

// ---------------------------------------------------------------------------
// Asset paths — place your files here.
// All paths are relative to the binary (assets/ is copied there by CMake).
// Any file that doesn't exist is silently skipped.
// ---------------------------------------------------------------------------
const char* AudioManager::SFX_PATHS[SFX_COUNT] = {
    "assets/audio/sfx/shoot.wav",        // SHOOT
    "assets/audio/sfx/reload.wav",       // RELOAD
    "assets/audio/sfx/jump.wav",         // JUMP
    "assets/audio/sfx/land.wav",         // LAND
    "assets/audio/sfx/dash.wav",         // DASH
    "assets/audio/sfx/melee.wav",        // MELEE
    "assets/audio/sfx/player_hit.wav",   // PLAYER_HIT
    "assets/audio/sfx/player_death.wav", // PLAYER_DEATH
    "assets/audio/sfx/enemy_hit.wav",    // ENEMY_HIT
    "assets/audio/sfx/enemy_death.wav",  // ENEMY_DEATH
    "assets/audio/sfx/key_pickup.wav",   // KEY_PICKUP
    "assets/audio/sfx/win.wav",          // WIN
    "assets/audio/sfx/game_over.wav",    // GAME_OVER
};

const char* AudioManager::MUSIC_PATHS[MUSIC_COUNT] = {
    "assets/audio/music/menu.ogg",       // MENU
    "assets/audio/music/gameplay.ogg",   // GAMEPLAY
};

// ---------------------------------------------------------------------------
// Volume helpers
// ---------------------------------------------------------------------------
float AudioManager::SFXVolume()   const {
    return Global::config.sfxVolume    * Global::config.masterVolume;
}
float AudioManager::MusicVolume() const {
    return Global::config.musicVolume  * Global::config.masterVolume;
}

// ---------------------------------------------------------------------------
// Init — load everything that exists, skip the rest
// ---------------------------------------------------------------------------
void AudioManager::Init() {
    const std::string appDir = GetApplicationDirectory();

    // Print the directory we're searching so you can verify the path
    TraceLog(LOG_INFO, "AudioManager: appDir = %s", appDir.c_str());

    for (int i = 0; i < SFX_COUNT; i++) {
        std::string fullPath = appDir + SFX_PATHS[i];
        bool exists = FileExists(fullPath.c_str());
        TraceLog(LOG_INFO, "SFX[%d] %s -> %s",
            i, fullPath.c_str(), exists ? "FOUND" : "MISSING");
        if (exists) {
            sounds[i]    = LoadSound(fullPath.c_str());
            sfxLoaded[i] = true;
            SetSoundVolume(sounds[i], SFXVolume());
        }
    }
    for (int i = 0; i < MUSIC_COUNT; i++) {
        std::string fullPath = appDir + MUSIC_PATHS[i];
        bool exists = FileExists(fullPath.c_str());
        TraceLog(LOG_INFO, "MUSIC[%d] %s -> %s",
            i, fullPath.c_str(), exists ? "FOUND" : "MISSING");
        if (exists) {
            tracks[i]      = LoadMusicStream(fullPath.c_str());
            musicLoaded[i] = true;
            SetMusicVolume(tracks[i], MusicVolume());
        }
    }
}

// ---------------------------------------------------------------------------
// Shutdown
// ---------------------------------------------------------------------------
void AudioManager::Shutdown() {
    StopMusic();
    for (int i = 0; i < SFX_COUNT;   i++) if (sfxLoaded[i])   UnloadSound(sounds[i]);
    for (int i = 0; i < MUSIC_COUNT;  i++) if (musicLoaded[i]) UnloadMusicStream(tracks[i]);
}

// ---------------------------------------------------------------------------
// Update — must be called every frame to stream music
// ---------------------------------------------------------------------------
void AudioManager::Update(float /*dt*/) {
    if (musicPlaying && currentMusic != MusicId::COUNT) {
        int idx = static_cast<int>(currentMusic);
        if (musicLoaded[idx])
            UpdateMusicStream(tracks[idx]);
    }
}

// ---------------------------------------------------------------------------
// PlaySFX
// ---------------------------------------------------------------------------
void AudioManager::PlaySFX(SoundId id) {
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= SFX_COUNT) return;
    if (!sfxLoaded[idx]) return;
    float vol = SFXVolume();
    TraceLog(LOG_INFO, "PlaySFX[%d] vol=%.2f", idx, vol);
    SetSoundVolume(sounds[idx], vol);
    PlaySound(sounds[idx]);
}

// ---------------------------------------------------------------------------
// PlayMusic — start a new track, stop the old one cleanly
// ---------------------------------------------------------------------------
void AudioManager::PlayMusic(MusicId id) {
    if (id == currentMusic && musicPlaying) return; // already playing this track
    StopMusic();
    int idx = static_cast<int>(id);
    if (idx < 0 || idx >= MUSIC_COUNT) return;
    if (!musicLoaded[idx]) return;
    SetMusicVolume(tracks[idx], MusicVolume());
    PlayMusicStream(tracks[idx]);
    currentMusic = id;
    musicPlaying = true;
}

// ---------------------------------------------------------------------------
// StopMusic
// ---------------------------------------------------------------------------
void AudioManager::StopMusic() {
    if (musicPlaying && currentMusic != MusicId::COUNT) {
        int idx = static_cast<int>(currentMusic);
        if (musicLoaded[idx]) StopMusicStream(tracks[idx]);
    }
    musicPlaying = false;
    currentMusic = MusicId::COUNT;
}

// ---------------------------------------------------------------------------
// RefreshVolumes — call after Options sliders change
// ---------------------------------------------------------------------------
void AudioManager::RefreshVolumes() {
    for (int i = 0; i < SFX_COUNT;  i++) if (sfxLoaded[i])   SetSoundVolume(sounds[i],  SFXVolume());
    for (int i = 0; i < MUSIC_COUNT; i++) if (musicLoaded[i]) SetMusicVolume(tracks[i],  MusicVolume());
}
