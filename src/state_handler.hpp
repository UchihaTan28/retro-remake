#ifndef _STATE_H_
#define _STATE_H_

#define ENEMY_DEFAULT_X 145
#define ENEMY_DEFAULT_Y 152


#include "game_handler.hpp"
#include "other.hpp"
#include <random>

using namespace std;
#pragma once

//------------------------------------------------------------------------------
// Fundamental game‐wide constants
//------------------------------------------------------------------------------
constexpr int   StageWidth         = GAME_WIDTH;    // logical playfield width
constexpr int   StageHeight        = GAME_HEIGHT;   // logical playfield height
constexpr int   ScreenWidth        = SCREEN_WIDTH;  // window width
constexpr int   ScreenHeight       = SCREEN_HEIGHT; // window height

//------------------------------------------------------------------------------
// Enemy action states
//------------------------------------------------------------------------------
enum class EnemyAction : int {
    None       = -1,  ///< no action
    Idle       =  0,  ///< standing still
    MoveLeft   =  1,  ///< walking left
    MoveRight  =  2,  ///< walking right
    Defeated   =  3,  ///< dying animation
    Punch       =  4,  ///< kicking attack
    Kick      =  5,  ///< punching attack
    Special    =  6,  ///< boss‐only special move
    Pause      =  7   ///< temporarily frozen (e.g. on hit)
};


//------------------------------------------------------------------------------
// Stage‐layout
//------------------------------------------------------------------------------
constexpr int StageBoundary        = 10;   ///< horizontal padding from edges

//------------------------------------------------------------------------------
// Timing delays (in “EndSequence” ticks)
//------------------------------------------------------------------------------
constexpr int EndDelayHigh           = 2;  ///< longer pause
constexpr int EndDelayLow            = 1;  ///< shorter pause

//------------------------------------------------------------------------------
// Font metrics
//------------------------------------------------------------------------------
constexpr int FontCharWidth          = 8;  ///< pixel width of each glyph

/// Center `nChars` worth of text in the stage
inline constexpr int centerText(int nChars) {
    return (StageWidth / 2) - ((nChars * FontCharWidth) / 2);
}

//------------------------------------------------------------------------------
// Animation speeds (frames per second)
//------------------------------------------------------------------------------
constexpr int EnemyLogicFPS           = 21;  ///< enemy AI “tick” rate
constexpr int EnemyWalkSpriteFPS      =  3;  ///< normal walk cycle
constexpr int EnemyRunSpriteFPS       =  5;  ///< fast run cycle
constexpr int SpinningChainSpriteFPS  =  6;  ///< level-3 special weapon spin

//------------------------------------------------------------------------------
// Movement speeds (pixels per frame)
//------------------------------------------------------------------------------
constexpr int EnemyWalkSpeed          =  1;
constexpr int EnemyRunSpeed           =  3;

//------------------------------------------------------------------------------
// Distance thresholds (pixels or counts)
//------------------------------------------------------------------------------
constexpr int EnemyRunBoundary        = 30;  ///< how close to screen edge before running back
constexpr int EnemyRetreatDistance    = 10;  ///< how far to run back before chasing again

//------------------------------------------------------------------------------
// State‐machine “move” states
//------------------------------------------------------------------------------
enum class MoveState : int {
    FollowPlayer         = 0,  ///< chase the hero
    ChargeAttack         = 1,  ///< wind up an attack
    RetreatRunningLeft   = 2,  ///< running back left
    RetreatRunningRight  = 3   ///< running back right
};

//------------------------------------------------------------------------------
// End‐of‐level celebration / defeat sequence
//------------------------------------------------------------------------------
enum class EndSequence : int {
    Start        =  0,
    PlayWinSound =  1,
    ShowPunch    =  2,
    ShowLowKick1=  3,
    ShowHighKick1 =  4,
    ShowHighKick2 =  5,
    ShowLowKick2=  6,
    ShowPunch2   =  7,
    Smile        =  8,
    CountLife    =  9,
    Transition   = 10,
    GameOver     = 11
};

//------------------------------------------------------------------------------
// Enemy‐specific end sequence (when player loses)
//------------------------------------------------------------------------------
enum class EnemyEndSequence : int {
    Start      = 0,
    LieDown    = 1,
    MoveFeet   = 2,
    Transition = 3,
    GameOver   = 4
};

