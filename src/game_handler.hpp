 // game_handler.hpp
#pragma once

#include <unordered_map>
#include <raylib.h>
#include <random>
#include <vector>
#include <string>

#include "sprite_handler.hpp"
#include "state_handler.hpp"
#include "player_handler.hpp"
#include "settings.hpp"

using std::string;
using std::vector;
using std::unordered_map;

//------------------------------------------------------------------------------
// Game‐state identifiers
//------------------------------------------------------------------------------
enum class GameState : int {
    Intro = 0,   ///< Title/intro screen
    Preview = 1, ///< “Get ready” screen before each level
    Play = 2     ///< Actual gameplay
};

class Player;
class PlayState; class IntroState; class PreviewState; 

class Game {
private:
    std::mt19937               _rng;
    void cleanUp();
    void initializeAllSprites(const vector<string>& list);
    void initializeMusicTracks(const vector<string>& list);
    void initializeSoundEffects(const vector<string>& list);

    

public:
    GameState                   state   = GameState::Intro;

     // Helper to get a zero-based “level index”
    int levelIndex() const { 
      return static_cast<int>(state) - 1; 
    };

    int                         level   = 1;
    int                         score   = 0;

    IntroState*                 introState   = nullptr;
    PreviewState*               previewState = nullptr;
    PlayState*                  playState    = nullptr;
    Player*                     player       = nullptr;

    unordered_map<string, Sprite>   sprites;
    unordered_map<string, Music>    musics;
    unordered_map<string, Sound>    sounds;

    Game();
    void run();

    //------------------------------------------------------------------------
    // Auto-save key: where we keep our binary state on disk
    //------------------------------------------------------------------------
    const std::string saveFileName_ = "savegame.dat";

    //------------------------------------------------------------------------
    // Load/save helper routines
    //------------------------------------------------------------------------
    void saveState();
    void loadState();
};

// ----------------------------------------------------------------
// Asset lists
inline const vector<string> musicsList = {
    "main_music"
};

inline const vector<string> soundsList = {
    "attack", "collision", "game_over", "defeated", "win", "counting","health_low","twitch_feet", "collision2"
}; 