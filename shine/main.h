#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include "as3helper.h"

static inline void error(const char *s) {
    AS3_Throw(s);
}

static inline bool verbose() {
    return true;
}

#endif
