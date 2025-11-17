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
        /* fallback to Download */
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

void xeno_log(const char *fmt, ...) {
    pthread_mutex_lock(&_log_mutex);
    if (!g_main) xeno_open_log();
    if (!g_main) { pthread_mutex_unlock(&_log_mutex); return; }
    va_list ap;
    va_start(ap, fmt);
    char buf[2048];
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    time_t t = time(NULL);
    fprintf(g_main, "[%ld] %s\n", (long)t, buf);
    safe_fsync(g_main);
    pthread_mutex_unlock(&_log_mutex);
}

void xeno_log_event(const char *json) {
    pthread_mutex_lock(&_log_mutex);
    mkdirs_strict(PRIMARY_BASE);
    char ev[1024];
    snprintf(ev, sizeof(ev), "%s/events.json", PRI_EVENTS);
    FILE *f = fopen(ev, "a");
    if (f) {
        time_t t = time(NULL);
        fprintf(f, "{\"ts\":%ld,%s}\n", (long)t, json[0] ? json+1 : json); // ensure valid JSON line
        fflush(f);
        int fd = fileno(f);
        if (fd >= 0) fsync(fd);
        fclose(f);
    }
    /* mirror critical summary to main */
    xeno_log("[EVENT] %.800s", json);
    pthread_mutex_unlock(&_log_mutex);
}

void xeno_log_bin(const char *relpath, const void *data, size_t size) {
    pthread_mutex_lock(&_log_mutex);
    mkdirs_strict(PRIMARY_BASE);
    char out[1024];
    snprintf(out, sizeof(out), "%s/%s", PRIMARY_BASE, relpath);
    /* create directories if needed */
    char cmd[1200];
    snprintf(cmd, sizeof(cmd), "mkdir -p $(dirname '%s')", out);
    system(cmd);
    FILE *f = fopen(out, "wb");
    if (f) {
        fwrite(data, 1, size, f);
        fflush(f);
        int fd = fileno(f);
        if (fd >= 0) fsync(fd);
        fclose(f);
        xeno_log("WROTE_BIN %s size=%zu", out, size);
    } else {
        xeno_log("FAILED_WRITE_BIN %s", out);
    }
    pthread_mutex_unlock(&_log_mutex);
}

void xeno_flush_all(void) {
    pthread_mutex_lock(&_log_mutex);
    if (g_main) {
        fflush(g_main);
        safe_fsync(g_main);
    }
    pthread_mutex_unlock(&_log_mutex);
}

void xeno_init(void) {
    xeno_open_log();
    xeno_log("xeno_init (extreme) initialized");
}
