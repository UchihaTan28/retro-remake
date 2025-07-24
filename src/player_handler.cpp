// player.cpp
#include "player_handler.hpp"
#include <raylib.h>

// Constructor: initialize fields via initializer list
Player::Player(Game *gm)
    : game_(gm)
    , x(kPlayerDefaultX)
    , y(kPlayerDefaultY)
    , lives(kPlayerDefaultLives)
    , controlsLocked(false)
    , pauseTimer(0)
    , stunJumpTimer_(0)
    , canAttack(true)
    , attackActive (false)
    , activateTime(0)
    , prevAction_(PlayerAction::None)
    , currAction_(PlayerAction::Default)
    , health(DEFAULT_HEALTH)
    , isInverted(false)
    , isShaking(false)
    , shakeDirRight(false)
    , isFlyingKick_(false)
    , canFlyKick_(true)
    , jumpDrift(JumpDrift::NoneDrift)
    , jumpAcceleration_(0)
    , jumpFrameCounter_(0)
    , showHit_(false)
    , life_counter(0)
{
    // Override the “normal” player sprite’s frame speed
    game_->sprites.at("player_default").setAnimationSpeed(kPlayerFrameRate);
}

void Player::clear() {
    // Reset everything back to defaults
    controlsLocked = false; canAttack = true;
    pauseTimer = stunJumpTimer_ = 0; 
    x = kPlayerDefaultX;
    y = kPlayerDefaultY;
    setMovement(0);

    prevAction_ = PlayerAction::None; health = DEFAULT_HEALTH;
    attackActive = false;
    activateTime = 0;
    showHit_ = false;
    life_counter = 0;
    // If flipped, unflip
    if (isInverted) invertSprites();
}

void Player::invertSprites() {
    // Flip all player sprites and the hit effect
    for (auto &name : playerSprites) {
        game_->sprites.at(name).invertHorizontally();
    }
    game_->sprites.at("effect_hit").invertHorizontally(); isInverted = !isInverted;
}

void Player::play() {
    // Motion‐shake effect
    if (isShaking) {
        x += shakeDirRight ? kPlayerShakeForce : -kPlayerShakeForce;
        shakeDirRight = !shakeDirRight;
    }
    // Update every sprite's position
    for (auto &name : playerSprites) {
        auto &spr = game_->sprites.at(name);
        spr.x = x;
        spr.y = y;
    }

    // Draw based on currentMovement
    auto &normal = game_->sprites.at("player_default");
    switch (currAction_) {
        case PlayerAction::WalkLeft:
        case PlayerAction::WalkRight:
            normal._isPaused = game_->playState->renderEnemyHit;
            normal.updateAndDraw();
            break;
        case PlayerAction::PunchStand:
            game_->sprites.at("player_punch_stand").draw();
            break;
        case PlayerAction::PunchCrouch:
            game_->sprites.at("player_punch_crouch").draw();
            break;    
        case PlayerAction::Crouch:
        case PlayerAction::JumpUp:
        case PlayerAction::JumpDown:
            if (isFlyingKick_)
                game_->sprites.at("player_kick_fly").draw();
            else
                game_->sprites.at("player_crouch").draw();
            break;
        case PlayerAction::DefaultHold:
            normal.drawFrame(1);
            break;
        case PlayerAction::KickStand:
            game_->sprites.at("player_kick_stand").draw();
            break;
        case PlayerAction::KickHigh:
            game_->sprites.at("player_kick_high").draw();
            break;    
        case PlayerAction::KickCrouch:
            game_->sprites.at("player_kick_crouch").draw();
            break;
        case PlayerAction::Smile:
            game_->sprites.at("player_smile").draw();
            break;
        case PlayerAction::Defeated:
            if (game_->sprites.at("player_defeated").updateAndDraw()) {
                PlaySound(game_->sounds.at("twitch_feet"));
                if (++life_counter == 3) {
                    setMovement(14);
                    game_->playState->enemyEndState = EnemyEndSequence::Transition;
                }
            }
            break;    
        case PlayerAction::VeryDefeated:
            game_->sprites.at("player_defeated").drawFrame(0);
            break;
        default:  // PlayerAction::Default
            normal.drawFrame(0);
    }

    // Draw hit effect if needed
    if (showHit_) {
        game_->sprites.at("effect_hit").draw();
    }

    // Auto‐flip to face enemy if not mid‐air attack
    if (!isFlyingKick_ && game_->playState->endState <= EndSequence::Start) {
        bool shouldFlip = (game_->playState->enemyX < x) != isInverted;
        if (shouldFlip) invertSprites();
    }
}

