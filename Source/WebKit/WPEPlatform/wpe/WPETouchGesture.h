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

#ifndef WPETouchGesture_h
#define WPETouchGesture_h

#if !defined(__WPE_PLATFORM_H_INSIDE__) && !defined(BUILDING_WEBKIT)
#error "Only <wpe/wpe-platform.h> can be included directly."
#endif

#include <glib-object.h>
#include <wpe/WPEDefines.h>

G_BEGIN_DECLS

#define WPE_TYPE_TOUCH_GESTURE (wpe_touch_gesture_get_type())

typedef struct _WPETouchGesture WPETouchGesture;

/**
 * WPETouchGestureType:
 * @WPE_TOUCH_GESTURE_NONE:
 * @WPE_TOUCH_GESTURE_TAP:
 * @WPE_TOUCH_GESTURE_DRAG:
 *
 * The type of a #WPETouchGesture
 */
typedef enum {
    WPE_TOUCH_GESTURE_NONE,

    WPE_TOUCH_GESTURE_TAP,
    WPE_TOUCH_GESTURE_DRAG,
} WPETouchGestureType;

WPE_API GType               wpe_touch_gesture_get_type               (void);
WPE_API WPETouchGesture    *wpe_touch_gesture_ref                    (WPETouchGesture      *gesture);
WPE_API void                wpe_touch_gesture_unref                  (WPETouchGesture      *gesture);
WPE_API WPETouchGestureType wpe_touch_gesture_get_touch_gesture_type (WPETouchGesture      *gesture);
WPE_API WPETouchGesture    *wpe_touch_gesture_tap_new                (double                x,
                                                                      double                y);
WPE_API WPETouchGesture    *wpe_touch_gesture_drag_new               (double                x,
                                                                      double                y,
                                                                      double                delta_x,
                                                                      double                delta_y);
WPE_API gboolean            wpe_touch_gesture_get_position           (WPETouchGesture      *gesture,
                                                                      double               *x,
                                                                      double               *y);
WPE_API gboolean            wpe_touch_gesture_get_delta              (WPETouchGesture      *gesture,
                                                                      double               *x,
                                                                      double               *y);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(WPETouchGesture, wpe_touch_gesture_unref)

G_END_DECLS

#endif /* WPETouchGesture_h */
