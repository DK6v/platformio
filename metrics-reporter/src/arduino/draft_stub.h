#pragma once

struct DraftStubs {
    void (*foo)();
    bool (*bar)(int);
    DraftStubs *prev;
};

extern DraftStubs *fooStubs_p;

void foo_default();
bool bar_default(int var);
