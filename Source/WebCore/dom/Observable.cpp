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
#include "config.h"

#include "CallbackResult.h"
#include "Exception.h"
#include "ExceptionCode.h"
#include "JSSubscriptionObserverCallback.h"
#include "Observable.h"
#include "Subscriber.h"
#include "SubscriberCallback.h"

namespace WebCore {

ExceptionOr<Ref<Observable>> Observable::create(Ref<SubscriberCallback> callback)
{
    return adoptRef(*new Observable(callback));
}

void Observable::subscribe(ScriptExecutionContext& context, std::optional<ObserverUnion> observer, SubscribeOptions options)
{
    Document* document = downcast<Document>(&context);
    if (document && !document->isFullyActive())
        return;

    Ref<Subscriber> subscriber = makeSubscriber(context, observer);

    if (options.signal)
        subscriber->signal().signalFollow(*options.signal);

    auto res = m_subscriber->handleEvent(subscriber);

    if (res.type() == CallbackResultType::ExceptionThrown)
        // FIXME: Provide the error thrown from handleEvent
        subscriber->error(JSC::jsUndefined());
}

Ref<Subscriber> Observable::makeSubscriber(ScriptExecutionContext& context, std::optional<ObserverUnion> observer)
{
    if (observer.has_value())
        return std::visit(WTF::makeVisitor([&](RefPtr<JSSubscriptionObserverCallback>& next) {
            dataLogF( "Saw subscription callback fn so passing to Subscriber");
            return Subscriber::create(context, next); }, [&](SubscriptionObserver& subscription) {
            dataLogF( "Saw subscription observer object so passing to Subscriber");
            return Subscriber::create(context, subscription.next, subscription.error, subscription.complete); }), observer.value());

    return Subscriber::create(context);
}

WTF_MAKE_ISO_ALLOCATED_IMPL(Observable);

Observable::Observable(Ref<SubscriberCallback> callback)
    : m_subscriber(callback)
{
}

} // namespace WebCore
