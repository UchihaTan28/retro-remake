#pragma once

#include <string>
#include <vector>
#include <array>
#include <raylib.h>
#include "settings.hpp"

class Sprite {
public:
    // ----------------------------------------------------------------
    // life-cycle
    explicit Sprite(const std::string &filePath);
    ~Sprite()= default;// { UnloadTexture(texture_); }

    // ----------------------------------------------------------------
    // rendering
    void unload() { UnloadTexture(texture_); } // unloads the GPU texture

    inline void draw() { // draw the current frame at position
        DrawTextureRec(texture_, sourceRect_, { float(x), float(y) }, WHITE);
    }
    inline void drawFrame(int index) { // draw an expliict frame by index
        sourceRect_.x = index * (texture_.width / frameCount_);
        draw();
    }

    // Animation control
    /// Advances the frame timer, loops if needed, draws, and
    /// returns true if we just wrapped around to frame 0.
    inline bool updateAndDraw() { 
        bool last = false;
        if (++frameTimer_ >= TARGET_FPS / ticksBwFrame_) {
            frameTimer_ = 0;
            if (!_isPaused) {
                if (++currFrame_ >= frameCount_) {
                    currFrame_ = 0;
                    last = true;
                }
            }
            sourceRect_.x = currFrame_ * (texture_.width / frameCount_);
        }
        draw();
        return last;
    }
    //how many sub-images (tiles) this texture is wrapped into
    inline void setFrameCount(int count) {
        frameCount_      = count;
        sourceRect_      = { 0, 0, float(texture_.width) / count, float(texture_.height) };
        currFrame_   = 0;
        frameTimer_  = 0;
    }

    // how many ticks to wait between frame advances
    inline void setAnimationSpeed(int speed) { ticksBwFrame_ = speed; }

    /// Jump back to the very first frame immediately
    inline void resetAnimation()       { currFrame_ = frameTimer_ = 0; sourceRect_.x = 0; }

    // ----------------------------------------------------------------
    // utilities
    inline void invertHorizontally()          { sourceRect_.width = -sourceRect_.width; } // mirror on the x-axis

    // Accessors
    inline Texture2D getTexture() const   { return texture_; }
    inline int      getTileCount() const  { return frameCount_; }

    // ----------------------------------------------------------------
    // position & state
    int x = 0, y = 0;
    bool _isPaused = false;

private:
    Texture2D texture_; //the GPU resident image
    int       frameCount_     = 1; // how man ytiles horizontally
    int       currFrame_  = 0;
    int       frameTimer_ = 0; // tick counter for timing
    Rectangle sourceRect_{}; // which slice of the texture to draw
    int       ticksBwFrame_    = FRAME_SPEED; //
};

// List of all sprite asset names (without extension)
inline const std::vector<std::string> spritesList = {
    //logos and fonts
    "game_name",    "logo_konami",  "font_symbols", 

    //backgrounds/ icons
    "bg_dojo", "life_icon",
    
    //HUD elements
    "hud_health",    "green_health",  "red_health",

    //player states
    "player_default","player_crouch",  "player_punch_stand","player_kick_stand",
    "player_punch_crouch", "player_kick_crouch","player_kick_high","player_kick_fly", "player_defeated", "player_smile",
    
    //enemy states
    "wang_default", "wang_kick", "wang_punch", "wang_hit", "wang_defeated", //wang
    "tao_default", "tao_kick",  "tao_punch", "tao_hit", "tao_defeated", //tao
    "chen_default", "chen_kick", "chen_punch", "chen_hit", "chen_defeated", //chen
    "lang_default", "lang_kick", "lang_punch", "lang_hit", "lang_defeated", //lang
    "mu_default", "mu_kick", "mu_punch", "mu_hit", "mu_defeated", //mu
    "spinning_chain","effect_hit"
};

// On-screen text constants
inline constexpr const char* COPYRIGHT_TEXT = "# 1985 konami";
inline constexpr const char* OTHER_TEXT     = "# 2025 tanay";
inline constexpr const char* TO_START_TEXT = "press enter to start";

// Characters supported for on-screen text
inline const std::array<char, 36> spriteLetters = {
    'a','b','c','d','e','f','g','h','i','k','l','m','n','o','p','q',
    'r','s','t','u','v','w','y','0','1','2','3','4','5','6','7','8','9','-',' ', '#'
};