//------------------------------------------------------------------------------
// Forward‐declaration
//------------------------------------------------------------------------------
class Game;

//------------------------------------------------------------------------------
// Base “state” class: handles common render/tick/input framework
//------------------------------------------------------------------------------
class State: public Timer
{
    protected:
        // must be overridden by each concrete state
        virtual void handleInput()       = 0;  ///< process keys/gamepad
        virtual void init()        = 0;  ///< one-time setup
        virtual void drawStage()       = 0;  ///< draw your specific contents
        virtual void onBlinkingComplete()   = 0;  ///< when a blink finishes
        
        Game*                                   game_;             ///< back-link
        bool                                    initialized_{};  ///< has init() run?
        RenderTexture2D                         renderTexture_;      ///< offscreen target

        // per-string blink timers
        unordered_map<string, int> _frameTimer;
        unordered_map<string, int> _currFrame;
    public:
        State(Game *gm);
        virtual ~State();
        inline void beginFrame() {
            BeginDrawing();
            BeginTextureMode(renderTexture_);
            ClearBackground(BLACK);
        }
    
        inline void endFrame() {
            EndTextureMode();
            Rectangle src = {0, 0, float(GAME_WIDTH), float(-GAME_HEIGHT)};
            Rectangle dst = {
                (SCREEN_WIDTH/2.0f) - ((SCREEN_WIDTH * (float(GAME_HEIGHT)/GAME_WIDTH))/2.0f),
                0, SCREEN_WIDTH * (float(GAME_HEIGHT)/GAME_WIDTH), SCREEN_HEIGHT
            };
            DrawTexturePro(renderTexture_.texture, src, dst, {0,0}, 0, WHITE);
            EndDrawing();
        }
    
        inline void draw() {
            beginFrame();
            drawStage();
            endFrame();
        }

        void run();
        void drawText(const string &text, int x, int y, bool blink);

        /// Reset any subclass-specific members
        virtual void cleanUp();

        /// Free GPU resources
        void unloadTexture();
        
};

//------------------------------------------------------------------------------
// Intro / title screen
//------------------------------------------------------------------------------
class IntroState : public State {
    using State::State;
    protected:
        void handleInput()       override;
        void init()        override;
        void drawStage()       override;
        void onBlinkingComplete()   override;
        void onTimeTick()        override;
        
        bool blinkEnter_{};// = false;
        static constexpr int maxBlinks_ = 4;
        int blinkCount_ = 0; ///< how many times toggled so far
    public:
        bool canProceed = true;
        void cleanUp();
};

//------------------------------------------------------------------------------
// “Get ready” preview before gameplay
//------------------------------------------------------------------------------
class PreviewState: public State 
{
    using State::State;
    protected:
        void handleInput()       override { /* none */ }
        void init()        override {}
        void drawStage()       override;
        void onBlinkingComplete()   override {}
        void onTimeTick()        override;
    public:
        void cleanUp();
};

class PlayState: public State 
{
    using State::State;
    protected:
        void handleInput();
        void drawStage();
        void init();
        void onBlinkingComplete();
        void onTimeTick();
    public:
        void cleanUp();
        void reset();
        void run();

        /// Advance the “end of level” state machine for the player’s victory/loss sequence.
        void processEndState();

        /// Advance the “end of level” state machine for the enemy’s victory/loss sequence.
        void processEnemyEndState();
        int enemyHealth;
        int enemyX, enemyY;
        EnemyAction enemyCurrentMove = EnemyAction::None;

        int retreatCounter{};    ///< how far enemy has run back //enemyMovementCounter;

        /// Update the position of every non-attack enemy sprite frame.
        ///
        /// Skips attack-only frames (“hit”, “punch”, “kick”) which are placed
        /// dynamically during attack processing.
        void updateEnemySpritePositions();

        void renderEnemy();

        /// Evaluate and advance the enemy’s movement state machine.
        // This is not just raw physics but a state machine/AI step.
        void updateEnemyMovementState();

        /// “Tick” the enemy’s movement timer and call updateEnemyAI when ready
        void tickEnemyMovement();