void Player::onTimeTick() {
    // Handle stun and flying‐kick timing
    if (controlsLocked && !game_->playState->renderEnemyHit) {
        if (currAction_ != PlayerAction::JumpUp && currAction_ != PlayerAction::JumpDown) {
            if (++pauseTimer == 3) {
                controlsLocked  = false; showHit_        = false;
                pauseTimer       = 0;
                activateTime   = 0;attackActive = true;
                // If they were holding down, stay crouched
                if (IsKeyDown(KEY_DOWN) && prevAction_ == PlayerAction::Crouch)
                    setMovement(4);
                else
                    setMovement(0);
            }
        } else if (isFlyingKick_ && ++stunJumpTimer_ == 2) {
                isFlyingKick_  = false; stunJumpTimer_  = 0;
        }
    }
    // Attack cooldown
    if (attackActive && ++activateTime == kPlayerAttackCooldownFrames) {
        activateTime   = 0; attackActive = false;
        canAttack      = true;
    }
}

void Player::setMovement(int move) {
    // Prevent going from defeated back to idle
    if (currAction_ == PlayerAction::Defeated && move == 0) return;
    prevAction_    = currAction_;
    currAction_ = static_cast<PlayerAction>(move);
}

void Player::handleInput() {
    // Only in active game_, and not stunned or paused
    if (game_->state != GameState::Play || controlsLocked
        || game_->playState->renderEnemyHit || game_->playState->pauseMovement)
        return;

    bool left  = IsKeyDown(KEY_LEFT), right = IsKeyDown(KEY_RIGHT);

    // Horizontal movement
    if (x > StageBoundary && left) {
        setMovement(2);
        x -= kPlayerSpeed;
    }
    else if (x < GAME_WIDTH - StageBoundary - game_->sprites.at("player_default").getTexture().width/2 && right) {
        setMovement(3);
        x += kPlayerSpeed;
    }
    else {
        // If at right edge, switch to idle_2, else idle
        setMovement((x >= GAME_WIDTH-StageBoundary && right) ? 1 : 0);
    }

    // Crouch
    if (IsKeyDown(KEY_DOWN)) {
        setMovement(4);
    }

    // Jump
    if (IsKeyDown(KEY_UP)) {
        jumpDrift = left ? JumpDrift::LeftDrift
                     : right ? JumpDrift::RightDrift
                             : JumpDrift::NoneDrift;
        setMovement(10);
        controlsLocked    = true;
        jumpAcceleration_ = kPlayerJumpAccelFrameRate;
    }

    // Helper lambda for attacks
    auto doAttack = [&](bool cond, int mv){
        if (cond) {
            controlsLocked = true;
            canAttack     = false;
            setMovement(mv);
            processCollision();
        }
    };

    // Three attack types
    doAttack(IsKeyDown(KEY_A) && canAttack,
             IsKeyDown(KEY_DOWN) ? 6 : 5);
    doAttack(IsKeyDown(KEY_S) && canAttack && (left||right),
             9);
    doAttack(IsKeyDown(KEY_S) && canAttack,
             IsKeyDown(KEY_DOWN) ? 8 : 7);

    // Release A/S to re‐enable next attack
    if ((IsKeyReleased(KEY_A) || IsKeyReleased(KEY_S)) && !attackActive && !showHit_) {
        activateTime   = 0; attackActive = true;
    }

    // Mid‐air flying kick
    if (IsKeyDown(KEY_S) && canFlyKick_
        && (currAction_ == PlayerAction::JumpUp || currAction_ == PlayerAction::JumpDown)
        && y <= (kPlayerJumpHeight + 23))
    {
        isFlyingKick_  = true;
        canFlyKick_ = false; stunJumpTimer_  = 0;
        processCollision();
    }
}

