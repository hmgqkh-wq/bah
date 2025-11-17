#define _POSIX_C_SOURCE 200809L
#include "xeno_wrapper.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>    // <-- FIXED: required for system()

static pthread_mutex_t _log_mutex = PTHREAD_MUTEX_INITIALIZER;
static FILE *g_main = NULL;

/* paths */
static const char *PRIMARY_BASE = "/sdcard/eden_wrapper_logs";
static const char *PRI_API = "/sdcard/eden_wrapper_logs/api";
static const char *PRI_DEV = "/sdcard/eden_wrapper_logs/device";
static const char *PRI_SHADER = "/sdcard/eden_wrapper_logs/shader";
static const char *PRI_EVENTS = "/sdcard/eden_wrapper_logs/events";

static void mkdirs_strict(const char *path) {
    mkdir("/sdcard", 0755);
    mkdir("/sdcard/eden_wrapper_logs", 0755);
    mkdir(PRI_API, 0755);
    mkdir(PRI_DEV, 0755);
    mkdir(PRI_SHADER, 0755);
    mkdir(PRI_EVENTS, 0755);
}

/* safe fsync helper */
static void safe_fsync(FILE *f) {
    if (!f) return;
    fflush(f);
    int fd = fileno(f);
    if (fd >= 0) fsync(fd);
}

void xeno_open_log(void) {
    pthread_mutex_lock(&_log_mutex);
    if (g_main) { pthread_mutex_unlock(&_log_mutex); return; }
    mkdirs_strict(PRIMARY_BASE);
    char path[512];
    snprintf(path, sizeof(path), "%s/vulkan_log.txt", PRIMARY_BASE);
    g_main = fopen(path, "a");
    if (g_main) {
        time_t t = time(NULL);
        fprintf(g_main, "=== xeno log start: %s", ctime(&t));
        safe_fsync(g_main);
    } else {
        mkdir("/sdcard/Download", 0755);
        FILE *fb = fopen("/sdcard/Download/eden_wrapper_fallback_log.txt", "a");
        if (fb) {
            time_t t = time(NULL);
            fprintf(fb, "=== fallback xeno log start: %s", ctime(&t));
            safe_fsync(fb);
            g_main = fb;
        }
    }
    pthread_mutex_unlock(&_log_mutex);
}
