#include <stddef.h>
#include "cpe.h"

cpeext_t extensions[] = {
    
};

cpeext_t *cpe_get_supported_exts(int *size) {
    if (size != NULL) {
        *size = (int)sizeof(extensions);
    }

    return extensions;
}