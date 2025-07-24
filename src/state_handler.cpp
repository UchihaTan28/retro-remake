#include "state_handler.hpp" 
#include <vector>
#include <algorithm>
#include <cstring>

//------------------------------------------------------------------------------
// file-local random helpers (only used inside this translation unit)
//------------------------------------------------------------------------------
namespace {
    // Returns a thread-local mersenne twister engine
    std::mt19937& localRng() {
      static thread_local std::mt19937 rng{std::random_device{}()};
      return rng;
    }
  
    int randBetween(int min, int max) {
      std::uniform_int_distribution<int> dist(min, max);
      return dist(localRng());
    }
  }

  State::~State() {
    // base cleanup: reset timers, free render texture
    cleanUp();
    unloadTexture();
}

//------------------------------------------------------------------------------
// State base class – ctor / dtor / main loop / cleanup
//------------------------------------------------------------------------------
State::State(Game *gm)
: game_(gm), initialized_(false) {
    // One-time init, then input → draw → tick each frame
    cleanUp();
    renderTexture_ = LoadRenderTexture(GAME_WIDTH, GAME_HEIGHT);
}

void State::run()
{
    if (!initialized_) {
        init();
        initialized_ = true;
    }
    handleInput();
    draw();
    timeTick();
}

void State::unloadTexture()
{
    UnloadRenderTexture(renderTexture_);
}

void State::cleanUp()
{
    // Reset timer and init flag
    frameCounter_ = 0; elapsedSeconds_ = 0;
    initialized_ = false;
    
}

//------------------------------------------------------------------------------
// drawText: render a blinking ASCII string via your sprite font
//------------------------------------------------------------------------------
void State::drawText(const string &text, int x, int y, bool blink)
{
    // initialize blink timers on first use
    if (!_currFrame.count(text)) {
        _currFrame[text]   = 0;
        _frameTimer[text]  = 0;
    }

    // advance blink counter if requested
    if (blink) {
        auto &t = _frameTimer[text];
        if (++t >= TARGET_FPS/FRAME_SPEED) {
            t = 0;
            if (++_currFrame[text] > 1) {
                _currFrame[text] = 0;
                onBlinkingComplete();
            }
        }
    }
    
    int blankIdx = spriteLetters.size() - 2;

    // draw each character
    for (char ch : text) {
        // find its frame in the sheet
        auto it = find(spriteLetters.begin(), spriteLetters.end(), ch);
        if (it == spriteLetters.end()) {
            x += FontCharWidth;// unknown glyph → skip
            continue;
        }
        int idx = distance(spriteLetters.begin(), it);

        // point our sprite at the right place on screen
        auto &font = game_->sprites.at("font_symbols");
        font.x = x;
        font.y = y;

        // blink logic: blank vs real
       if (blink &&  _currFrame[text] != 1){
            font.drawFrame(blankIdx);
        }
       else {
            font.drawFrame(idx);
       }
        x += FontCharWidth;
    }
}

//------------------------------------------------------------------------------
// IntroState: title screen
//------------------------------------------------------------------------------
void IntroState::init()
{
    // set konami logo to center
    auto &logoSprite= game_->sprites.at("logo_konami");
    logoSprite.x = (GAME_WIDTH / 2) - (game_->sprites.at("logo_konami").getTexture().width / 2);
    logoSprite.y = 30;
    
    // set game_name to center
    auto &gameNameSprite= game_->sprites.at("game_name");
    gameNameSprite.x = (GAME_WIDTH / 2) - (game_->sprites.at("game_name").getTexture().width / 2);
    gameNameSprite.y = 75;
}

void IntroState::handleInput()
{
    // Wait for ENTER to start blinking, then allow proceed
    if (IsKeyReleased(KEY_ENTER))
    {
        canProceed = true;
    }
    else if(IsKeyDown(KEY_ENTER) && canProceed && !blinkEnter_)
    {
        blinkEnter_ = true;
        PlayMusicStream(game_->musics.at("main_music"));
    }
}

