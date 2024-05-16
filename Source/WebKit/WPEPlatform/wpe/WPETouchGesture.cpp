/*
 * Copyright (C) 2024 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "WPETouchGesture.h"

#include <variant>
#include <wtf/FastMalloc.h>

struct WPETouchGestureTap {
    double x { 0 };
    double y { 0 };
};

struct WPETouchGestureDrag {
    double x { 0 };
    double y { 0 };
    double deltaX { 0 };
    double deltaY { 0 };
};

/**
 * WPETouchGesture: (ref-func wpe_touch_gesture_ref) (unref-func wpe_touch_gesture_unref)
 *
 */
struct _WPETouchGesture {
    WTF_MAKE_STRUCT_FAST_ALLOCATED;

    WPETouchGestureType type { WPE_TOUCH_GESTURE_NONE };
    std::variant<WPETouchGestureTap, WPETouchGestureDrag> variant;

    int referenceCount { 1 };
};

G_DEFINE_BOXED_TYPE(WPETouchGesture, wpe_touch_gesture, wpe_touch_gesture_ref, wpe_touch_gesture_unref)

/**
 * wpe_touch_gesture_ref:
 * @gesture: a #WPETouchGesture
 *
 * Atomically increments the reference count of @gesture by one.
 *
 * This function is MT-safe and may be called from any thread.
 *
 * Returns: The passed in #WPETouchGesture
 */
WPETouchGesture* wpe_touch_gesture_ref(WPETouchGesture* gesture)
{
    g_atomic_int_inc(&gesture->referenceCount);
    return gesture;
}

/**
 * wpe_touch_gesture_unref:
 * @gesture: a #WPETouchGesture
 *
 * Atomically decrements the reference count of @gesture by one.
 *
 * If the reference count drops to 0, all memory allocated by the #WPETouchGesture is
 * released. This function is MT-safe and may be called from any thread.
 */
void wpe_touch_gesture_unref(WPETouchGesture* gesture)
{
    if (g_atomic_int_dec_and_test(&gesture->referenceCount))
        delete gesture;
}

/**
 * wpe_touch_gesture_get_touch_gesture_type:
 * @gesture: a #WPETouchGesture
 *
 * Get the #WPETouchGestureType of @gesture
 *
 * Returns: a #WPETouchGestureType
 */
WPETouchGestureType wpe_touch_gesture_get_touch_gesture_type(WPETouchGesture* gesture)
{
    g_return_val_if_fail(gesture, WPE_TOUCH_GESTURE_NONE);

    return gesture->type;
}

/**
 * wpe_touch_gesture_tap_new:
 * @x: the x coordinate of the tap
 * @y: the y coordinate of the tap
 *
 * Create a #WPETouchGesture for a tap gesture.
 *
 * Returns: (transfer full): a new allocated #WPETouchGesture.
 */
WPETouchGesture* wpe_touch_gesture_tap_new(double x, double y)
{
    return new _WPETouchGesture { WPE_TOUCH_GESTURE_TAP, WPETouchGestureTap { x, y }, 1 };
}

/**
 * wpe_touch_gesture_drag_new:
 * @x: the x coordinate of the initial touch
 * @y: the y coordinate of the initial touch
 * @delta_x: the delta on x axis
 * @delta_y: the delta on y axis
 *
 * Create a #WPETouchGesture for a drag gesture.
 *
 * Returns: (transfer full): a new allocated #WPETouchGesture.
 */
WPETouchGesture* wpe_touch_gesture_drag_new(double x, double y, double delta_x, double delta_y)
{
    return new _WPETouchGesture { WPE_TOUCH_GESTURE_DRAG, WPETouchGestureDrag { x, y, delta_x, delta_y }, 1 };
}

/**
 * wpe_touch_gesture_get_position:
 * @gesture: a #WPETouchGesture
 * @x: (out): location to store x coordinate
 * @y: (out): location to store y coordinate
 *
 * Get the position of @gesture. If the #WPETouchGesture doesn't have
 * a position, %FALSE is returned.
 *
 * Returns: %TRUE if position is returned in @x and @y,
 *    or %FALSE if @gesture doesn't have a positon
 */
gboolean wpe_touch_gesture_get_position(WPETouchGesture* gesture, double* x, double* y)
{
    g_return_val_if_fail(gesture, FALSE);

    return WTF::switchOn(gesture->variant,
        [&x, &y](const WPETouchGestureTap& tap) -> gboolean {
            if (x)
                *x = tap.x;
            if (y)
                *y = tap.y;
            return TRUE;
        },
        [&x, &y](const WPETouchGestureDrag& drag) -> gboolean {
            if (x)
                *x = drag.x;
            if (y)
                *y = drag.y;
            return TRUE;
        }
    );
}

/**
 * wpe_touch_gesture_get_delta:
 * @gesture: a #WPETouchGesture
 * @x: (out): location to store delta on x axis
 * @y: (out): location to store delta on y axis
 *
 * Get the delta of @gesture such as "drag" gesture. If the #WPETouchGesture
 * doesn't have a delta, %FALSE is returned.
 *
 * Returns: %TRUE if delta is returned in @x and @y,
 *    or %FALSE if @gesture doesn't have a delta
 */
gboolean wpe_touch_gesture_get_delta(WPETouchGesture* gesture, double* x, double* y)
{
    g_return_val_if_fail(gesture, FALSE);

    return WTF::switchOn(gesture->variant,
        [](const WPETouchGestureTap&) -> gboolean {
            return FALSE;
        },
        [&x, &y](const WPETouchGestureDrag& drag) -> gboolean {
            if (x)
                *x = drag.deltaX;
            if (y)
                *y = drag.deltaY;
            return TRUE;
        }
    );
}
