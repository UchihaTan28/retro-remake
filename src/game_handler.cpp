// game_handler.cpp

#include "game_handler.hpp"
#include "state_handler.hpp"
#include "player_handler.hpp"

#include <raylib.h>
#include <vector>
#include <string>
#include <fstream>

using  std::vector;
using  std::string;

// --------------------------------------------------------------------------------------
// Constructor: set up window, audio, and initial game state
// --------------------------------------------------------------------------------------
Game::Game()
    : state(GameState::Intro)
    , level(1)
    , score(0)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, GAME_TITLE);
    InitAudioDevice();
    SetTargetFPS(TARGET_FPS);

    // ----------------------------------------------------------------------
    // Load all sprite textures, music tracks, and sound effects into maps
    // ----------------------------------------------------------------------
    initializeAllSprites(spritesList);
    initializeMusicTracks(musicsList);
    initializeSoundEffects(soundsList);

    // ------------------------------------------------------------------
    // Attempt to restore last session (if any)
    // ------------------------------------------------------------------
    loadState();

    // ----------------------------------------------------------------------
    // Configure how many frames each animated sprite contains
    // ----------------------------------------------------------------------
    // Player animations
    sprites.at("player_default").setFrameCount(2);
    sprites.at("player_defeated").setFrameCount(2);

    // Wang animations
    sprites.at("wang_default").setFrameCount(2);
    sprites.at("wang_kick"   ).setFrameCount(2);
    sprites.at("wang_punch"  ).setFrameCount(2);

    // Tao animations
    sprites.at("tao_default").setFrameCount(2);
    sprites.at("tao_kick"   ).setFrameCount(2);
    sprites.at("tao_punch"  ).setFrameCount(2);

    // Chen animations
    sprites.at("chen_default").setFrameCount(4);
    sprites.at("chen_kick"   ).setFrameCount(2);
    sprites.at("chen_punch"  ).setFrameCount(2);

    // Lang animations
    sprites.at("lang_default").setFrameCount(2);
    sprites.at("lang_kick"   ).setFrameCount(2);
    sprites.at("lang_punch"  ).setFrameCount(2);

    // Mu animations
    sprites.at("mu_default").setFrameCount(2);
    sprites.at("mu_kick"   ).setFrameCount(2);
    sprites.at("mu_punch"  ).setFrameCount(2);

    // Special & font
    sprites.at("spinning_chain").setFrameCount(8);
    sprites.at("font_symbols"  ).setFrameCount(spriteLetters.size());

    // ----------------------------------------------------------------------
    // Override frame‐advance speed for every enemy animation
    // ----------------------------------------------------------------------
    for (const auto &spriteName : enemies)
    {
        sprites.at(spriteName + "_default").setAnimationSpeed(EnemyWalkSpriteFPS);
        sprites.at(spriteName + "_kick"   ).setAnimationSpeed(EnemyWalkSpriteFPS);
        sprites.at(spriteName + "_punch"  ).setAnimationSpeed(EnemyWalkSpriteFPS);
    }

    // ----------------------------------------------------------------------
    // Instantiate player and game states (intro, preview, play)
    // ----------------------------------------------------------------------
    player       = new Player(this);
    introState   = new IntroState(this);
    previewState = new PreviewState(this);
    playState    = new PlayState(this);
}

// --------------------------------------------------------------------------------------
// Main loop: dispatch to the current state until the window closes
// --------------------------------------------------------------------------------------
void Game::run()
{
    while (!IsKeyDown(KEY_ESCAPE) && !WindowShouldClose())
    {
        if      (state == GameState::Intro)   introState->run();
        else if (state == GameState::Preview) previewState->run();
        else                                  playState->run();
    }

    cleanUp();
    saveState();
    CloseWindow();
}

// ----------------------------------------------------------------------
// Write out `state`, `level`, and `score` to a binary file.
// ----------------------------------------------------------------------
void Game::saveState()
{
    std::ofstream ofs(saveFileName_, std::ios::binary);
    if (!ofs) return; // silently fail if we can't write
    ofs.write(reinterpret_cast<const char*>(&state), sizeof(state));
    ofs.write(reinterpret_cast<const char*>(&level), sizeof(level));
    ofs.write(reinterpret_cast<const char*>(&score), sizeof(score));
}

// ----------------------------------------------------------------------
// If `savegame.dat` exists and is well-formed, reload our three fields.
// ----------------------------------------------------------------------
void Game::loadState()
{
    std::ifstream ifs(saveFileName_, std::ios::binary);
    if (!ifs) return; // no save to load

    GameState savedState;
    int       savedLevel;
    int       savedScore;

    ifs.read(reinterpret_cast<char*>(&savedState), sizeof(savedState));
    ifs.read(reinterpret_cast<char*>(&savedLevel), sizeof(savedLevel));
    ifs.read(reinterpret_cast<char*>(&savedScore), sizeof(savedScore));

    // Only apply if the file was complete
    if (ifs) {
        state = savedState;
        level = savedLevel;
        score = savedScore;
    }
}

// --------------------------------------------------------------------------------------
// Helpers: batch‐load textures, music, and sound effects from lists of names
// --------------------------------------------------------------------------------------
void Game::initializeAllSprites(const vector<string> &spritesList)
{
    for (const auto &name : spritesList)
    {
        sprites.emplace(name, Sprite(ASSETS_PATH + "images/" + name + ".png"));
    }
}

void Game::initializeMusicTracks(const vector<string> &musicsList)
{
    for (const auto &name : musicsList)
    {
        musics.emplace(name,
            LoadMusicStream((ASSETS_PATH + "musics/" + name + ".mp3").c_str()));
    }
}

void Game::initializeSoundEffects(const vector<string> &soundsList)
{
    for (const auto &name : soundsList)
    {
        sounds.emplace(name,
            LoadSound((ASSETS_PATH + "sounds/" + name + ".wav").c_str()));
    }
}

// --------------------------------------------------------------------------------------
// Tear down all resources: textures, render-to-texture targets, sound & music
// --------------------------------------------------------------------------------------
void Game::cleanUp()
{
    // Unload all sprite textures
    for (const auto &spriteName : spritesList)
    {
        sprites.at(spriteName).unload();
    }

    // Unload render textures used by each state
    previewState->unloadTexture();
    playState->unloadTexture();
    introState->unloadTexture();

    // Unload all sound effects
    for (const auto &spriteName : soundsList)
    {
        UnloadSound(sounds.at(spriteName));
    }

    // Unload all streaming music tracks
    for (const auto &spriteName : musicsList)
    {
        UnloadMusicStream(musics.at(spriteName));
    }

    CloseAudioDevice();
}

// --------------------------------------------------------------------------------------
// Entry point: create Game instance and hand control to its run() method
// --------------------------------------------------------------------------------------
int main()
{
    Game game;
    game.run();
    return EXIT_SUCCESS;
}