void IntroState::drawStage()
{
    // Draw Konami logo and game title
    game_->sprites.at("logo_konami").draw();
    game_->sprites.at("game_name").draw();

    // Copyright & “other” text
    drawText(COPYRIGHT_TEXT,
             (GAME_WIDTH/2) - ((std::strlen(COPYRIGHT_TEXT) * FontCharWidth)/2),
             98,
             false);
    drawText(OTHER_TEXT,
             centerText(std::strlen(OTHER_TEXT)),
             108,
             false);

    // “Press Enter to begin” blinking prompt
    drawText(TO_START_TEXT,
             centerText(std::strlen(TO_START_TEXT)),
             118,
             blinkEnter_);


    const char* controlsTitle = " controls";
    drawText(controlsTitle,
             centerText(std::strlen(controlsTitle)),
             130,
             false);

    drawText(" left - left arrow",
             centerText(std::strlen(" left - left arrow")),
             145,
             false);

    drawText(" right - right arrow",
             centerText(std::strlen(" right - right arrow")),
             166,
             false);

    drawText(" jump - up arrow",
             centerText(std::strlen(" jump - up arrow")),
             182,
             false);

    drawText(" crouch - down arrow",
             centerText(std::strlen(" crouch - down arrow")),
             198,
             false);

    drawText(" kick - s",
             centerText(std::strlen(" kick - s")),
             214,
             false);

    drawText(" punch - a",
             centerText(std::strlen(" punch - a")),
             225,
             false);

    drawText(" quit - escape",
             centerText(std::strlen(" quit - escape")),
             245,
             false);

    // Continue streaming background music
    UpdateMusicStream(game_->musics.at("main_music"));
}


/*void IntroState::drawStage()
{
    game_->sprites.at("logo_konami").draw();
    game_->sprites.at("game_name").draw();

    //draw copyright mark
    drawText(COPYRIGHT_TEXT, (GAME_WIDTH / 2) - ((std::strlen(COPYRIGHT_TEXT) * FontCharWidth) / 2), 95, false);
    drawText(OTHER_TEXT, centerText(strlen(OTHER_TEXT)), 105, false);

    // press enter to begin
    drawText(TO_START_TEXT, centerText(strlen(TO_START_TEXT)), 155, blinkEnter_);

    UpdateMusicStream(game_->musics.at("main_music"));
}*/

void IntroState::onBlinkingComplete()
{
    // Advance blink count until we switch to Preview
    if (blinkCount_ == maxBlinks_)
    {
        game_->state = GameState::Preview;
        this->cleanUp();
        return;
    }
    blinkCount_ ++;
}

void IntroState::cleanUp()
{
    blinkCount_ = 0; blinkEnter_ = false;
    State::cleanUp();
}

void IntroState::onTimeTick(){}

//------------------------------------------------------------------------------
// PreviewState: “Get ready” screen before PlayState
//------------------------------------------------------------------------------
void PreviewState::drawStage()
{
    UpdateMusicStream(game_->musics.at("main_music"));
    drawText(
        "stage 0" + to_string(game_->level), 
        centerText(8),
       centerText(1),
        false
    );
}


void PreviewState::onTimeTick() {
    // after 10 ticks, move on to PlayState
    if (elapsedSeconds_ == 10) {
        game_->state = GameState::Play;
        cleanUp();
    }
}
void PreviewState::cleanUp() {
    State::cleanUp();
}

//------------------------------------------------------------------------------
// PlayState: actual gameplay state
//------------------------------------------------------------------------------
void PlayState::init()
{
    game_->sprites.at("life_icon").y = 45;

    game_->sprites.at("hud_health").y = 205;
    game_->sprites.at("hud_health").x = (GAME_WIDTH / 2) - (game_->sprites.at("hud_health").getTexture().width / 2);

    game_->sprites.at("green_health").y = 208;
    game_->sprites.at("red_health").y = 208;

    game_->sprites.at("spinning_chain").setAnimationSpeed(SpinningChainSpriteFPS);

    reset();
}

