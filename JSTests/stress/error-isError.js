//@ requireOptions("--useErrorIsError=1")

function shouldBe(actual, expected) {
    if (actual !== expected)
        throw new Error(`expected ${expected} but got ${actual}`);
}

function notReached() {
    throw new Error("should not reach here");
}

function revokedProxy(target) {
    const {proxy, revoke} = Proxy.revocable(target, {});
    revoke();
    return proxy;
}

shouldBe(Error.isError(undefined), false);
shouldBe(Error.isError(null), false);
shouldBe(Error.isError(true), false);
shouldBe(Error.isError(42), false);
shouldBe(Error.isError("test"), false);

shouldBe(Error.isError([]), false);
shouldBe(Error.isError(new Proxy([], {})), false);

shouldBe(Error.isError({}), false);
shouldBe(Error.isError(new Proxy({}, {})), false);

const errorConstructors = [Error, EvalError, RangeError, ReferenceError, SyntaxError, TypeError, URIError];
if (typeof WebAssembly !== 'undefined')
    errorConstructors.push(WebAssembly.CompileError, WebAssembly.LinkError, WebAssembly.RuntimeError);
for (const E of errorConstructors) {
    shouldBe(Error.isError(E), false);
    shouldBe(Error.isError(new Proxy(E, {})), false);

    shouldBe(Error.isError(E.prototype), false);
    shouldBe(Error.isError(new Proxy(E.prototype, {})), false);

    shouldBe(Error.isError(new E), true);
    shouldBe(Error.isError(new Proxy(new E, {})), true);
    shouldBe(Error.isError(new Proxy(new Proxy(new E, {}), {})), true);

    try {
        Error.isError(revokedProxy(new E));
        notReached();
    } catch (e) {
        shouldBe(e instanceof TypeError, true);
    }
    try {
        Error.isError(new Proxy(revokedProxy(new E), {}));
        notReached();
    } catch (e) {
        shouldBe(e instanceof TypeError, true);
    }
    try {
        Error.isError(revokedProxy(new Proxy(new E, {})));
        notReached();
    } catch (e) {
        shouldBe(e instanceof TypeError, true);
    }
}

shouldBe(Error.isError(AggregateError), false);
shouldBe(Error.isError(new Proxy(AggregateError, {})), false);

shouldBe(Error.isError(AggregateError.prototype), false);
shouldBe(Error.isError(new Proxy(AggregateError.prototype, {})), false);

shouldBe(Error.isError(new AggregateError([])), true);
shouldBe(Error.isError(new Proxy(new AggregateError([]), {})), true);
shouldBe(Error.isError(new Proxy(new Proxy(new AggregateError([]), {}), {})), true);

try {
    Error.isError(revokedProxy(new AggregateError([])));
    notReached();
} catch (e) {
    shouldBe(e instanceof TypeError, true);
}
try {
    Error.isError(new Proxy(revokedProxy(new AggregateError([])), {}));
    notReached();
} catch (e) {
    shouldBe(e instanceof TypeError, true);
}
try {
    Error.isError(revokedProxy(new Proxy(new AggregateError([]), {})));
    notReached();
} catch (e) {
    shouldBe(e instanceof TypeError, true);
}
