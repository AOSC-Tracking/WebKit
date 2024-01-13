/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "ScriptWrappable.h"
#include <wtf/RefCounted.h>

#include "AbortController.h"
#include "JSSubscriptionObserverCallback.h"
#include "ScriptExecutionContext.h"
#include "VoidCallback.h"

namespace WebCore {

class Subscriber final : public ScriptWrappable, public RefCounted<Subscriber> {
    WTF_MAKE_ISO_ALLOCATED(Subscriber);

public:
    void next(JSC::JSValue);
    void complete();
    void error(JSC::JSValue);
    void addTeardown(Ref<VoidCallback>);

    bool active() { return m_active; }
    AbortSignal& signal() { return m_abortController->signal(); }

    static Ref<Subscriber> create(ScriptExecutionContext&);

    static Ref<Subscriber> create(ScriptExecutionContext&, RefPtr<JSSubscriptionObserverCallback> next);

    static Ref<Subscriber> create(ScriptExecutionContext&, RefPtr<JSSubscriptionObserverCallback> next,
        RefPtr<JSSubscriptionObserverCallback> error,
        RefPtr<VoidCallback>);

    explicit Subscriber(ScriptExecutionContext&, RefPtr<JSSubscriptionObserverCallback> next,
        RefPtr<JSSubscriptionObserverCallback> error,
        RefPtr<VoidCallback>);

private:
    bool m_active = true;
    Ref<AbortController> m_abortController;
    RefPtr<ScriptExecutionContext> m_context;
    RefPtr<JSSubscriptionObserverCallback> m_next;
    RefPtr<JSSubscriptionObserverCallback> m_error;
    RefPtr<VoidCallback> m_complete;
    ListHashSet<Ref<VoidCallback>> m_teardowns;

    void close();
    void abort();
    void closeAndTeardown();

    Document* document() const { return downcast<Document>(m_context.get()); }
};

} // namespace WebCore
