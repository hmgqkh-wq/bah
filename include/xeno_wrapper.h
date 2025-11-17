#ifndef XENO_WRAPPER_H
#define XENO_WRAPPER_H

#include <stdint.h>

void xeno_init(void);
void xeno_open_log(void);
void xeno_log(const char *fmt, ...);
void xeno_log_event(const char *json);
void xeno_log_bin(const char *relpath, const void *data, size_t size);
void xeno_flush_all(void);

#endif
