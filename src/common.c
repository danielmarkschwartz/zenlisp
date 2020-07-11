#include <assert.h>
#include <stdlib.h>

#include "common.h"

// Wrapper around malloc that cannot fail
void *zl_malloc(size_t n) {
    void *ret = malloc(n);
    assert(ret);
    return ret;
}
