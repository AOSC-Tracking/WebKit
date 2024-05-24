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
#include "WPETouchGestureDetector.h"

#include <optional>
#include <wtf/Deque.h>
#include <wtf/glib/WTFGType.h>

namespace WPE {

class TouchGestureDetector final {
public:
    ~TouchGestureDetector()
    {
        while (auto* gesture = getDetectedGesture())
            wpe_touch_gesture_unref(gesture);
    }

    void processEvent(WPEEvent* event)
    {
        switch (wpe_event_get_event_type(event)) {
        case WPE_EVENT_TOUCH_DOWN:
            if (double x, y; wpe_event_get_position(event, &x, &y))
                m_touchDownPoint = Point { x, y };
            break;
        case WPE_EVENT_TOUCH_CANCEL:
            reset();
            break;
        case WPE_EVENT_TOUCH_MOVE:
            if (!m_touchDownPoint)
                break;
            if (double x, y; wpe_event_get_position(event, &x, &y)) {
                if (!m_dragReferencePoint && std::hypot(x - m_touchDownPoint->x, y - m_touchDownPoint->y) > dragActivationThresholdPx)
                    m_dragReferencePoint = m_touchDownPoint;
                if (m_dragReferencePoint) {
                    Point delta { x - m_dragReferencePoint->x, y - m_dragReferencePoint->y };
                    m_detectedGestures.append(wpe_touch_gesture_drag_new(
                        m_touchDownPoint->x,
                        m_touchDownPoint->y,
                        delta.x,
                        delta.y
                    ));
                    m_dragReferencePoint = Point { x, y };
                }
            }
            break;
        case WPE_EVENT_TOUCH_UP:
            if (double x, y; wpe_event_get_position(event, &x, &y) && m_touchDownPoint) {
                if (m_dragReferencePoint) {
                    Point delta { x - m_dragReferencePoint->x, y - m_dragReferencePoint->y };
                    m_detectedGestures.append(wpe_touch_gesture_drag_new(
                        m_touchDownPoint->x,
                        m_touchDownPoint->y,
                        delta.x,
                        delta.y
                    ));
                } else if (m_touchDownPoint)
                    m_detectedGestures.append(wpe_touch_gesture_tap_new(m_touchDownPoint->x, m_touchDownPoint->y));
            }
            reset();
            break;
        default:
            break;
        }
    }

    WPETouchGesture* getDetectedGesture()
    {
        if (!m_detectedGestures.isEmpty())
            return m_detectedGestures.takeFirst();
        return nullptr;
    }

private:
    void reset()
    {
        m_touchDownPoint = std::nullopt;
        m_dragReferencePoint = std::nullopt;
    }

    struct Point {
        double x;
        double y;
    };

    // FIXME: These ought to be either configurable or derived from system
    //        properties, such as screen size and pixel density.
    static constexpr uint32_t dragActivationThresholdPx { 8 };

    std::optional<Point> m_touchDownPoint;
    std::optional<Point> m_dragReferencePoint;
    WTF::Deque<WPETouchGesture*> m_detectedGestures;
};

} // namespace WPE


/**
 * WPETouchGestureDetector:
 *
 */
struct _WPETouchGestureDetectorPrivate {
    WPE::TouchGestureDetector detector;
};

WEBKIT_DEFINE_TYPE(WPETouchGestureDetector, wpe_touch_gesture_detector, G_TYPE_OBJECT)

static void wpeProcessEvent(WPETouchGestureDetector* detector, WPEEvent* event)
{
    detector->priv->detector.processEvent(event);
}

static WPETouchGesture* wpeGetDetectedGesture(WPETouchGestureDetector* detector)
{
    return detector->priv->detector.getDetectedGesture();
}

static void wpe_touch_gesture_detector_class_init(WPETouchGestureDetectorClass* touchGestureDetectorClass)
{
    touchGestureDetectorClass->process_event = wpeProcessEvent;
    touchGestureDetectorClass->get_detected_gesture = wpeGetDetectedGesture;
}

/**
 * wpe_touch_gesture_detector_process_event:
 * @detector: a #WPETouchGestureDetector
 * @event: a #WPEEvent
 *
 * Process @event in @detector in search of gestures
 */
void wpe_touch_gesture_detector_process_event(WPETouchGestureDetector *detector, WPEEvent* event)
{
    g_return_if_fail(detector && event);

    auto* detectorClass = WPE_TOUCH_GESTURE_DETECTOR_GET_CLASS(detector);
    if (detectorClass->process_event)
        detectorClass->process_event(detector, event);
}

/**
 * wpe_touch_gesture_detector_get_detected_gesture:
 * @detector: a #WPETouchGestureDetector
 *
 * Get the gesture detected by @detector if any was detected during processing
 *
 * Returns: (transfer full): a new allocated #WPETouchGesture
 */
WPETouchGesture* wpe_touch_gesture_detector_get_detected_gesture(WPETouchGestureDetector *detector)
{
    g_return_val_if_fail(detector, nullptr);

    auto* detectorClass = WPE_TOUCH_GESTURE_DETECTOR_GET_CLASS(detector);
    if (detectorClass->get_detected_gesture)
        return detectorClass->get_detected_gesture(detector);
    return nullptr;
}
