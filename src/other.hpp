#ifndef OTHER_HPP
#define OTHER_HPP

#include "settings.hpp"   // for TARGET_FPS, FRAME_SPEED

//------------------------------------------------------------------------------
// Timer: base class for per-second callbacks driven by a fixed FPS.
//------------------------------------------------------------------------------
class Timer {
protected:
    /// Called once per second (after 59 ticks).
    virtual void onTimeTick() = 0;

    int frameCounter_   = 0;  ///< frames since last “second”
    int elapsedSeconds_ = 0;  ///< seconds elapsed (modulo 60)

public:
    /// Advance the timer by one frame; calls onTimeTick() each second.
    inline void timeTick() {
        frameCounter_++;
        if (frameCounter_ >= (TARGET_FPS / FRAME_SPEED)) {
            frameCounter_ = 0;
            if (elapsedSeconds_ == 59) {
                elapsedSeconds_ = 0;
                onTimeTick();
                return;
            }
            elapsedSeconds_++;
            onTimeTick();
        }
    }
};

//------------------------------------------------------------------------------
// CollisionInfo: defines a rectangular hit-box and an optional kick offset.
//------------------------------------------------------------------------------
struct CollisionInfo {
    int offsetLeft;     ///< horizontal from sprite origin → left edge of box
    int offsetRight;    ///< horizontal from sprite origin → right edge of box
    int offsetTop;      ///< vertical   from sprite origin → top    of box
    int boxWidth;       ///< width   of the collision rectangle
    int boxHeight;      ///< height  of the collision rectangle
    int kickAdjustment; ///< extra horizontal shift during kicking animation
};

#endif // OTHER_HPP
