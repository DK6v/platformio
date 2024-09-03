#pragma once

#undef STUB_IMPL

#define STUB_CALL(_func, ...) \
do { \
    auto *impl_p = STUB_IMPL; \
    do { \
        if (impl_p->_func != nullptr) { \
            return impl_p->_func(__VA_ARGS__); \
        } \
        impl_p = impl_p->prev; \
    } while (impl_p != nullptr); \
    return _func ## _default(__VA_ARGS__); \
} while(false)
