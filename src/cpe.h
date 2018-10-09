#ifndef __CPE_H__
#define __CPE_H__

typedef struct cpeext_s {
    const char *name;
    int version;
} cpeext_t;

cpeext_t *cpe_get_supported_exts(int *size);

#endif // __CPE_H__