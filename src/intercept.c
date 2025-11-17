#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "vulkan/vulkan.h"
#include "xeno_wrapper.h"

/* We will resolve many functions dynamically and intercept them to log EVERYTHING. */

/* function pointer types we might need */
typedef VkResult (*PFN_vkEnumeratePhysicalDevices)(VkInstance, uint32_t*, VkPhysicalDevice*);
typedef VkResult (*PFN_vkEnumerateInstanceExtensionProperties)(const char*, uint32_t*, VkExtensionProperties*);
typedef VkResult (*PFN_vkEnumerateDeviceExtensionProperties)(VkPhysicalDevice, const char*, uint32_t*, VkExtensionProperties*);
typedef void (*PFN_vkGetPhysicalDeviceProperties)(VkPhysicalDevice, VkPhysicalDeviceProperties*);
typedef void (*PFN_vkGetPhysicalDeviceQueueFamilyProperties)(VkPhysicalDevice, uint32_t*, VkQueueFamilyProperties*);
typedef VkResult (*PFN_vkCreateShaderModule)(VkDevice, const void*, const void*, void*);

static void *real_lib = NULL;
static PFN_vkGetInstanceProcAddr real_vkGetInstanceProcAddr = NULL;

/* cached core pointers */
static PFN_vkEnumeratePhysicalDevices real_vkEnumeratePhysicalDevices = NULL;
static PFN_vkEnumerateInstanceExtensionProperties real_vkEnumerateInstanceExtensionProperties = NULL;
static PFN_vkEnumerateDeviceExtensionProperties real_vkEnumerateDeviceExtensionProperties = NULL;
static PFN_vkGetPhysicalDeviceProperties real_vkGetPhysicalDeviceProperties = NULL;
static PFN_vkGetPhysicalDeviceQueueFamilyProperties real_vkGetPhysicalDeviceQueueFamilyProperties = NULL;
static PFN_vkCreateShaderModule real_vkCreateShaderModule = NULL;

static void load_real_symbols(void) {
    if (real_lib) return;
    real_lib = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (!real_lib) {
        xeno_log("intercept: dlopen(libvulkan.so) failed");
        return;
    }
    real_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(real_lib, "vkGetInstanceProcAddr");
    real_vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)dlsym(real_lib, "vkEnumeratePhysicalDevices");
    real_vkEnumerateInstanceExtensionProperties = (PFN_vkEnumerateInstanceExtensionProperties)dlsym(real_lib, "vkEnumerateInstanceExtensionProperties");
    real_vkEnumerateDeviceExtensionProperties = (PFN_vkEnumerateDeviceExtensionProperties)dlsym(real_lib, "vkEnumerateDeviceExtensionProperties");
    real_vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)dlsym(real_lib, "vkGetPhysicalDeviceProperties");
    real_vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)dlsym(real_lib, "vkGetPhysicalDeviceQueueFamilyProperties");
    real_vkCreateShaderModule = (PFN_vkCreateShaderModule)dlsym(real_lib, "vkCreateShaderModule");
    xeno_log("intercept: resolved real symbols");
}

/* Extremely verbose logging of enumerate instance extensions */
VkResult vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties) {
    load_real_symbols();
    if (!real_vkEnumerateInstanceExtensionProperties) return (VkResult)-1;
    VkResult r = real_vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
    if (pPropertyCount && pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            char j[512];
            snprintf(j, sizeof(j), "{\"event\":\"instance_extension\",\"name\":\"%s\",\"specVersion\":%u}", pProperties[i].extensionName, pProperties[i].specVersion);
            xeno_log_event(j);
            xeno_log("INST_EXT %s v%u", pProperties[i].extensionName, pProperties[i].specVersion);
        }
    } else if (pPropertyCount) {
        char j[256];
        snprintf(j, sizeof(j), "{\"event\":\"instance_extension_count\",\"count\":%u}", *pPropertyCount);
        xeno_log_event(j);
    }
    return r;
}

/* Device extension enumeration logging */
VkResult vkEnumerateDeviceExtensionProperties(VkPhysicalDevice phys, const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties) {
    load_real_symbols();
    if (!real_vkEnumerateDeviceExtensionProperties) return (VkResult)-1;
    VkResult r = real_vkEnumerateDeviceExtensionProperties(phys, pLayerName, pPropertyCount, pProperties);
    if (pPropertyCount && pProperties) {
        for (uint32_t i = 0; i < *pPropertyCount; ++i) {
            char j[512];
            snprintf(j, sizeof(j), "{\"event\":\"device_extension\",\"name\":\"%s\",\"specVersion\":%u}", pProperties[i].extensionName, pProperties[i].specVersion);
            xeno_log_event(j);
            xeno_log("DEV_EXT %s v%u", pProperties[i].extensionName, pProperties[i].specVersion);
        }
    } else if (pPropertyCount) {
        char j[256];
        snprintf(j, sizeof(j), "{\"event\":\"device_extension_count\",\"count\":%u}", *pPropertyCount);
        xeno_log_event(j);
    }
    return r;
}

