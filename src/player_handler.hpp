#ifndef _PLAYER_H_
#define _PLAYER_H_

#pragma once

#include "game_handler.hpp"
#include "other.hpp"
#include <vector>
#include <string>

using std::vector;
using std::string;

//------------------------------------------------------------------------------
// Player action states
//------------------------------------------------------------------------------
enum class PlayerAction : int {
    None            = -1,  
    Default            =  0,  
    DefaultHold        =  1,  
    WalkLeft        =  2,  
    WalkRight       =  3,  
    Crouch          =  4, 
    PunchStand      =  5, 
    PunchCrouch     =  6,  
    KickStand       =  7,  
    KickCrouch      =  8,  
    KickHigh        =  9,  
    JumpUp          = 10,  
    JumpDown        = 11,  
    Smile           = 12,  
    Defeated        = 13,  
    VeryDefeated    = 14  
};

//------------------------------------------------------------------------------
// Jump drift directions
//------------------------------------------------------------------------------
enum class JumpDrift : int {
    NoneDrift = 0,  
    LeftDrift = 1,  
    RightDrift= 2   
};

//------------------------------------------------------------------------------
// Movement & timing constants
//------------------------------------------------------------------------------
constexpr int kPlayerDefaultX                  = 35;   
constexpr int kPlayerDefaultY                  = 160;  
constexpr int kPlayerSpeed                     = 1;    
constexpr int kPlayerFrameRate                 = 12;   
constexpr int kPlayerDefaultLives              = 2;    

constexpr int kPlayerJumpHeight                = 114;  
constexpr int kPlayerJumpSpeed                 = 2;    
constexpr int kPlayerJumpAccelFrameRate        = 53;   

constexpr int kPlayerAttackCooldownFrames      = 2;    
constexpr int kPlayerStunFrames                = 3;    
constexpr int kPlayerShakeForce                = 2;   


class Game;

//------------------------------------------------------------------------------
// Player class
//------------------------------------------------------------------------------
class Player : public Timer {
public:
    explicit Player(Game* gameInstance);
    ~Player() = default;

    /// Reset player to initial state
    void clear(); 

    /// Update & draw the player each frame
    void play(); 

    /// Invert all player sprites horizontally
    void invertSprites(); 

    /// Change to a new action (resets animation frame)
    /// @param action  new player action
    void setMovement(int action); 

    /// Process controller/keyboard input
    void handleInput(); 
    /// Process vertical jump motion (called each frame)
    void processJump(); 

    /// Execute an attack if condition is true
    /// @param condition  whether attack key pressed and off cooldown
    /// @param action     which PlayerAction attack
    void processAttack(bool condition, PlayerAction action); 

    /// Perform collision detection for active attack
    void processCollision(); 

    /// Populate the player’s attack‐hit bounding box in world coordinates.
    /// @param outX      world‐space X of the hit box
    /// @param outY      world‐space Y of the hit box
    /// @param outWidth  width of the hit box
    /// @param outHeight height of the hit box
    /// @param info      precomputed CollisionInfo for this attack
    void calculateAttackCollisionBounds(int *outX,
                                                int *outY,
                                                int *outWidth,
                                                int *outHeight,
                                                CollisionInfo info);
    
    // Public state members
    int             x{kPlayerDefaultX};
    int             oldX{0};
    bool            shakeDirRight{false};
    int             y{kPlayerDefaultY};
    int             lives{kPlayerDefaultLives};
    int             health{DEFAULT_HEALTH};
    int             bonusScore{0};
    int             life_counter{0}; //

    bool            controlsLocked{false};     ///< when stunned or mid‐attack
    bool            canAttack{true};
    bool            attackActive {false};
    bool            isInverted{false};
    bool            isShaking{false}; 
    bool            showHit_{false};
    JumpDrift       jumpDrift{JumpDrift::NoneDrift};
    int             activateTime{0};   
    PlayerAction    currAction_{PlayerAction::Default};
    PlayerAction    prevAction_{PlayerAction::None};
protected:
    Game*           game_{nullptr};
    /// Timer tick callback
    void onTimeTick() override; 
private:
    // timers & counters
    int             pauseTimer;       
    int             stunJumpTimer_{0};   
    int             jumpFrameCounter_{0};
    int             jumpAcceleration_{0};

    // movement state
    bool            isFlyingKick_{false};
    bool            canFlyKick_{true};

    /// Update all sprite positions to (x_, y_)
    void updateSpritePositions_(); 
};

//------------------------------------------------------------------------------
// Collision HitBoxes for attacks & body
//------------------------------------------------------------------------------
const CollisionInfo kCollisionPunchCrouch = {
    28, 0, 22, 3, 3, 0//26, 0, 17, 2, 2, 0
};

const CollisionInfo kCollisionKickCrouch = {
    30, 0, 27, 6, 5, 0//28, 0, 25, 4, 3, 0
};

const CollisionInfo kCollisionKickStand = {
    25, 0, 24, 6, 5, 0//23, 0, 22, 4, 3, 0
};

const CollisionInfo kCollisionAirAttack = {
    31, 0, 24, 4, 5, 0//29, 0, 22, 3, 3, 0
};

const CollisionInfo kCollisionKickHigh = {
    27, 0, 3, 5, 4, 0//25, 0, 3, 4, 4, 0
};

const CollisionInfo kCollisionPunchStand = {
    25, 0, 17, 3, 3, 0//23, 0, 13, 3, 3, 0
};

const CollisionInfo kCollisionBody = {
    8, 10, 1, 10, 32, 0//7, 9, 1, 10, 31, 0
};


//------------------------------------------------------------------------------
// Player sprite names
//------------------------------------------------------------------------------
const vector<string> playerSprites = {
    "player_default","player_crouch","player_kick_stand","player_kick_crouch","player_punch_stand"
    ,"player_punch_crouch","player_kick_fly","player_kick_high","player_defeated","player_smile"
};
#endif