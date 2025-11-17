#ifndef VULKAN_MINIMAL_H
#define VULKAN_MINIMAL_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* minimal subset used by collector */
#define VKAPI_ATTR
#define VKAPI_CALL

typedef void* VkInstance;
typedef void* VkPhysicalDevice;
typedef void* VkDevice;
typedef void* VkQueue;
typedef uint32_t VkResult;
typedef uint32_t VkFormat;

typedef void (*PFN_vkVoidFunction)(void);
typedef PFN_vkVoidFunction (*PFN_vkGetInstanceProcAddr)(VkInstance, const char*);

/* structures (minimal fields we use/log) */
typedef struct VkExtensionProperties {
    char extensionName[256];
    uint32_t specVersion;
} VkExtensionProperties;

typedef struct VkPhysicalDeviceProperties {
    uint32_t apiVersion;
    uint32_t driverVersion;
    uint32_t vendorID;
    uint32_t deviceID;
    uint32_t deviceType;
    char deviceName[256];
    uint8_t pipelineCacheUUID[16];
} VkPhysicalDeviceProperties;

typedef struct VkQueueFamilyProperties {
    uint32_t queueFlags;
    uint32_t queueCount;
    uint32_t timestampValidBits;
    uint32_t minImageTransferGranularity[3];
} VkQueueFamilyProperties;

typedef struct VkFormatProperties {
    uint32_t linearTilingFeatures;
    uint32_t optimalTilingFeatures;
    uint32_t bufferFeatures;
} VkFormatProperties;

#ifdef __cplusplus
}
#endif
#endif