void PlayState::run()
{
    State::run();
    if (!pauseMovement)
    {
        game_->player->timeTick();
        game_->player->processJump();
    }

    if (!game_->player->showHit_ && game_->player->health > 0 && enemyHealth > 0)
        tickEnemyMovement();
}

void PlayState::handleInput()
{
    if (game_->player->health > 0 && enemyHealth > 0)
        game_->player->handleInput();

    // Restart on ENTER after game over    
    if (((game_->playState->enemyEndState == EnemyEndSequence::GameOver) || (game_->playState->endState == EndSequence::GameOver)) && IsKeyDown(KEY_ENTER))
    {
        cleanUp();
        game_->state = GameState::Intro;
        game_->score = 0; game_->level = 1;
        game_->player->lives = kPlayerDefaultLives;
        game_->introState->canProceed = false;
    }
}

void PlayState::drawStage()
{
    // draw background, HUD, text labels, health bars, sprites, etc.
    UpdateMusicStream(game_->musics.at("main_music"));

    // background is the last to draw
    game_->sprites.at("bg_dojo").draw();

    drawText(OTHER_TEXT, centerText(strlen(OTHER_TEXT)), 24, false);

    drawText(
        "stage-0" + to_string(game_->level), 
        165, 
        38, 
        false
    );

    drawText("score", 22, 38, false);
    drawText(to_string(game_->score), 22, 46, false);

    drawText("version", centerText(7), 38, false);
    drawText(VERSION, centerText(5), 46, false);

    game_->sprites.at("life_icon").x = 165;
    for (int x = 0; x < game_->player->lives; x++)
    {
        game_->sprites.at("life_icon").draw();
        game_->sprites.at("life_icon").x += 8;
    }

     // HUD + health bars, collision, rendering, etc.
    drawText("player", 46, (GAME_HEIGHT - 24), false);
    drawText(enemies[game_->level - 1], (208 - (enemies[game_->level - 1].size() * 8)), (GAME_HEIGHT - 24), false);

    game_->sprites.at("hud_health").draw();

    game_->sprites.at("green_health").x = 104;
    game_->sprites.at("red_health").x = 104;

    //draw player's health gauge
    for (int x = 0; x < game_->player->health; x++)
    {
        string h_hud = (game_->player->health > LOW_HEALTH)? "green" : "red";
        game_->sprites.at(h_hud+ "_health").draw();
        game_->sprites.at(h_hud+ "_health").x -= 8;
    }

    //draw health gauge
    game_->sprites.at("green_health").x = 144;
    game_->sprites.at("red_health").x = 144;

    for (int x = 0; x < enemyHealth; x++)
    {
        string h_hud = (enemyHealth > LOW_HEALTH)? "green" : "red";
        game_->sprites.at(h_hud+ "_health").draw();
        game_->sprites.at(h_hud + "_health").x += 8;
    }

    // show enemy
    renderEnemy();

    // show player
    game_->player->play();

    if (renderEnemyHit)
        game_->sprites.at(enemies[game_->level - 1] + "_hit").draw();


    if (game_->playState->endState == EndSequence::GameOver || game_->playState->enemyEndState == EnemyEndSequence::GameOver)
    {
        drawText("game_over", centerText(9), centerText(1), false);

        drawText(
            (game_->playState->endState == EndSequence::GameOver )? "you win": "you lose", 
            ((game_->playState->endState == EndSequence::GameOver )? centerText(7): centerText(8)),
            centerText(1)+8,
            false
        );
    }
}

void PlayState::tickEnemyMovement()
{
    retreatCounter++;
    if (retreatCounter >= (TARGET_FPS / EnemyLogicFPS))
    {
        retreatCounter = 0;
        updateEnemyMovementState();
    }
}

void PlayState::enemyPursuePlayer()
{
    if (enemyX > game_->player->x)
        offsetEnemyX(EnemyWalkSpeed, false);
    if (enemyX < game_->player->x)
        offsetEnemyX(EnemyWalkSpeed, true);
}