        /// Move the enemy directly toward the player’s current position
        void enemyPursuePlayer();

        /// Choose and begin a basic attack (kick or punch) at random
        void enemyBasicAttack();

        /// @returns true if the player is within the enemy’s engagement range
        bool playerInRange();
    
        void flipEnemySprites();
        
        /// Queue up the “end‐of‐round” choreography based on the given player action
        /// @param actionID   ID of the player’s finishing move
        /// @param flipSprite whether to flip the player sprite before playing
        /// @param playSfx    whether to play a sound effect
        void prepareEndOfRoundChoreography(int actionID, bool flipSprite, bool playSfx);

        bool isEnemyFlipped = false;
        
        bool pauseMovement{};    ///< freeze all motion
        int rotatingChainX, rotatingChainY;     ///< level-3 weapon spin pos

        int haltTime;
        int haltTimeHit;
        int maxHaltTime;

        // state‐machine vars
        EndSequence endState = EndSequence::Start;
        EnemyEndSequence enemyEndState = EnemyEndSequence::Start;
        MoveState enemyMoveState = MoveState::FollowPlayer;

        int enemyRandomAttack = -1;

        /// @returns true if the last enemy attack frame collided with the player
        bool isCollidedWithPlayer();

        /// Apply the results of a collision (shake, health loss, knockback)
        void processCollisionWithPlayer();
        
        bool renderEnemyHit;
        void resetEnemyMove();

        /// Compute the player’s collision box for the current attack
        /// @param outX       world‐space X of collision box
        /// @param outY       world‐space Y of collision box
        /// @param outWidth   width of collision box
        /// @param outHeight  height of collision box
        /// @param info       precomputed CollisionInfo for this attack
        void calculatePlayerCollisionBounds(int *outX, int *outY, int *outWidth, int *outHeight, const vector<CollisionInfo>& info);

        /// Offset the enemy’s X position by delta, optionally to the right
        /// @param delta      magnitude of shift
        /// @param toRight    true = X+=delta, false = X–=delta
        void offsetEnemyX(int delta, bool toRight);

        /// Generic “run away or charge” helper: if canAdvance, dash; else reverse direction
        /// @param canAdvance   whether you’re allowed to move forward
        /// @param movingRight  intended forward direction
        void advanceOrRetreat(bool canAdvance, bool movingRight);
        
        /// Advance the enemy rapidly in the given horizontal direction
        /// @param goingRight  true = move right, false = move left
        void moveEnemyRight(bool goingRight);

        int runCounter = 0;
};

const vector<EnemyAction> attackList = {
    EnemyAction::Kick,
    EnemyAction::Punch
};


const vector<string> enemySprites = {
    "kick", "punch", "default", "defeated", "hit"
};

// collision boxes for each enemy’s body (idle / walk / run, etc.)
const vector<CollisionInfo> enemyBodyHitBoxes = {
    {5, 8, 8, 19, 31, 10},
    {6, 3, 3, 4, 31, 11},
    {10, 7, 9, 16, 31, 9},
    {8, 3, 8, 8, 31, 8},
    {11, 2, 7, 16, 31, 6}
};

// collision boxes when the enemy punches
const vector<CollisionInfo> enemyPunchHitBoxes  = {
    {0, 47, 31, 4, 2, 0},
    {6, 28, 11, 3, 2, 0},
    {0, 36, 21, 3, 3, 0},
    {0, 22, 35, 5, 3, 0},
    {6, 27, 14, 2, 3, 0}
};

// collision boxes when the enemy kicks
const vector<CollisionInfo> enemyKickHitBoxes  = {
    {0, 42, 22, 7, 4, 0},
    {0, 31, 15, 5, 2, 0},
    {0, 34, 15, 4, 1, 0},
    {0, 24, 7, 3, 5, 0},
    {0, 31, 25, 5, 3, 0}
};

// collision boxes for the level-3 spinning chain attack
const vector<CollisionInfo> chainAttackHitBoxes  = {
  // offsetLeft, y, width, height, kickAdjustment, _ (filled this with the chain’s hit‐box)
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0},
    {21, 38, 21, 3, 3, 0},
    {0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0}
};

const vector<string> enemies = {
    "wang", "tao", "chen", "lang", "mu"
};

#endif 


