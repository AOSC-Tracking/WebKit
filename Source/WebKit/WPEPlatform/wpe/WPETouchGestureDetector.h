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

#ifndef WPETouchGestureDetector_h
#define WPETouchGestureDetector_h

#if !defined(__WPE_PLATFORM_H_INSIDE__) && !defined(BUILDING_WEBKIT)
#error "Only <wpe/wpe-platform.h> can be included directly."
#endif

#include <glib-object.h>
#include <wpe/WPEDefines.h>
#include <wpe/WPEEvent.h>
#include <wpe/WPETouchGesture.h>

G_BEGIN_DECLS

#define WPE_TYPE_TOUCH_GESTURE_DETECTOR (wpe_touch_gesture_detector_get_type())
WPE_DECLARE_DERIVABLE_TYPE (WPETouchGestureDetector, wpe_touch_gesture_detector, WPE, TOUCH_GESTURE_DETECTOR, GObject)

struct _WPETouchGestureDetectorClass
{
    GObjectClass parent_class;

    void             (* process_event)        (WPETouchGestureDetector *detector,
                                               WPEEvent                *event);
    WPETouchGesture *(* get_detected_gesture) (WPETouchGestureDetector *detector);

    gpointer padding[32];
};

WPE_API void             wpe_touch_gesture_detector_process_event        (WPETouchGestureDetector *detector,
                                                                          WPEEvent                *event);
WPE_API WPETouchGesture *wpe_touch_gesture_detector_get_detected_gesture (WPETouchGestureDetector *detector);

G_END_DECLS

#endif /* WPETouchGestureDetector_h */