void PlayState::enemyBasicAttack()
{
    enemyMoveState = MoveState::ChargeAttack;
    enemyRandomAttack = randBetween(0, 1);
    enemyCurrentMove = attackList[enemyRandomAttack];
}

bool PlayState::playerInRange()
{
    int boundary = (game_->sprites.at("player_default").getTexture().width / game_->sprites.at("player_default").getTileCount()) + 10;

    return (enemyX >= game_->player->x - boundary
            && isEnemyFlipped)
            ||
            (enemyX <= game_->player->x + boundary
            && !isEnemyFlipped);
}

void PlayState::offsetEnemyX(int amount, bool isAdd)
{
    enemyX = (isAdd)? enemyX + amount : enemyX - amount;
    if (game_->level == 3)
        rotatingChainX = (isAdd)? rotatingChainX + amount : rotatingChainX - amount;   
}

void PlayState::moveEnemyRight(bool goingRight) {
    // pre‐compute your left/right limits:
    const int leftLimit  = StageBoundary + EnemyRunBoundary;
    const int rightLimit = GAME_WIDTH 
        - (StageBoundary + EnemyRunBoundary)
        - (game_->sprites.at("player_default").getTexture().width / 2);

    if (runCounter > EnemyRetreatDistance) {
        // when done backing off, go back to follow and reset speed
        enemyMoveState = MoveState::FollowPlayer;
        game_->sprites
            .at(enemies[game_->level - 1] + "_default")
            .setAnimationSpeed(EnemyWalkSpriteFPS);
    }
    if ((goingRight && enemyX < rightLimit) ||
             (!goingRight && enemyX > leftLimit)) {
        // run in chosen direction
        offsetEnemyX(EnemyRunSpriteFPS, goingRight);
        runCounter++;
    }
    else {
        // hit the wall, switch to running‐animation facing the other way
        enemyMoveState = goingRight 
            ? MoveState::RetreatRunningLeft: MoveState::RetreatRunningRight;
    }
}

void PlayState::updateEnemyMovementState()
{
    switch(enemyMoveState)
    {
        case MoveState::ChargeAttack:
            break;
        case MoveState::RetreatRunningLeft:
            enemyCurrentMove = EnemyAction::Idle;
            moveEnemyRight(false);
            //enemyRunLeft();
            break;
        case MoveState::RetreatRunningRight:
            enemyCurrentMove = EnemyAction::Idle;
            moveEnemyRight(true);
            break;
        default:
            enemyPursuePlayer();

            if (playerInRange())
            {
                enemyBasicAttack();
            }

            break;
    }
}

void PlayState::onBlinkingComplete(){}

void PlayState::cleanUp()     { reset(); game_->player->clear(); State::cleanUp(); }


void PlayState::onTimeTick()
{
    if (pauseMovement)
    {
        haltTime += 1;

        if (haltTime == 2)
        {
            pauseMovement = false;
            haltTime = 0;

            if (game_->player->currAction_ == PlayerAction::JumpDown || game_->player->currAction_ == PlayerAction::JumpUp)
            {
                game_->player->showHit_ = false;
                game_->player->attackActive = true;
                game_->player->activateTime = 0;
            }

            enemyHealth -= 1;
            if (enemyHealth == 0)
            {
                StopMusicStream(game_->musics.at("main_music"));
                haltTime = 0;
            }
            else
            {
                if (enemyMoveState != MoveState::RetreatRunningLeft && enemyMoveState != MoveState::RetreatRunningRight)
                {
                    runCounter = 0;
                    enemyMoveState = (!isEnemyFlipped)? MoveState::RetreatRunningRight : MoveState::RetreatRunningLeft;
                    game_->sprites.at(enemies[game_->level - 1] + "_default").setAnimationSpeed(EnemyRunSpriteFPS);   
                }
            }
        }
    }

    if (enemyHealth == 0)
    {
        haltTime += 1;
        if (haltTime == maxHaltTime)
        {
            processEndState();
            haltTime = 0;
        }
    }

    if (enemyHealth != 0 && game_->player->health == 0 )
    {
        haltTime ++;
        if (haltTime == maxHaltTime)
        {
            processEnemyEndState();
            haltTime = 0;
        }
    }

    if (renderEnemyHit)
    {
        haltTimeHit ++;
        if (haltTimeHit == 4)
        {
            haltTimeHit = 0;
            renderEnemyHit = false;
            resetEnemyMove();

            game_->player->x = game_->player->oldX;
            game_->player->isShaking = false;

            if (
                (game_->player->currAction_ == PlayerAction::WalkRight && !IsKeyDown(KEY_RIGHT))
                || (game_->player->currAction_ == PlayerAction::WalkLeft && !IsKeyDown(KEY_LEFT))
                || (game_->player->currAction_ == PlayerAction::Crouch && !IsKeyDown(KEY_DOWN))
            )
            {
                game_->player->setMovement(0);
            }

            if (game_->player->health == 0)
            {
                enemyCurrentMove = EnemyAction::Pause;
            }
        }
    }
}

