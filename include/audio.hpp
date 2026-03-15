#pragma once
#include <raylib.h>
#include <string>

// ---------------------------------------------------------------------------
// SoundId — every distinct sound effect in the game.
// Add new entries before COUNT and add the matching path in audio.cpp.
// ---------------------------------------------------------------------------
enum class SoundId {
    SHOOT,         // gunshot
    RELOAD,        // cylinder click / reload finish
    JUMP,          // jump launch
    LAND,          // landing thud
    DASH,          // dash whoosh
    MELEE,         // sword swing
    PLAYER_HIT,    // player takes damage
    PLAYER_DEATH,  // player dies
    ENEMY_HIT,     // enemy takes damage
    ENEMY_DEATH,   // enemy dies
    KEY_PICKUP,    // key collected
    WIN,           // level complete fanfare
    GAME_OVER,     // game over sting
    COUNT
};

// ---------------------------------------------------------------------------
// MusicId — streaming background tracks.
// ---------------------------------------------------------------------------
enum class MusicId {
    MENU,
    GAMEPLAY,
    COUNT
};

// ---------------------------------------------------------------------------
// AudioManager
//
// Usage:
//   audio.Init();                        // call once after InitAudioDevice()
//   audio.PlaySFX(SoundId::JUMP);        // fire-and-forget sound effect
//   audio.PlayMusic(MusicId::GAMEPLAY);  // start / crossfade a music track
//   audio.Update(dt);                    // call every frame — streams music
//   audio.Shutdown();                    // call before CloseAudioDevice()
//
// Missing audio files are silently skipped — the game never crashes because
// a .wav or .ogg doesn't exist yet.
// ---------------------------------------------------------------------------
class AudioManager {
public:
    void Init();
    void Shutdown();
    void Update(float dt);

    // Play a one-shot sound effect. Volume is scaled by sfxVolume * masterVolume.
    void PlaySFX(SoundId id);

    // Start playing a music track. If the same track is already playing, does
    // nothing. Stops the previous track cleanly.
    void PlayMusic(MusicId id);

    // Stop music without starting a new track.
    void StopMusic();

    // Re-apply current volume settings (call after the Options slider changes).
    void RefreshVolumes();

private:
    static constexpr int SFX_COUNT   = static_cast<int>(SoundId::COUNT);
    static constexpr int MUSIC_COUNT = static_cast<int>(MusicId::COUNT);

    Sound sounds[SFX_COUNT]   = {};
    Music tracks[MUSIC_COUNT] = {};
    bool  sfxLoaded  [SFX_COUNT]   = {};
    bool  musicLoaded[MUSIC_COUNT] = {};

    MusicId currentMusic     = MusicId::COUNT; // COUNT = nothing playing
    bool    musicPlaying     = false;

    static const char* SFX_PATHS  [SFX_COUNT];
    static const char* MUSIC_PATHS[MUSIC_COUNT];

    float SFXVolume()   const;
    float MusicVolume() const;
};

extern AudioManager audio;
