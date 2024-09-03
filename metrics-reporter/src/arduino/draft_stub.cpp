#include <draft.h>
#include <stdio.h>

#include <stub_framework.h>
#include <draft_stub.h>

#define STUB_IMPL (draftStubs_p)

static DraftStubs draftStubs {
    .foo = foo_default,
    .bar = bar_default,
    .prev = nullptr
};

DraftStubs *draftStubs_p = &draftStubs;

void foo() {
    STUB_CALL(foo);
}

void foo_default() {
    printf("Called default %s function\n", __func__);
}

bool bar(int var) {
    STUB_CALL(bar, var);
}

bool bar_default(int var) {
    printf("Called default %s function, arg: %d\n", __func__, var);
    return true;
}