void PlayState::processEnemyEndState()
{
    switch(enemyEndState)
    {
        case EnemyEndSequence::LieDown:
            game_->player->setMovement(13);
            game_->player->y = kPlayerDefaultY;
            game_->sprites.at("player_defeated").resetAnimation();
            PlaySound(game_->sounds.at("defeated"));
            enemyEndState = EnemyEndSequence::MoveFeet;
            break;
        case EnemyEndSequence::MoveFeet:
            break;
        case EnemyEndSequence::GameOver:
            break;
        case EnemyEndSequence::Transition:
            if (game_->player->lives > 0)
            {
                game_->player->lives --;
                game_->state = GameState::Preview;
                PlayMusicStream(game_->musics.at("main_music"));
                cleanUp();
                return;
            }
            PlaySound(game_->sounds.at("game_over"));
            enemyEndState = EnemyEndSequence::GameOver;
            break;
        default:
            // END_STATE_ENEMY_START
            enemyEndState = EnemyEndSequence::LieDown;
            enemyCurrentMove = EnemyAction::Pause;
            StopMusicStream(game_->musics.at("main_music"));
            game_->player->life_counter = 0;
            break;
    }
}

void PlayState::processEndState()
{
    //stage win conclusion action flow: p -> hk-> lk -> lk -> hk -> p
    switch(endState)
    {
        case EndSequence::PlayWinSound:
            PlaySound(game_->sounds.at("win"));
            endState = EndSequence::ShowPunch;
            break;
        case EndSequence::ShowPunch:
            prepareEndOfRoundChoreography(5, false, true);
            break;
        case EndSequence::ShowLowKick1:
            prepareEndOfRoundChoreography(8, true, true);
            break;
        case EndSequence::ShowLowKick2:
            prepareEndOfRoundChoreography(8, true, true);
            break;    
        case EndSequence::ShowHighKick1:
            prepareEndOfRoundChoreography(9, true, true);
            break;
        case EndSequence::ShowHighKick2:
            prepareEndOfRoundChoreography(9, true, true);
            break;
        case EndSequence::ShowPunch2:
            prepareEndOfRoundChoreography(5, true, true);
            break;
        case EndSequence::Smile:
            prepareEndOfRoundChoreography(12, true, false);
            break;
        case EndSequence::CountLife:
            maxHaltTime = EndDelayLow;
            if (game_->player->health > 0)
            {
                game_->player->health -= 1;
                PlaySound(game_->sounds.at("counting"));
                game_->score += 100;
                return;
            }
            endState = EndSequence::Transition;
            break;
        case EndSequence::Transition:
            maxHaltTime = EndDelayHigh;
            if (game_->level == 5)
            {
                PlaySound(game_->sounds.at("game_over"));
                endState = EndSequence::GameOver;
                return;
            }
            cleanUp();
            game_->level ++;
            game_->state = GameState::Preview;
            PlayMusicStream(game_->musics.at("main_music"));
            break;
        case EndSequence::GameOver:
            break;
        default:
            // END_STATE_START
            enemyCurrentMove = EnemyAction::Defeated;
            PlaySound(game_->sounds.at("defeated"));
            endState = EndSequence::PlayWinSound;
            break;
    }
}

