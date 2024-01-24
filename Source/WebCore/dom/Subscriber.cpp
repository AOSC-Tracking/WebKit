/*
 * Copyright (C) 2024 Keith Cirkel <webkit@keithcirkel.co.uk>. All rights reserved.
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
#include "config.h"

#include "AbortController.h"
#include "AbortSignal.h"
#include "JSSubscriptionObserverCallback.h"
#include "Subscriber.h"
#include "SubscriberCallback.h"
#include "VoidCallback.h"

namespace WebCore {

Ref<Subscriber> Subscriber::create(ScriptExecutionContext& context)
{
    dataLogF("new Subscriber(context, nullopt, nullopt, nullopt");
    return adoptRef(*new Subscriber(context, nullptr, nullptr, nullptr));
}

Ref<Subscriber> Subscriber::create(ScriptExecutionContext& context, RefPtr<JSSubscriptionObserverCallback> nextCallback)
{
    dataLogF("new Subscriber(context, nextCallback, nullopt, nullopt");
    return adoptRef(*new Subscriber(context, nextCallback, nullptr, nullptr));
}

Ref<Subscriber> Subscriber::create(ScriptExecutionContext& context, RefPtr<JSSubscriptionObserverCallback> nextCallback, RefPtr<JSSubscriptionObserverCallback> errorCallback, RefPtr<VoidCallback> completeCallback)
{
    dataLogF("new Subscriber(context, nextCallback %d, errorCallback %d, completeCallback %d", !!nextCallback, !!errorCallback, !!completeCallback);
    return adoptRef(*new Subscriber(context, nextCallback, errorCallback, completeCallback));
}

Subscriber::Subscriber(ScriptExecutionContext& context, RefPtr<JSSubscriptionObserverCallback> nextCallback, RefPtr<JSSubscriptionObserverCallback> errorCallback, RefPtr<VoidCallback> completeCallback)
    : m_abortController(AbortController::create(context))
    , m_context(&context)
    , m_next(WTFMove(nextCallback))
    , m_error(WTFMove(errorCallback))
    , m_complete(WTFMove(completeCallback))
{
    dataLogF("Subscriber(ctx, %d, %d, %d)", !!m_next, !!m_error, !!m_complete);

    if (m_abortController->signal().aborted())
        close();

    else
        m_abortController->signal().addAlgorithm([this](JSC::JSValue) {
            closeAndTeardown();
        });
}

void Subscriber::next(JSC::JSValue value)
{
    if (document() && !document()->isFullyActive())
        return;

    if (!m_active)
        return;

    dataLogF("Calling next %d", !!m_next);
    if (m_next)
        m_next->handleEvent(value);
}

void Subscriber::error(JSC::JSValue value)
{
    if (document() && !document()->isFullyActive())
        return;

    auto error = m_error;
    dataLogF("Calling error %d", !!m_error);

    close();

    if (error)
        error->handleEvent(value);

    else
        reportException(
            JSC::jsCast<JSDOMGlobalObject*>(m_context.get()->globalObject()),
            JSC::Exception::create(m_context.get()->vm(), value));

    abort();
}

void Subscriber::complete()
{
    if (document() && !document()->isFullyActive())
        return;

    auto complete = m_complete;
    dataLogF("Calling complete %d", !!m_complete);

    close();

    if (complete) {
        dataLogF("Finally calling complete");
        complete->handleEvent();
    }

    abort();
}

void Subscriber::addTeardown(Ref<VoidCallback> callback)
{
    if (m_active)
        m_teardowns.add(callback);
    else
        callback.get().handleEvent();
}

void Subscriber::close()
{
    m_active = false;
    m_next = nullptr;
    m_error = nullptr;
    m_complete = nullptr;
}

void Subscriber::abort()
{
    m_abortController->abort(*JSC::jsCast<JSDOMGlobalObject*>(m_context.get()->globalObject()), JSC::jsUndefined());
}

void Subscriber::closeAndTeardown()
{
    close();
    for (auto iter = m_teardowns.rbegin(); iter != m_teardowns.rend(); ++iter) {
        (*iter).get().handleEvent();
    }
    m_teardowns.clear();
}

WTF_MAKE_ISO_ALLOCATED_IMPL(Subscriber);

} // namespace WebCore
