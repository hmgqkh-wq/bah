#ifndef XENO_WRAPPER_H
#define XENO_WRAPPER_H

#include <stdint.h>
#include <stddef.h>   // <-- FIXED: required for size_t

void xeno_init(void);
void xeno_open_log(void);
void xeno_log(const char *fmt, ...);
void xeno_log_event(const char *json);
void xeno_log_bin(const char *relpath, const void *data, size_t size);
void xeno_flush_all(void);

/* FIXED: add missing prototype so intercept.c can use the function */
struct VkPhysicalDeviceProperties;  // forward type
int generate_feature_dump_to(const char *path,
                             void *phys,
                             struct VkPhysicalDeviceProperties *props);

#endif