void PlayState::prepareEndOfRoundChoreography(int pMove, bool flip, bool playSound)
{
    if (flip)
        game_->player->invertSprites();
    game_->player->setMovement(pMove);
    if (playSound)
        PlaySound(game_->sounds.at("attack"));
    
    // Advance to the next state, without wrapping past GameOver:
    if (endState != EndSequence::GameOver) {
        endState = static_cast<EndSequence>(
            static_cast<int>(endState) + 1
        );
    }
}

void PlayState::updateEnemySpritePositions()
{
    const string& id = enemies[game_->level - 1];
    for(int x = 0; x < enemySprites.size() ; x++)
    {
        if (enemySprites[x] == "hit" || enemySprites[x] == "punch" || enemySprites[x] == "kick")
            continue;
        game_->sprites.at(id + "_" + enemySprites[x]).x = enemyX;
        game_->sprites.at(id + "_" + enemySprites[x]).y = enemyY;
    }
}

void PlayState::calculatePlayerCollisionBounds(int *outX, int *outY, int *outBoxWidth, int *outBoxHeight, const vector<CollisionInfo> &collisionInfo)
{
    *outX = (isEnemyFlipped)? ((enemyX - enemyBodyHitBoxes[game_->level - 1].kickAdjustment) + collisionInfo[game_->level - 1].offsetRight) : ((enemyX - enemyBodyHitBoxes[game_->level - 1].kickAdjustment) + collisionInfo[game_->level - 1].offsetLeft);
    *outY = (enemyY + collisionInfo[game_->level - 1].offsetTop);
    *outBoxWidth = collisionInfo[game_->level - 1].boxWidth;
    *outBoxHeight = collisionInfo[game_->level - 1].boxHeight;
}

bool PlayState::isCollidedWithPlayer()
{
    int pX = (game_->player->isInverted)? (game_->player->x + kCollisionBody.offsetRight) : (game_->player->x + kCollisionBody.offsetLeft);
    int pY = (game_->player->y + kCollisionBody.offsetTop);
    int lowerX1 = kCollisionBody.boxWidth - 1 + pX;
    int lowerY1 = kCollisionBody.boxHeight - 1 + pY;

    int lowerX2 = 0, outX=0;
    int lowerY2 = 0, outY=0;
    int outBoxWidth = 0;
    int outBoxHeight = 0;
    
    switch(enemyCurrentMove)
    {
        case EnemyAction::Kick:
            calculatePlayerCollisionBounds(&outX, &outY, &outBoxWidth, &outBoxHeight, enemyKickHitBoxes);
            break;
        case EnemyAction::Punch:
            calculatePlayerCollisionBounds(&outX, &outY, &outBoxWidth, &outBoxHeight, enemyPunchHitBoxes );
            break;
        default:
            //ENEMY_MOVE_SPECIAL
            break;
    }

    lowerX2 = outBoxWidth - 1 + outX ;
    lowerY2 = outBoxHeight - 1 + outY;

    if (lowerX1 < outX || lowerX2 < game_->player->x  || lowerY1 < outY || lowerY2 < game_->player->y )
    {
        return false;
    }
    // collision
    game_->sprites.at(enemies[game_->level - 1] + "_hit").x = outX;
    game_->sprites.at(enemies[game_->level - 1] + "_hit").y = outY;
    return true;
}

void PlayState::resetEnemyMove()
{
    enemyCurrentMove = EnemyAction::Idle;
    enemyMoveState = MoveState::FollowPlayer;
    game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[enemyRandomAttack]).resetAnimation();
}