/* Physical device properties logging */
void vkGetPhysicalDeviceProperties(VkPhysicalDevice phys, VkPhysicalDeviceProperties *pProps) {
    load_real_symbols();
    if (!real_vkGetPhysicalDeviceProperties) return;
    real_vkGetPhysicalDeviceProperties(phys, pProps);

    char j[1024];
    snprintf(j, sizeof(j), "{\"event\":\"physical_device_properties\",\"vendorID\":%u,\"deviceID\":%u,\"apiVersion\":%u,\"deviceName\":\"%s\"}",
        pProps->vendorID, pProps->deviceID, pProps->apiVersion, pProps->deviceName);
    xeno_log_event(j);
    xeno_log("PHY PROPS vendor=%u device=%u api=%u name=%s", pProps->vendorID, pProps->deviceID, pProps->apiVersion, pProps->deviceName);

    /* write a compact JSON feature dump for Phase-2 */
    generate_feature_dump_to(NULL, phys, pProps);
}

/* Queue family properties */
void vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice phys, uint32_t *pCount, VkQueueFamilyProperties *pProps) {
    load_real_symbols();
    if (!real_vkGetPhysicalDeviceQueueFamilyProperties) return;
    real_vkGetPhysicalDeviceQueueFamilyProperties(phys, pCount, pProps);
    if (pCount && pProps) {
        for (uint32_t i = 0; i < *pCount; ++i) {
            char j[512];
            snprintf(j, sizeof(j), "{\"event\":\"queue_family\",\"index\":%u,\"flags\":%u,\"count\":%u,\"timestampBits\":%u}",
                    i, pProps[i].queueFlags, pProps[i].queueCount, pProps[i].timestampValidBits);
            xeno_log_event(j);
            xeno_log("QF idx=%u flags=%u count=%u", i, pProps[i].queueFlags, pProps[i].queueCount);
        }
    }
}

/* CreateShaderModule — best-effort SPIR-V capture and write to file */
VkResult vkCreateShaderModule(VkDevice device, const void *pCreateInfo, const void *pAllocator, void *pShaderModule) {
    load_real_symbols();
    if (!real_vkCreateShaderModule) return (VkResult)-1;

    /* Attempt to read codeSize & pCode using expected offsets - guarded */
    uint64_t codeSize = 0;
    const uint32_t *pCode = NULL;
    /* Try several offset layouts to increase capture rate */
    for (int attempt = 0; attempt < 3; ++attempt) {
        if (attempt == 0) {
            memcpy(&codeSize, (const char*)pCreateInfo + 16, sizeof(uint64_t));
            pCode = *(const uint32_t**)((const char*)pCreateInfo + 24);
        } else if (attempt == 1) {
            memcpy(&codeSize, (const char*)pCreateInfo + 24, sizeof(uint64_t));
            pCode = *(const uint32_t**)((const char*)pCreateInfo + 32);
        } else {
            memcpy(&codeSize, (const char*)pCreateInfo + 8, sizeof(uint64_t));
            pCode = *(const uint32_t**)((const char*)pCreateInfo + 16);
        }
        if (codeSize > 3 && pCode) break;
    }

    if (codeSize > 3 && pCode) {
        /* compute a simple checksum */
        uint32_t xorv = 0;
        size_t words = codeSize / 4;
        for (size_t i = 0; i < words; ++i) xorv ^= pCode[i];
        char j[512];
        snprintf(j, sizeof(j), "{\"event\":\"shader_module\",\"size\":%llu,\"xor\":%u}", (unsigned long long)codeSize, xorv);
        xeno_log_event(j);
        /* write binary */
        char rel[256];
        time_t t = time(NULL);
        pid_t pid = getpid();
        snprintf(rel, sizeof(rel), "shader/spirv_pid%u_%ld.bin", (unsigned)pid, (long)t);
        xeno_log_bin(rel, pCode, (size_t)codeSize);
    } else {
        xeno_log("shader capture failed or codeSize==0");
    }

    return real_vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

/* vkGetInstanceProcAddr: log lookups and return our intercepts if requested */
PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char *pName) {
    load_real_symbols();
    if (!real_vkGetInstanceProcAddr) return NULL;

    if (pName) {
        char tmp[512];
        snprintf(tmp, sizeof(tmp), "{\"event\":\"proc_lookup\",\"name\":\"%s\"}", pName);
        xeno_log_event(tmp);
    }

    /* we provide our intercepted functions */
    if (pName) {
        if (strcmp(pName, "vkEnumerateInstanceExtensionProperties") == 0) return (PFN_vkVoidFunction)vkEnumerateInstanceExtensionProperties;
        if (strcmp(pName, "vkEnumerateDeviceExtensionProperties") == 0) return (PFN_vkVoidFunction)vkEnumerateDeviceExtensionProperties;
        if (strcmp(pName, "vkGetPhysicalDeviceProperties") == 0) return (PFN_vkVoidFunction)vkGetPhysicalDeviceProperties;
        if (strcmp(pName, "vkGetPhysicalDeviceQueueFamilyProperties") == 0) return (PFN_vkVoidFunction)vkGetPhysicalDeviceQueueFamilyProperties;
        if (strcmp(pName, "vkCreateShaderModule") == 0) return (PFN_vkVoidFunction)vkCreateShaderModule;
    }

    return real_vkGetInstanceProcAddr(instance, pName);
}

/* Ensure logger inits early */
__attribute__((constructor))
static void xeno_init_ctor(void) {
    xeno_init();
}
