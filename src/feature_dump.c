#define _GNU_SOURCE
#include "xeno_wrapper.h"
#include "vulkan/vulkan.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Minimal JSON dump: device properties + time */
int generate_feature_dump_to(const char *path, VkPhysicalDevice phys, VkPhysicalDeviceProperties *props)
{
    const char *out = (path && path[0]) ? path : "/sdcard/eden_wrapper_logs/device/xeno_feature_dump.json";
    FILE *f = fopen(out, "w");
    if (!f) return -1;
    time_t t = time(NULL);
    fprintf(f, "{\n");
    fprintf(f, "\"ts\": %ld,\n", (long)t);
    fprintf(f, "\"vendorID\": %u,\n", props->vendorID);
    fprintf(f, "\"deviceID\": %u,\n", props->deviceID);
    fprintf(f, "\"apiVersion\": %u,\n", props->apiVersion);
    fprintf(f, "\"deviceName\": \"%s\"\n", props->deviceName);
    fprintf(f, "}\n");
    fclose(f);
    xeno_log("feature_dump wrote %s", out);
    return 0;
}