void PlayState::processCollisionWithPlayer()
{
    enemyCurrentMove = EnemyAction::Pause;
    renderEnemyHit = true;
    PlaySound(game_->sounds.at("collision2"));
    haltTimeHit = 0;

    game_->player->oldX = game_->player->x;
    game_->player->shakeDirRight = true; game_->player->isShaking = true;

    game_->player->health --;

    if (game_->player->health == LOW_HEALTH)
    {
        PlaySound(game_->sounds.at("health_low"));
    }

    if (!isEnemyFlipped)
    {
        offsetEnemyX(EnemyWalkSpeed, true);
        return;
    }
    offsetEnemyX(EnemyWalkSpeed, false);
}

void PlayState::renderEnemy()
{
    updateEnemySpritePositions();

    switch(enemyCurrentMove)
    {
        case EnemyAction::MoveLeft:
            break;
        case EnemyAction::Defeated:
            game_->sprites.at(enemies[game_->level - 1] + "_defeated").draw();
            break;
        case EnemyAction::Kick:
        case EnemyAction::Punch:
            game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[enemyRandomAttack]).y = enemyY;
            game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[enemyRandomAttack]).x = enemyX - enemyBodyHitBoxes[game_->level - 1].kickAdjustment;
            game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[enemyRandomAttack])._isPaused = game_->player->showHit_;
            
            if (game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[enemyRandomAttack]).updateAndDraw()) // if last frame
            {
                if (!isCollidedWithPlayer())
                {
                    resetEnemyMove();
                }
                if(isCollidedWithPlayer() && game_->player->health > 0)
                {
                    // collision with player
                    processCollisionWithPlayer();
                }
            }
            break;
        case EnemyAction::Pause:
            game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[enemyRandomAttack]).drawFrame(1);
            break;
        default:

            if (game_->level == 3)
            {
                auto &spinningChainSprite = game_->sprites.at("spinning_chain");
                spinningChainSprite.x = rotatingChainX;
                spinningChainSprite.y = rotatingChainY;
                spinningChainSprite._isPaused = game_->player->showHit_;
                spinningChainSprite.updateAndDraw();
            }

            game_->sprites.at(enemies[game_->level - 1] + "_default")._isPaused = game_->player->showHit_;
            game_->sprites.at(enemies[game_->level - 1] + "_default").updateAndDraw();

            // check collision on *every* frame (or only on lastFrame, your choice)
            if (isCollidedWithPlayer())
                processCollisionWithPlayer();

            break;
    }

    //flip checker
    if (enemyX < (game_->player->x)  && !isEnemyFlipped && enemyCurrentMove != EnemyAction::Punch && enemyCurrentMove != EnemyAction::Kick )
    {
        flipEnemySprites();
    }
    if (enemyX > game_->player->x  && isEnemyFlipped && enemyCurrentMove != EnemyAction::Punch && enemyCurrentMove != EnemyAction::Kick )
    {
        flipEnemySprites();
    }
}

void PlayState::flipEnemySprites()
{
    for(int x = 0; x < enemySprites.size(); x++)
    {
        game_->sprites.at(enemies[game_->level - 1] + "_" + enemySprites[x]).invertHorizontally();   
    }
    game_->sprites.at("spinning_chain").invertHorizontally();
    isEnemyFlipped = !isEnemyFlipped;

    if (isEnemyFlipped)
    {
        rotatingChainX -= 17;
        return;
    }
    rotatingChainX += 17;
}

void PlayState::reset()
{
    enemyCurrentMove = EnemyAction::Idle;
    enemyX = ENEMY_DEFAULT_X;
    enemyY = ENEMY_DEFAULT_Y; enemyHealth = DEFAULT_HEALTH;
    pauseMovement = false;
    haltTimeHit = 0; haltTime = 0;
    maxHaltTime = EndDelayHigh;
    retreatCounter = 0;
    
    rotatingChainY = 153; rotatingChainX = 135;
    
    renderEnemyHit = false; endState = EndSequence::Start;
    enemyEndState = EnemyEndSequence::Start;
    enemyMoveState = MoveState::FollowPlayer;

    if (isEnemyFlipped)
    {
        flipEnemySprites();
    }
}