void Player::processJump() {
    // Vertical jump motion + horizontal drift
    if ((currAction_ == PlayerAction::JumpUp || currAction_ == PlayerAction::JumpDown)
        && health > 0
        && !game_->playState->renderEnemyHit)
    {
        if (++jumpFrameCounter_ >= (TARGET_FPS / jumpAcceleration_)) {
            jumpFrameCounter_ = 0;

            // Horizontal drift
            if (jumpDrift == JumpDrift::LeftDrift && x > StageBoundary) {
                x -= kPlayerJumpSpeed;
            }
            else if (jumpDrift == JumpDrift::RightDrift
                && x < GAME_WIDTH - StageBoundary - game_->sprites.at("player_default").getTexture().width/2)
            {
                x += kPlayerJumpSpeed;
            }

            // Ascend or descend
            if (currAction_ == PlayerAction::JumpUp) {
                if (y > kPlayerJumpHeight) {
                    jumpAcceleration_--;
                    y -= kPlayerJumpSpeed;
                    return;
                }
                setMovement(11);
                return;
            }
            if (y < kPlayerDefaultY) {
                if (jumpAcceleration_ < kPlayerJumpAccelFrameRate)
                    jumpAcceleration_++;
                y += kPlayerJumpSpeed;
                return;
            }
            // Landed
            y = kPlayerDefaultY;
            setMovement(0);
            controlsLocked = false; isFlyingKick_  = false;
            canFlyKick_ = true;
        }
    }
}

void Player::calculateAttackCollisionBounds(int *pX, int *pY, int *pW, int *pH, CollisionInfo col) {
    *pX = isInverted ? x : (x + col.offsetLeft);
    *pW = col.boxWidth;
    *pH = col.boxHeight;
    *pY = y + col.offsetTop;
}

void Player::processCollision() {
    // Pick correct collision box based on movement
    CollisionInfo col = kCollisionPunchStand;
    int bonus = 100;
    switch (currAction_) {
        case PlayerAction::PunchCrouch: col = kCollisionPunchCrouch;    break;
        case PlayerAction::KickHigh: col = kCollisionKickHigh; bonus=200; break;
        case PlayerAction::KickCrouch:  col = kCollisionKickCrouch;     break;
        case PlayerAction::KickStand:col = kCollisionKickStand;   break;
        case PlayerAction::JumpDown:
            col = kCollisionAirAttack; bonus=250; break;
        case PlayerAction::JumpUp:
        default: break;
    }

    int pX, pY, pW, pH;
    calculateAttackCollisionBounds(&pX, &pY, &pW, &pH, col);

    auto *st = game_->playState;
    int idx      = game_->levelIndex();      // one cast, centrally
    int eX = st->enemyX + (st->isEnemyFlipped
                ? enemyBodyHitBoxes[idx].offsetRight
                : enemyBodyHitBoxes[idx].offsetLeft);
    int eY = st->enemyY + enemyBodyHitBoxes[idx].offsetTop;
    int eW = enemyBodyHitBoxes[idx].boxWidth;
    int eH = enemyBodyHitBoxes[idx].boxHeight;

    // AABB test
    if (pX > eX+eW-1 || eX > pX+pW-1 || pY > eY+eH-1 || eY > pY+pH-1) {
        PlaySound(game_->sounds.at("attack"));
        return;
    }

    // Hit!
    PlaySound(game_->sounds.at("collision"));
    showHit_            = true;
    game_->score       += bonus;
    game_->sprites.at("effect_hit").x = pX;
    game_->sprites.at("effect_hit").y = pY;
    st->haltTime       = 0;
    st->pauseMovement  = true;
}
