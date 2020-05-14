/*
** Copyright (c) 2015-2018 The Khronos Group Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

/*
** This header is generated from the Khronos Vulkan XML API Registry.
**
*/

#include <vulkan/vulkan.h>

#ifdef VK_KHR_surface
static PFN_vkDestroySurfaceKHR pfn_vkDestroySurfaceKHR;
VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(
    VkInstance                                  instance,
    VkSurfaceKHR                                surface,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroySurfaceKHR(
        instance,
        surface,
        pAllocator
    );
}

static PFN_vkGetPhysicalDeviceSurfaceSupportKHR pfn_vkGetPhysicalDeviceSurfaceSupportKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    VkSurfaceKHR                                surface,
    VkBool32*                                   pSupported)
{
    return pfn_vkGetPhysicalDeviceSurfaceSupportKHR(
        physicalDevice,
        queueFamilyIndex,
        surface,
        pSupported
    );
}

static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR pfn_vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilitiesKHR*                   pSurfaceCapabilities)
{
    return pfn_vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        physicalDevice,
        surface,
        pSurfaceCapabilities
    );
}

static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR pfn_vkGetPhysicalDeviceSurfaceFormatsKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormatKHR*                         pSurfaceFormats)
{
    return pfn_vkGetPhysicalDeviceSurfaceFormatsKHR(
        physicalDevice,
        surface,
        pSurfaceFormatCount,
        pSurfaceFormats
    );
}

static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR pfn_vkGetPhysicalDeviceSurfacePresentModesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pPresentModeCount,
    VkPresentModeKHR*                           pPresentModes)
{
    return pfn_vkGetPhysicalDeviceSurfacePresentModesKHR(
        physicalDevice,
        surface,
        pPresentModeCount,
        pPresentModes
    );
}

#endif /* VK_KHR_surface */
#ifdef VK_KHR_swapchain
static PFN_vkCreateSwapchainKHR pfn_vkCreateSwapchainKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(
    VkDevice                                    device,
    const VkSwapchainCreateInfoKHR*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchain)
{
    return pfn_vkCreateSwapchainKHR(
        device,
        pCreateInfo,
        pAllocator,
        pSwapchain
    );
}

static PFN_vkDestroySwapchainKHR pfn_vkDestroySwapchainKHR;
VKAPI_ATTR void VKAPI_CALL vkDestroySwapchainKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroySwapchainKHR(
        device,
        swapchain,
        pAllocator
    );
}

static PFN_vkGetSwapchainImagesKHR pfn_vkGetSwapchainImagesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainImagesKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pSwapchainImageCount,
    VkImage*                                    pSwapchainImages)
{
    return pfn_vkGetSwapchainImagesKHR(
        device,
        swapchain,
        pSwapchainImageCount,
        pSwapchainImages
    );
}

static PFN_vkAcquireNextImageKHR pfn_vkAcquireNextImageKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout,
    VkSemaphore                                 semaphore,
    VkFence                                     fence,
    uint32_t*                                   pImageIndex)
{
    return pfn_vkAcquireNextImageKHR(
        device,
        swapchain,
        timeout,
        semaphore,
        fence,
        pImageIndex
    );
}

static PFN_vkQueuePresentKHR pfn_vkQueuePresentKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(
    VkQueue                                     queue,
    const VkPresentInfoKHR*                     pPresentInfo)
{
    return pfn_vkQueuePresentKHR(
        queue,
        pPresentInfo
    );
}

#endif /* VK_KHR_swapchain */
#ifdef VK_KHR_display
static PFN_vkGetPhysicalDeviceDisplayPropertiesKHR pfn_vkGetPhysicalDeviceDisplayPropertiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPropertiesKHR*                     pProperties)
{
    return pfn_vkGetPhysicalDeviceDisplayPropertiesKHR(
        physicalDevice,
        pPropertyCount,
        pProperties
    );
}

static PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR pfn_vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pPropertyCount,
    VkDisplayPlanePropertiesKHR*                pProperties)
{
    return pfn_vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
        physicalDevice,
        pPropertyCount,
        pProperties
    );
}

static PFN_vkGetDisplayPlaneSupportedDisplaysKHR pfn_vkGetDisplayPlaneSupportedDisplaysKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneSupportedDisplaysKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    planeIndex,
    uint32_t*                                   pDisplayCount,
    VkDisplayKHR*                               pDisplays)
{
    return pfn_vkGetDisplayPlaneSupportedDisplaysKHR(
        physicalDevice,
        planeIndex,
        pDisplayCount,
        pDisplays
    );
}

static PFN_vkGetDisplayModePropertiesKHR pfn_vkGetDisplayModePropertiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayModePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    uint32_t*                                   pPropertyCount,
    VkDisplayModePropertiesKHR*                 pProperties)
{
    return pfn_vkGetDisplayModePropertiesKHR(
        physicalDevice,
        display,
        pPropertyCount,
        pProperties
    );
}

static PFN_vkCreateDisplayModeKHR pfn_vkCreateDisplayModeKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayModeKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display,
    const VkDisplayModeCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDisplayModeKHR*                           pMode)
{
    return pfn_vkCreateDisplayModeKHR(
        physicalDevice,
        display,
        pCreateInfo,
        pAllocator,
        pMode
    );
}

static PFN_vkGetDisplayPlaneCapabilitiesKHR pfn_vkGetDisplayPlaneCapabilitiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneCapabilitiesKHR(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayModeKHR                            mode,
    uint32_t                                    planeIndex,
    VkDisplayPlaneCapabilitiesKHR*              pCapabilities)
{
    return pfn_vkGetDisplayPlaneCapabilitiesKHR(
        physicalDevice,
        mode,
        planeIndex,
        pCapabilities
    );
}

static PFN_vkCreateDisplayPlaneSurfaceKHR pfn_vkCreateDisplayPlaneSurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayPlaneSurfaceKHR(
    VkInstance                                  instance,
    const VkDisplaySurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateDisplayPlaneSurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

#endif /* VK_KHR_display */
#ifdef VK_KHR_display_swapchain
static PFN_vkCreateSharedSwapchainsKHR pfn_vkCreateSharedSwapchainsKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSharedSwapchainsKHR(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainCreateInfoKHR*             pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkSwapchainKHR*                             pSwapchains)
{
    return pfn_vkCreateSharedSwapchainsKHR(
        device,
        swapchainCount,
        pCreateInfos,
        pAllocator,
        pSwapchains
    );
}

#endif /* VK_KHR_display_swapchain */
#ifdef VK_KHR_xlib_surface
#ifdef VK_USE_PLATFORM_XLIB_KHR
static PFN_vkCreateXlibSurfaceKHR pfn_vkCreateXlibSurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(
    VkInstance                                  instance,
    const VkXlibSurfaceCreateInfoKHR*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateXlibSurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

static PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR pfn_vkGetPhysicalDeviceXlibPresentationSupportKHR;
VkBool32 vkGetPhysicalDeviceXlibPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    Display*                                    dpy,
    VisualID                                    visualID)
{
    return pfn_vkGetPhysicalDeviceXlibPresentationSupportKHR(
        physicalDevice,
        queueFamilyIndex,
        dpy,
        visualID
    );
}

#endif /* VK_USE_PLATFORM_XLIB_KHR */
#endif /* VK_KHR_xlib_surface */
#ifdef VK_KHR_xcb_surface
#ifdef VK_USE_PLATFORM_XCB_KHR
static PFN_vkCreateXcbSurfaceKHR pfn_vkCreateXcbSurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateXcbSurfaceKHR(
    VkInstance                                  instance,
    const VkXcbSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateXcbSurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

static PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR pfn_vkGetPhysicalDeviceXcbPresentationSupportKHR;
VkBool32 vkGetPhysicalDeviceXcbPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    xcb_connection_t*                           connection,
    xcb_visualid_t                              visual_id)
{
    return pfn_vkGetPhysicalDeviceXcbPresentationSupportKHR(
        physicalDevice,
        queueFamilyIndex,
        connection,
        visual_id
    );
}

#endif /* VK_USE_PLATFORM_XCB_KHR */
#endif /* VK_KHR_xcb_surface */
#ifdef VK_KHR_wayland_surface
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
static PFN_vkCreateWaylandSurfaceKHR pfn_vkCreateWaylandSurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateWaylandSurfaceKHR(
    VkInstance                                  instance,
    const VkWaylandSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateWaylandSurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

static PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR pfn_vkGetPhysicalDeviceWaylandPresentationSupportKHR;
VkBool32 vkGetPhysicalDeviceWaylandPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    struct wl_display*                          display)
{
    return pfn_vkGetPhysicalDeviceWaylandPresentationSupportKHR(
        physicalDevice,
        queueFamilyIndex,
        display
    );
}

#endif /* VK_USE_PLATFORM_WAYLAND_KHR */
#endif /* VK_KHR_wayland_surface */
#ifdef VK_KHR_mir_surface
#ifdef VK_USE_PLATFORM_MIR_KHR
static PFN_vkCreateMirSurfaceKHR pfn_vkCreateMirSurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateMirSurfaceKHR(
    VkInstance                                  instance,
    const VkMirSurfaceCreateInfoKHR*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateMirSurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

static PFN_vkGetPhysicalDeviceMirPresentationSupportKHR pfn_vkGetPhysicalDeviceMirPresentationSupportKHR;
VkBool32 vkGetPhysicalDeviceMirPresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex,
    MirConnection*                              connection)
{
    return pfn_vkGetPhysicalDeviceMirPresentationSupportKHR(
        physicalDevice,
        queueFamilyIndex,
        connection
    );
}

#endif /* VK_USE_PLATFORM_MIR_KHR */
#endif /* VK_KHR_mir_surface */
#ifdef VK_KHR_android_surface
#ifdef VK_USE_PLATFORM_ANDROID_KHR
static PFN_vkCreateAndroidSurfaceKHR pfn_vkCreateAndroidSurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateAndroidSurfaceKHR(
    VkInstance                                  instance,
    const VkAndroidSurfaceCreateInfoKHR*        pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateAndroidSurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

#endif /* VK_USE_PLATFORM_ANDROID_KHR */
#endif /* VK_KHR_android_surface */
#ifdef VK_KHR_win32_surface
#ifdef VK_USE_PLATFORM_WIN32_KHR
static PFN_vkCreateWin32SurfaceKHR pfn_vkCreateWin32SurfaceKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(
    VkInstance                                  instance,
    const VkWin32SurfaceCreateInfoKHR*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateWin32SurfaceKHR(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

static PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR pfn_vkGetPhysicalDeviceWin32PresentationSupportKHR;
VkBool32 vkGetPhysicalDeviceWin32PresentationSupportKHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t                                    queueFamilyIndex)
{
    return pfn_vkGetPhysicalDeviceWin32PresentationSupportKHR(
        physicalDevice,
        queueFamilyIndex
    );
}

#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_win32_surface */
#ifdef VK_KHR_get_physical_device_properties2
static PFN_vkGetPhysicalDeviceFeatures2KHR pfn_vkGetPhysicalDeviceFeatures2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceFeatures2KHR*               pFeatures)
{
    pfn_vkGetPhysicalDeviceFeatures2KHR(
        physicalDevice,
        pFeatures
    );
}

static PFN_vkGetPhysicalDeviceProperties2KHR pfn_vkGetPhysicalDeviceProperties2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceProperties2KHR*             pProperties)
{
    pfn_vkGetPhysicalDeviceProperties2KHR(
        physicalDevice,
        pProperties
    );
}

static PFN_vkGetPhysicalDeviceFormatProperties2KHR pfn_vkGetPhysicalDeviceFormatProperties2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkFormatProperties2KHR*                     pFormatProperties)
{
    pfn_vkGetPhysicalDeviceFormatProperties2KHR(
        physicalDevice,
        format,
        pFormatProperties
    );
}

static PFN_vkGetPhysicalDeviceImageFormatProperties2KHR pfn_vkGetPhysicalDeviceImageFormatProperties2KHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceImageFormatInfo2KHR*  pImageFormatInfo,
    VkImageFormatProperties2KHR*                pImageFormatProperties)
{
    return pfn_vkGetPhysicalDeviceImageFormatProperties2KHR(
        physicalDevice,
        pImageFormatInfo,
        pImageFormatProperties
    );
}

static PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR pfn_vkGetPhysicalDeviceQueueFamilyProperties2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    uint32_t*                                   pQueueFamilyPropertyCount,
    VkQueueFamilyProperties2KHR*                pQueueFamilyProperties)
{
    pfn_vkGetPhysicalDeviceQueueFamilyProperties2KHR(
        physicalDevice,
        pQueueFamilyPropertyCount,
        pQueueFamilyProperties
    );
}

static PFN_vkGetPhysicalDeviceMemoryProperties2KHR pfn_vkGetPhysicalDeviceMemoryProperties2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    VkPhysicalDeviceMemoryProperties2KHR*       pMemoryProperties)
{
    pfn_vkGetPhysicalDeviceMemoryProperties2KHR(
        physicalDevice,
        pMemoryProperties
    );
}

static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR pfn_vkGetPhysicalDeviceSparseImageFormatProperties2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSparseImageFormatInfo2KHR* pFormatInfo,
    uint32_t*                                   pPropertyCount,
    VkSparseImageFormatProperties2KHR*          pProperties)
{
    pfn_vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
        physicalDevice,
        pFormatInfo,
        pPropertyCount,
        pProperties
    );
}

#endif /* VK_KHR_get_physical_device_properties2 */
#ifdef VK_KHR_maintenance1
static PFN_vkTrimCommandPoolKHR pfn_vkTrimCommandPoolKHR;
VKAPI_ATTR void VKAPI_CALL vkTrimCommandPoolKHR(
    VkDevice                                    device,
    VkCommandPool                               commandPool,
    VkCommandPoolTrimFlagsKHR                   flags)
{
    pfn_vkTrimCommandPoolKHR(
        device,
        commandPool,
        flags
    );
}

#endif /* VK_KHR_maintenance1 */
#ifdef VK_KHR_external_memory_capabilities
static PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR pfn_vkGetPhysicalDeviceExternalBufferPropertiesKHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalBufferPropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalBufferInfoKHR* pExternalBufferInfo,
    VkExternalBufferPropertiesKHR*              pExternalBufferProperties)
{
    pfn_vkGetPhysicalDeviceExternalBufferPropertiesKHR(
        physicalDevice,
        pExternalBufferInfo,
        pExternalBufferProperties
    );
}

#endif /* VK_KHR_external_memory_capabilities */
#ifdef VK_KHR_external_memory_win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
static PFN_vkGetMemoryWin32HandleKHR pfn_vkGetMemoryWin32HandleKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryWin32HandleKHR(
    VkDevice                                    device,
    const VkMemoryGetWin32HandleInfoKHR*        pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    return pfn_vkGetMemoryWin32HandleKHR(
        device,
        pGetWin32HandleInfo,
        pHandle
    );
}

static PFN_vkGetMemoryWin32HandlePropertiesKHR pfn_vkGetMemoryWin32HandlePropertiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryWin32HandlePropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBitsKHR       handleType,
    HANDLE                                      handle,
    VkMemoryWin32HandlePropertiesKHR*           pMemoryWin32HandleProperties)
{
    return pfn_vkGetMemoryWin32HandlePropertiesKHR(
        device,
        handleType,
        handle,
        pMemoryWin32HandleProperties
    );
}

#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_memory_win32 */
#ifdef VK_KHR_external_memory_fd
static PFN_vkGetMemoryFdKHR pfn_vkGetMemoryFdKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryFdKHR(
    VkDevice                                    device,
    const VkMemoryGetFdInfoKHR*                 pGetFdInfo,
    int*                                        pFd)
{
    return pfn_vkGetMemoryFdKHR(
        device,
        pGetFdInfo,
        pFd
    );
}

static PFN_vkGetMemoryFdPropertiesKHR pfn_vkGetMemoryFdPropertiesKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryFdPropertiesKHR(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBitsKHR       handleType,
    int                                         fd,
    VkMemoryFdPropertiesKHR*                    pMemoryFdProperties)
{
    return pfn_vkGetMemoryFdPropertiesKHR(
        device,
        handleType,
        fd,
        pMemoryFdProperties
    );
}

#endif /* VK_KHR_external_memory_fd */
#ifdef VK_KHR_external_semaphore_capabilities
static PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR pfn_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalSemaphoreInfoKHR* pExternalSemaphoreInfo,
    VkExternalSemaphorePropertiesKHR*           pExternalSemaphoreProperties)
{
    pfn_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR(
        physicalDevice,
        pExternalSemaphoreInfo,
        pExternalSemaphoreProperties
    );
}

#endif /* VK_KHR_external_semaphore_capabilities */
#ifdef VK_KHR_external_semaphore_win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
static PFN_vkImportSemaphoreWin32HandleKHR pfn_vkImportSemaphoreWin32HandleKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkImportSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkImportSemaphoreWin32HandleInfoKHR*  pImportSemaphoreWin32HandleInfo)
{
    return pfn_vkImportSemaphoreWin32HandleKHR(
        device,
        pImportSemaphoreWin32HandleInfo
    );
}

static PFN_vkGetSemaphoreWin32HandleKHR pfn_vkGetSemaphoreWin32HandleKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreWin32HandleKHR(
    VkDevice                                    device,
    const VkSemaphoreGetWin32HandleInfoKHR*     pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    return pfn_vkGetSemaphoreWin32HandleKHR(
        device,
        pGetWin32HandleInfo,
        pHandle
    );
}

#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_semaphore_win32 */
#ifdef VK_KHR_external_semaphore_fd
static PFN_vkImportSemaphoreFdKHR pfn_vkImportSemaphoreFdKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkImportSemaphoreFdKHR(
    VkDevice                                    device,
    const VkImportSemaphoreFdInfoKHR*           pImportSemaphoreFdInfo)
{
    return pfn_vkImportSemaphoreFdKHR(
        device,
        pImportSemaphoreFdInfo
    );
}

static PFN_vkGetSemaphoreFdKHR pfn_vkGetSemaphoreFdKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetSemaphoreFdKHR(
    VkDevice                                    device,
    const VkSemaphoreGetFdInfoKHR*              pGetFdInfo,
    int*                                        pFd)
{
    return pfn_vkGetSemaphoreFdKHR(
        device,
        pGetFdInfo,
        pFd
    );
}

#endif /* VK_KHR_external_semaphore_fd */
#ifdef VK_KHR_push_descriptor
static PFN_vkCmdPushDescriptorSetKHR pfn_vkCmdPushDescriptorSetKHR;
VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetKHR(
    VkCommandBuffer                             commandBuffer,
    VkPipelineBindPoint                         pipelineBindPoint,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites)
{
    pfn_vkCmdPushDescriptorSetKHR(
        commandBuffer,
        pipelineBindPoint,
        layout,
        set,
        descriptorWriteCount,
        pDescriptorWrites
    );
}

#endif /* VK_KHR_push_descriptor */
#ifdef VK_KHR_descriptor_update_template
static PFN_vkCreateDescriptorUpdateTemplateKHR pfn_vkCreateDescriptorUpdateTemplateKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    const VkDescriptorUpdateTemplateCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorUpdateTemplateKHR*              pDescriptorUpdateTemplate)
{
    return pfn_vkCreateDescriptorUpdateTemplateKHR(
        device,
        pCreateInfo,
        pAllocator,
        pDescriptorUpdateTemplate
    );
}

static PFN_vkDestroyDescriptorUpdateTemplateKHR pfn_vkDestroyDescriptorUpdateTemplateKHR;
VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorUpdateTemplateKHR(
    VkDevice                                    device,
    VkDescriptorUpdateTemplateKHR               descriptorUpdateTemplate,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroyDescriptorUpdateTemplateKHR(
        device,
        descriptorUpdateTemplate,
        pAllocator
    );
}

static PFN_vkUpdateDescriptorSetWithTemplateKHR pfn_vkUpdateDescriptorSetWithTemplateKHR;
VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSetWithTemplateKHR(
    VkDevice                                    device,
    VkDescriptorSet                             descriptorSet,
    VkDescriptorUpdateTemplateKHR               descriptorUpdateTemplate,
    const void*                                 pData)
{
    pfn_vkUpdateDescriptorSetWithTemplateKHR(
        device,
        descriptorSet,
        descriptorUpdateTemplate,
        pData
    );
}

static PFN_vkCmdPushDescriptorSetWithTemplateKHR pfn_vkCmdPushDescriptorSetWithTemplateKHR;
VKAPI_ATTR void VKAPI_CALL vkCmdPushDescriptorSetWithTemplateKHR(
    VkCommandBuffer                             commandBuffer,
    VkDescriptorUpdateTemplateKHR               descriptorUpdateTemplate,
    VkPipelineLayout                            layout,
    uint32_t                                    set,
    const void*                                 pData)
{
    pfn_vkCmdPushDescriptorSetWithTemplateKHR(
        commandBuffer,
        descriptorUpdateTemplate,
        layout,
        set,
        pData
    );
}

#endif /* VK_KHR_descriptor_update_template */
#ifdef VK_KHR_shared_presentable_image
static PFN_vkGetSwapchainStatusKHR pfn_vkGetSwapchainStatusKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainStatusKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain)
{
    return pfn_vkGetSwapchainStatusKHR(
        device,
        swapchain
    );
}

#endif /* VK_KHR_shared_presentable_image */
#ifdef VK_KHR_external_fence_capabilities
static PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR pfn_vkGetPhysicalDeviceExternalFencePropertiesKHR;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceExternalFencePropertiesKHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceExternalFenceInfoKHR* pExternalFenceInfo,
    VkExternalFencePropertiesKHR*               pExternalFenceProperties)
{
    pfn_vkGetPhysicalDeviceExternalFencePropertiesKHR(
        physicalDevice,
        pExternalFenceInfo,
        pExternalFenceProperties
    );
}

#endif /* VK_KHR_external_fence_capabilities */
#ifdef VK_KHR_external_fence_win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
static PFN_vkImportFenceWin32HandleKHR pfn_vkImportFenceWin32HandleKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkImportFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkImportFenceWin32HandleInfoKHR*      pImportFenceWin32HandleInfo)
{
    return pfn_vkImportFenceWin32HandleKHR(
        device,
        pImportFenceWin32HandleInfo
    );
}

static PFN_vkGetFenceWin32HandleKHR pfn_vkGetFenceWin32HandleKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceWin32HandleKHR(
    VkDevice                                    device,
    const VkFenceGetWin32HandleInfoKHR*         pGetWin32HandleInfo,
    HANDLE*                                     pHandle)
{
    return pfn_vkGetFenceWin32HandleKHR(
        device,
        pGetWin32HandleInfo,
        pHandle
    );
}

#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_fence_win32 */
#ifdef VK_KHR_external_fence_fd
static PFN_vkImportFenceFdKHR pfn_vkImportFenceFdKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkImportFenceFdKHR(
    VkDevice                                    device,
    const VkImportFenceFdInfoKHR*               pImportFenceFdInfo)
{
    return pfn_vkImportFenceFdKHR(
        device,
        pImportFenceFdInfo
    );
}

static PFN_vkGetFenceFdKHR pfn_vkGetFenceFdKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceFdKHR(
    VkDevice                                    device,
    const VkFenceGetFdInfoKHR*                  pGetFdInfo,
    int*                                        pFd)
{
    return pfn_vkGetFenceFdKHR(
        device,
        pGetFdInfo,
        pFd
    );
}

#endif /* VK_KHR_external_fence_fd */
#ifdef VK_KHR_get_surface_capabilities2
static PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR pfn_vkGetPhysicalDeviceSurfaceCapabilities2KHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    VkSurfaceCapabilities2KHR*                  pSurfaceCapabilities)
{
    return pfn_vkGetPhysicalDeviceSurfaceCapabilities2KHR(
        physicalDevice,
        pSurfaceInfo,
        pSurfaceCapabilities
    );
}

static PFN_vkGetPhysicalDeviceSurfaceFormats2KHR pfn_vkGetPhysicalDeviceSurfaceFormats2KHR;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormats2KHR(
    VkPhysicalDevice                            physicalDevice,
    const VkPhysicalDeviceSurfaceInfo2KHR*      pSurfaceInfo,
    uint32_t*                                   pSurfaceFormatCount,
    VkSurfaceFormat2KHR*                        pSurfaceFormats)
{
    return pfn_vkGetPhysicalDeviceSurfaceFormats2KHR(
        physicalDevice,
        pSurfaceInfo,
        pSurfaceFormatCount,
        pSurfaceFormats
    );
}

#endif /* VK_KHR_get_surface_capabilities2 */
#ifdef VK_KHR_get_memory_requirements2
static PFN_vkGetImageMemoryRequirements2KHR pfn_vkGetImageMemoryRequirements2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageMemoryRequirementsInfo2KHR*    pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements)
{
    pfn_vkGetImageMemoryRequirements2KHR(
        device,
        pInfo,
        pMemoryRequirements
    );
}

static PFN_vkGetBufferMemoryRequirements2KHR pfn_vkGetBufferMemoryRequirements2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkBufferMemoryRequirementsInfo2KHR*   pInfo,
    VkMemoryRequirements2KHR*                   pMemoryRequirements)
{
    pfn_vkGetBufferMemoryRequirements2KHR(
        device,
        pInfo,
        pMemoryRequirements
    );
}

static PFN_vkGetImageSparseMemoryRequirements2KHR pfn_vkGetImageSparseMemoryRequirements2KHR;
VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements2KHR(
    VkDevice                                    device,
    const VkImageSparseMemoryRequirementsInfo2KHR* pInfo,
    uint32_t*                                   pSparseMemoryRequirementCount,
    VkSparseImageMemoryRequirements2KHR*        pSparseMemoryRequirements)
{
    pfn_vkGetImageSparseMemoryRequirements2KHR(
        device,
        pInfo,
        pSparseMemoryRequirementCount,
        pSparseMemoryRequirements
    );
}

#endif /* VK_KHR_get_memory_requirements2 */
#ifdef VK_KHR_sampler_ycbcr_conversion
static PFN_vkCreateSamplerYcbcrConversionKHR pfn_vkCreateSamplerYcbcrConversionKHR;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSamplerYcbcrConversionKHR(
    VkDevice                                    device,
    const VkSamplerYcbcrConversionCreateInfoKHR* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSamplerYcbcrConversionKHR*                pYcbcrConversion)
{
    return pfn_vkCreateSamplerYcbcrConversionKHR(
        device,
        pCreateInfo,
        pAllocator,
        pYcbcrConversion
    );
}

static PFN_vkDestroySamplerYcbcrConversionKHR pfn_vkDestroySamplerYcbcrConversionKHR;
VKAPI_ATTR void VKAPI_CALL vkDestroySamplerYcbcrConversionKHR(
    VkDevice                                    device,
    VkSamplerYcbcrConversionKHR                 ycbcrConversion,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroySamplerYcbcrConversionKHR(
        device,
        ycbcrConversion,
        pAllocator
    );
}

#endif /* VK_KHR_sampler_ycbcr_conversion */
#ifdef VK_KHR_bind_memory2
static PFN_vkBindBufferMemory2KHR pfn_vkBindBufferMemory2KHR;
VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindBufferMemoryInfoKHR*            pBindInfos)
{
    return pfn_vkBindBufferMemory2KHR(
        device,
        bindInfoCount,
        pBindInfos
    );
}

static PFN_vkBindImageMemory2KHR pfn_vkBindImageMemory2KHR;
VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory2KHR(
    VkDevice                                    device,
    uint32_t                                    bindInfoCount,
    const VkBindImageMemoryInfoKHR*             pBindInfos)
{
    return pfn_vkBindImageMemory2KHR(
        device,
        bindInfoCount,
        pBindInfos
    );
}

#endif /* VK_KHR_bind_memory2 */
#ifdef VK_ANDROID_native_buffer
static PFN_vkGetSwapchainGrallocUsageANDROID pfn_vkGetSwapchainGrallocUsageANDROID;
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainGrallocUsageANDROID(
    VkDevice                                    device,
    VkFormat                                    format,
    VkImageUsageFlags                           imageUsage,
    int*                                        grallocUsage)
{
    return pfn_vkGetSwapchainGrallocUsageANDROID(
        device,
        format,
        imageUsage,
        grallocUsage
    );
}

static PFN_vkAcquireImageANDROID pfn_vkAcquireImageANDROID;
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireImageANDROID(
    VkDevice                                    device,
    VkImage                                     image,
    int                                         nativeFenceFd,
    VkSemaphore                                 semaphore,
    VkFence                                     fence)
{
    return pfn_vkAcquireImageANDROID(
        device,
        image,
        nativeFenceFd,
        semaphore,
        fence
    );
}

static PFN_vkQueueSignalReleaseImageANDROID pfn_vkQueueSignalReleaseImageANDROID;
VKAPI_ATTR VkResult VKAPI_CALL vkQueueSignalReleaseImageANDROID(
    VkQueue                                     queue,
    uint32_t                                    waitSemaphoreCount,
    const VkSemaphore*                          pWaitSemaphores,
    VkImage                                     image,
    int*                                        pNativeFenceFd)
{
    return pfn_vkQueueSignalReleaseImageANDROID(
        queue,
        waitSemaphoreCount,
        pWaitSemaphores,
        image,
        pNativeFenceFd
    );
}

#endif /* VK_ANDROID_native_buffer */
#ifdef VK_EXT_debug_report
static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
    VkInstance                                  instance,
    const VkDebugReportCallbackCreateInfoEXT*   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDebugReportCallbackEXT*                   pCallback)
{
    return pfn_vkCreateDebugReportCallbackEXT(
        instance,
        pCreateInfo,
        pAllocator,
        pCallback
    );
}

static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
    VkInstance                                  instance,
    VkDebugReportCallbackEXT                    callback,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroyDebugReportCallbackEXT(
        instance,
        callback,
        pAllocator
    );
}

static PFN_vkDebugReportMessageEXT pfn_vkDebugReportMessageEXT;
VKAPI_ATTR void VKAPI_CALL vkDebugReportMessageEXT(
    VkInstance                                  instance,
    VkDebugReportFlagsEXT                       flags,
    VkDebugReportObjectTypeEXT                  objectType,
    uint64_t                                    object,
    size_t                                      location,
    int32_t                                     messageCode,
    const char*                                 pLayerPrefix,
    const char*                                 pMessage)
{
    pfn_vkDebugReportMessageEXT(
        instance,
        flags,
        objectType,
        object,
        location,
        messageCode,
        pLayerPrefix,
        pMessage
    );
}

#endif /* VK_EXT_debug_report */
#ifdef VK_EXT_debug_marker
static PFN_vkDebugMarkerSetObjectTagEXT pfn_vkDebugMarkerSetObjectTagEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectTagEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectTagInfoEXT*        pTagInfo)
{
    return pfn_vkDebugMarkerSetObjectTagEXT(
        device,
        pTagInfo
    );
}

static PFN_vkDebugMarkerSetObjectNameEXT pfn_vkDebugMarkerSetObjectNameEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectNameEXT(
    VkDevice                                    device,
    const VkDebugMarkerObjectNameInfoEXT*       pNameInfo)
{
    return pfn_vkDebugMarkerSetObjectNameEXT(
        device,
        pNameInfo
    );
}

static PFN_vkCmdDebugMarkerBeginEXT pfn_vkCmdDebugMarkerBeginEXT;
VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerBeginEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo)
{
    pfn_vkCmdDebugMarkerBeginEXT(
        commandBuffer,
        pMarkerInfo
    );
}

static PFN_vkCmdDebugMarkerEndEXT pfn_vkCmdDebugMarkerEndEXT;
VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerEndEXT(
    VkCommandBuffer                             commandBuffer)
{
    pfn_vkCmdDebugMarkerEndEXT(
        commandBuffer
    );
}

static PFN_vkCmdDebugMarkerInsertEXT pfn_vkCmdDebugMarkerInsertEXT;
VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerInsertEXT(
    VkCommandBuffer                             commandBuffer,
    const VkDebugMarkerMarkerInfoEXT*           pMarkerInfo)
{
    pfn_vkCmdDebugMarkerInsertEXT(
        commandBuffer,
        pMarkerInfo
    );
}

#endif /* VK_EXT_debug_marker */
#ifdef VK_AMD_draw_indirect_count
static PFN_vkCmdDrawIndirectCountAMD pfn_vkCmdDrawIndirectCountAMD;
VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    pfn_vkCmdDrawIndirectCountAMD(
        commandBuffer,
        buffer,
        offset,
        countBuffer,
        countBufferOffset,
        maxDrawCount,
        stride
    );
}

static PFN_vkCmdDrawIndexedIndirectCountAMD pfn_vkCmdDrawIndexedIndirectCountAMD;
VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirectCountAMD(
    VkCommandBuffer                             commandBuffer,
    VkBuffer                                    buffer,
    VkDeviceSize                                offset,
    VkBuffer                                    countBuffer,
    VkDeviceSize                                countBufferOffset,
    uint32_t                                    maxDrawCount,
    uint32_t                                    stride)
{
    pfn_vkCmdDrawIndexedIndirectCountAMD(
        commandBuffer,
        buffer,
        offset,
        countBuffer,
        countBufferOffset,
        maxDrawCount,
        stride
    );
}

#endif /* VK_AMD_draw_indirect_count */
#ifdef VK_AMD_shader_info
static PFN_vkGetShaderInfoAMD pfn_vkGetShaderInfoAMD;
VKAPI_ATTR VkResult VKAPI_CALL vkGetShaderInfoAMD(
    VkDevice                                    device,
    VkPipeline                                  pipeline,
    VkShaderStageFlagBits                       shaderStage,
    VkShaderInfoTypeAMD                         infoType,
    size_t*                                     pInfoSize,
    void*                                       pInfo)
{
    return pfn_vkGetShaderInfoAMD(
        device,
        pipeline,
        shaderStage,
        infoType,
        pInfoSize,
        pInfo
    );
}

#endif /* VK_AMD_shader_info */
#ifdef VK_NV_external_memory_capabilities
static PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV pfn_vkGetPhysicalDeviceExternalImageFormatPropertiesNV;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkExternalMemoryHandleTypeFlagsNV           externalHandleType,
    VkExternalImageFormatPropertiesNV*          pExternalImageFormatProperties)
{
    return pfn_vkGetPhysicalDeviceExternalImageFormatPropertiesNV(
        physicalDevice,
        format,
        type,
        tiling,
        usage,
        flags,
        externalHandleType,
        pExternalImageFormatProperties
    );
}

#endif /* VK_NV_external_memory_capabilities */
#ifdef VK_NV_external_memory_win32
#ifdef VK_USE_PLATFORM_WIN32_KHR
static PFN_vkGetMemoryWin32HandleNV pfn_vkGetMemoryWin32HandleNV;
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryWin32HandleNV(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkExternalMemoryHandleTypeFlagsNV           handleType,
    HANDLE*                                     pHandle)
{
    return pfn_vkGetMemoryWin32HandleNV(
        device,
        memory,
        handleType,
        pHandle
    );
}

#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_NV_external_memory_win32 */
#ifdef VK_KHX_device_group
static PFN_vkGetDeviceGroupPeerMemoryFeaturesKHX pfn_vkGetDeviceGroupPeerMemoryFeaturesKHX;
VKAPI_ATTR void VKAPI_CALL vkGetDeviceGroupPeerMemoryFeaturesKHX(
    VkDevice                                    device,
    uint32_t                                    heapIndex,
    uint32_t                                    localDeviceIndex,
    uint32_t                                    remoteDeviceIndex,
    VkPeerMemoryFeatureFlagsKHX*                pPeerMemoryFeatures)
{
    pfn_vkGetDeviceGroupPeerMemoryFeaturesKHX(
        device,
        heapIndex,
        localDeviceIndex,
        remoteDeviceIndex,
        pPeerMemoryFeatures
    );
}

static PFN_vkCmdSetDeviceMaskKHX pfn_vkCmdSetDeviceMaskKHX;
VKAPI_ATTR void VKAPI_CALL vkCmdSetDeviceMaskKHX(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    deviceMask)
{
    pfn_vkCmdSetDeviceMaskKHX(
        commandBuffer,
        deviceMask
    );
}

static PFN_vkCmdDispatchBaseKHX pfn_vkCmdDispatchBaseKHX;
VKAPI_ATTR void VKAPI_CALL vkCmdDispatchBaseKHX(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    baseGroupX,
    uint32_t                                    baseGroupY,
    uint32_t                                    baseGroupZ,
    uint32_t                                    groupCountX,
    uint32_t                                    groupCountY,
    uint32_t                                    groupCountZ)
{
    pfn_vkCmdDispatchBaseKHX(
        commandBuffer,
        baseGroupX,
        baseGroupY,
        baseGroupZ,
        groupCountX,
        groupCountY,
        groupCountZ
    );
}

static PFN_vkGetDeviceGroupPresentCapabilitiesKHX pfn_vkGetDeviceGroupPresentCapabilitiesKHX;
VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceGroupPresentCapabilitiesKHX(
    VkDevice                                    device,
    VkDeviceGroupPresentCapabilitiesKHX*        pDeviceGroupPresentCapabilities)
{
    return pfn_vkGetDeviceGroupPresentCapabilitiesKHX(
        device,
        pDeviceGroupPresentCapabilities
    );
}

static PFN_vkGetDeviceGroupSurfacePresentModesKHX pfn_vkGetDeviceGroupSurfacePresentModesKHX;
VKAPI_ATTR VkResult VKAPI_CALL vkGetDeviceGroupSurfacePresentModesKHX(
    VkDevice                                    device,
    VkSurfaceKHR                                surface,
    VkDeviceGroupPresentModeFlagsKHX*           pModes)
{
    return pfn_vkGetDeviceGroupSurfacePresentModesKHX(
        device,
        surface,
        pModes
    );
}

static PFN_vkGetPhysicalDevicePresentRectanglesKHX pfn_vkGetPhysicalDevicePresentRectanglesKHX;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDevicePresentRectanglesKHX(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    uint32_t*                                   pRectCount,
    VkRect2D*                                   pRects)
{
    return pfn_vkGetPhysicalDevicePresentRectanglesKHX(
        physicalDevice,
        surface,
        pRectCount,
        pRects
    );
}

static PFN_vkAcquireNextImage2KHX pfn_vkAcquireNextImage2KHX;
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImage2KHX(
    VkDevice                                    device,
    const VkAcquireNextImageInfoKHX*            pAcquireInfo,
    uint32_t*                                   pImageIndex)
{
    return pfn_vkAcquireNextImage2KHX(
        device,
        pAcquireInfo,
        pImageIndex
    );
}

#endif /* VK_KHX_device_group */
#ifdef VK_NN_vi_surface
#ifdef VK_USE_PLATFORM_VI_NN
static PFN_vkCreateViSurfaceNN pfn_vkCreateViSurfaceNN;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateViSurfaceNN(
    VkInstance                                  instance,
    const VkViSurfaceCreateInfoNN*              pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateViSurfaceNN(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

#endif /* VK_USE_PLATFORM_VI_NN */
#endif /* VK_NN_vi_surface */
#ifdef VK_KHX_device_group_creation
static PFN_vkEnumeratePhysicalDeviceGroupsKHX pfn_vkEnumeratePhysicalDeviceGroupsKHX;
VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroupsKHX(
    VkInstance                                  instance,
    uint32_t*                                   pPhysicalDeviceGroupCount,
    VkPhysicalDeviceGroupPropertiesKHX*         pPhysicalDeviceGroupProperties)
{
    return pfn_vkEnumeratePhysicalDeviceGroupsKHX(
        instance,
        pPhysicalDeviceGroupCount,
        pPhysicalDeviceGroupProperties
    );
}

#endif /* VK_KHX_device_group_creation */
#ifdef VK_NVX_device_generated_commands
static PFN_vkCmdProcessCommandsNVX pfn_vkCmdProcessCommandsNVX;
VKAPI_ATTR void VKAPI_CALL vkCmdProcessCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdProcessCommandsInfoNVX*          pProcessCommandsInfo)
{
    pfn_vkCmdProcessCommandsNVX(
        commandBuffer,
        pProcessCommandsInfo
    );
}

static PFN_vkCmdReserveSpaceForCommandsNVX pfn_vkCmdReserveSpaceForCommandsNVX;
VKAPI_ATTR void VKAPI_CALL vkCmdReserveSpaceForCommandsNVX(
    VkCommandBuffer                             commandBuffer,
    const VkCmdReserveSpaceForCommandsInfoNVX*  pReserveSpaceInfo)
{
    pfn_vkCmdReserveSpaceForCommandsNVX(
        commandBuffer,
        pReserveSpaceInfo
    );
}

static PFN_vkCreateIndirectCommandsLayoutNVX pfn_vkCreateIndirectCommandsLayoutNVX;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    const VkIndirectCommandsLayoutCreateInfoNVX* pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkIndirectCommandsLayoutNVX*                pIndirectCommandsLayout)
{
    return pfn_vkCreateIndirectCommandsLayoutNVX(
        device,
        pCreateInfo,
        pAllocator,
        pIndirectCommandsLayout
    );
}

static PFN_vkDestroyIndirectCommandsLayoutNVX pfn_vkDestroyIndirectCommandsLayoutNVX;
VKAPI_ATTR void VKAPI_CALL vkDestroyIndirectCommandsLayoutNVX(
    VkDevice                                    device,
    VkIndirectCommandsLayoutNVX                 indirectCommandsLayout,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroyIndirectCommandsLayoutNVX(
        device,
        indirectCommandsLayout,
        pAllocator
    );
}

static PFN_vkCreateObjectTableNVX pfn_vkCreateObjectTableNVX;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateObjectTableNVX(
    VkDevice                                    device,
    const VkObjectTableCreateInfoNVX*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkObjectTableNVX*                           pObjectTable)
{
    return pfn_vkCreateObjectTableNVX(
        device,
        pCreateInfo,
        pAllocator,
        pObjectTable
    );
}

static PFN_vkDestroyObjectTableNVX pfn_vkDestroyObjectTableNVX;
VKAPI_ATTR void VKAPI_CALL vkDestroyObjectTableNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroyObjectTableNVX(
        device,
        objectTable,
        pAllocator
    );
}

static PFN_vkRegisterObjectsNVX pfn_vkRegisterObjectsNVX;
VKAPI_ATTR VkResult VKAPI_CALL vkRegisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectTableEntryNVX* const*         ppObjectTableEntries,
    const uint32_t*                             pObjectIndices)
{
    return pfn_vkRegisterObjectsNVX(
        device,
        objectTable,
        objectCount,
        ppObjectTableEntries,
        pObjectIndices
    );
}

static PFN_vkUnregisterObjectsNVX pfn_vkUnregisterObjectsNVX;
VKAPI_ATTR VkResult VKAPI_CALL vkUnregisterObjectsNVX(
    VkDevice                                    device,
    VkObjectTableNVX                            objectTable,
    uint32_t                                    objectCount,
    const VkObjectEntryTypeNVX*                 pObjectEntryTypes,
    const uint32_t*                             pObjectIndices)
{
    return pfn_vkUnregisterObjectsNVX(
        device,
        objectTable,
        objectCount,
        pObjectEntryTypes,
        pObjectIndices
    );
}

static PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX pfn_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
    VkPhysicalDevice                            physicalDevice,
    VkDeviceGeneratedCommandsFeaturesNVX*       pFeatures,
    VkDeviceGeneratedCommandsLimitsNVX*         pLimits)
{
    pfn_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX(
        physicalDevice,
        pFeatures,
        pLimits
    );
}

#endif /* VK_NVX_device_generated_commands */
#ifdef VK_NV_clip_space_w_scaling
static PFN_vkCmdSetViewportWScalingNV pfn_vkCmdSetViewportWScalingNV;
VKAPI_ATTR void VKAPI_CALL vkCmdSetViewportWScalingNV(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstViewport,
    uint32_t                                    viewportCount,
    const VkViewportWScalingNV*                 pViewportWScalings)
{
    pfn_vkCmdSetViewportWScalingNV(
        commandBuffer,
        firstViewport,
        viewportCount,
        pViewportWScalings
    );
}

#endif /* VK_NV_clip_space_w_scaling */
#ifdef VK_EXT_direct_mode_display
static PFN_vkReleaseDisplayEXT pfn_vkReleaseDisplayEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display)
{
    return pfn_vkReleaseDisplayEXT(
        physicalDevice,
        display
    );
}

#endif /* VK_EXT_direct_mode_display */
#ifdef VK_EXT_acquire_xlib_display
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
static PFN_vkAcquireXlibDisplayEXT pfn_vkAcquireXlibDisplayEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display)
{
    return pfn_vkAcquireXlibDisplayEXT(
        physicalDevice,
        dpy,
        display
    );
}

static PFN_vkGetRandROutputDisplayEXT pfn_vkGetRandROutputDisplayEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay)
{
    return pfn_vkGetRandROutputDisplayEXT(
        physicalDevice,
        dpy,
        rrOutput,
        pDisplay
    );
}

#endif /* VK_USE_PLATFORM_XLIB_XRANDR_EXT */
#endif /* VK_EXT_acquire_xlib_display */
#ifdef VK_EXT_display_surface_counter
static PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT pfn_vkGetPhysicalDeviceSurfaceCapabilities2EXT;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities)
{
    return pfn_vkGetPhysicalDeviceSurfaceCapabilities2EXT(
        physicalDevice,
        surface,
        pSurfaceCapabilities
    );
}

#endif /* VK_EXT_display_surface_counter */
#ifdef VK_EXT_display_control
static PFN_vkDisplayPowerControlEXT pfn_vkDisplayPowerControlEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkDisplayPowerControlEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayPowerInfoEXT*                pDisplayPowerInfo)
{
    return pfn_vkDisplayPowerControlEXT(
        device,
        display,
        pDisplayPowerInfo
    );
}

static PFN_vkRegisterDeviceEventEXT pfn_vkRegisterDeviceEventEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDeviceEventEXT(
    VkDevice                                    device,
    const VkDeviceEventInfoEXT*                 pDeviceEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    return pfn_vkRegisterDeviceEventEXT(
        device,
        pDeviceEventInfo,
        pAllocator,
        pFence
    );
}

static PFN_vkRegisterDisplayEventEXT pfn_vkRegisterDisplayEventEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
    return pfn_vkRegisterDisplayEventEXT(
        device,
        display,
        pDisplayEventInfo,
        pAllocator,
        pFence
    );
}

static PFN_vkGetSwapchainCounterEXT pfn_vkGetSwapchainCounterEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue)
{
    return pfn_vkGetSwapchainCounterEXT(
        device,
        swapchain,
        counter,
        pCounterValue
    );
}

#endif /* VK_EXT_display_control */
#ifdef VK_GOOGLE_display_timing
static PFN_vkGetRefreshCycleDurationGOOGLE pfn_vkGetRefreshCycleDurationGOOGLE;
VKAPI_ATTR VkResult VKAPI_CALL vkGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties)
{
    return pfn_vkGetRefreshCycleDurationGOOGLE(
        device,
        swapchain,
        pDisplayTimingProperties
    );
}

static PFN_vkGetPastPresentationTimingGOOGLE pfn_vkGetPastPresentationTimingGOOGLE;
VKAPI_ATTR VkResult VKAPI_CALL vkGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings)
{
    return pfn_vkGetPastPresentationTimingGOOGLE(
        device,
        swapchain,
        pPresentationTimingCount,
        pPresentationTimings
    );
}

#endif /* VK_GOOGLE_display_timing */
#ifdef VK_EXT_discard_rectangles
static PFN_vkCmdSetDiscardRectangleEXT pfn_vkCmdSetDiscardRectangleEXT;
VKAPI_ATTR void VKAPI_CALL vkCmdSetDiscardRectangleEXT(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstDiscardRectangle,
    uint32_t                                    discardRectangleCount,
    const VkRect2D*                             pDiscardRectangles)
{
    pfn_vkCmdSetDiscardRectangleEXT(
        commandBuffer,
        firstDiscardRectangle,
        discardRectangleCount,
        pDiscardRectangles
    );
}

#endif /* VK_EXT_discard_rectangles */
#ifdef VK_EXT_hdr_metadata
static PFN_vkSetHdrMetadataEXT pfn_vkSetHdrMetadataEXT;
VKAPI_ATTR void VKAPI_CALL vkSetHdrMetadataEXT(
    VkDevice                                    device,
    uint32_t                                    swapchainCount,
    const VkSwapchainKHR*                       pSwapchains,
    const VkHdrMetadataEXT*                     pMetadata)
{
    pfn_vkSetHdrMetadataEXT(
        device,
        swapchainCount,
        pSwapchains,
        pMetadata
    );
}

#endif /* VK_EXT_hdr_metadata */
#ifdef VK_MVK_ios_surface
#ifdef VK_USE_PLATFORM_IOS_MVK
static PFN_vkCreateIOSSurfaceMVK pfn_vkCreateIOSSurfaceMVK;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateIOSSurfaceMVK(
    VkInstance                                  instance,
    const VkIOSSurfaceCreateInfoMVK*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateIOSSurfaceMVK(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

#endif /* VK_USE_PLATFORM_IOS_MVK */
#endif /* VK_MVK_ios_surface */
#ifdef VK_MVK_macos_surface
#ifdef VK_USE_PLATFORM_MACOS_MVK
static PFN_vkCreateMacOSSurfaceMVK pfn_vkCreateMacOSSurfaceMVK;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateMacOSSurfaceMVK(
    VkInstance                                  instance,
    const VkMacOSSurfaceCreateInfoMVK*          pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkSurfaceKHR*                               pSurface)
{
    return pfn_vkCreateMacOSSurfaceMVK(
        instance,
        pCreateInfo,
        pAllocator,
        pSurface
    );
}

#endif /* VK_USE_PLATFORM_MACOS_MVK */
#endif /* VK_MVK_macos_surface */
#ifdef VK_EXT_sample_locations
static PFN_vkCmdSetSampleLocationsEXT pfn_vkCmdSetSampleLocationsEXT;
VKAPI_ATTR void VKAPI_CALL vkCmdSetSampleLocationsEXT(
    VkCommandBuffer                             commandBuffer,
    const VkSampleLocationsInfoEXT*             pSampleLocationsInfo)
{
    pfn_vkCmdSetSampleLocationsEXT(
        commandBuffer,
        pSampleLocationsInfo
    );
}

static PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT pfn_vkGetPhysicalDeviceMultisamplePropertiesEXT;
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMultisamplePropertiesEXT(
    VkPhysicalDevice                            physicalDevice,
    VkSampleCountFlagBits                       samples,
    VkMultisamplePropertiesEXT*                 pMultisampleProperties)
{
    pfn_vkGetPhysicalDeviceMultisamplePropertiesEXT(
        physicalDevice,
        samples,
        pMultisampleProperties
    );
}

#endif /* VK_EXT_sample_locations */
#ifdef VK_EXT_validation_cache
static PFN_vkCreateValidationCacheEXT pfn_vkCreateValidationCacheEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkCreateValidationCacheEXT(
    VkDevice                                    device,
    const VkValidationCacheCreateInfoEXT*       pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkValidationCacheEXT*                       pValidationCache)
{
    return pfn_vkCreateValidationCacheEXT(
        device,
        pCreateInfo,
        pAllocator,
        pValidationCache
    );
}

static PFN_vkDestroyValidationCacheEXT pfn_vkDestroyValidationCacheEXT;
VKAPI_ATTR void VKAPI_CALL vkDestroyValidationCacheEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    const VkAllocationCallbacks*                pAllocator)
{
    pfn_vkDestroyValidationCacheEXT(
        device,
        validationCache,
        pAllocator
    );
}

static PFN_vkMergeValidationCachesEXT pfn_vkMergeValidationCachesEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkMergeValidationCachesEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        dstCache,
    uint32_t                                    srcCacheCount,
    const VkValidationCacheEXT*                 pSrcCaches)
{
    return pfn_vkMergeValidationCachesEXT(
        device,
        dstCache,
        srcCacheCount,
        pSrcCaches
    );
}

static PFN_vkGetValidationCacheDataEXT pfn_vkGetValidationCacheDataEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkGetValidationCacheDataEXT(
    VkDevice                                    device,
    VkValidationCacheEXT                        validationCache,
    size_t*                                     pDataSize,
    void*                                       pData)
{
    return pfn_vkGetValidationCacheDataEXT(
        device,
        validationCache,
        pDataSize,
        pData
    );
}

#endif /* VK_EXT_validation_cache */
#ifdef VK_EXT_external_memory_host
static PFN_vkGetMemoryHostPointerPropertiesEXT pfn_vkGetMemoryHostPointerPropertiesEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkGetMemoryHostPointerPropertiesEXT(
    VkDevice                                    device,
    VkExternalMemoryHandleTypeFlagBitsKHR       handleType,
    const void*                                 pHostPointer,
    VkMemoryHostPointerPropertiesEXT*           pMemoryHostPointerProperties)
{
    return pfn_vkGetMemoryHostPointerPropertiesEXT(
        device,
        handleType,
        pHostPointer,
        pMemoryHostPointerProperties
    );
}

#endif /* VK_EXT_external_memory_host */
#ifdef VK_AMD_buffer_marker
static PFN_vkCmdWriteBufferMarkerAMD pfn_vkCmdWriteBufferMarkerAMD;
VKAPI_ATTR void VKAPI_CALL vkCmdWriteBufferMarkerAMD(
    VkCommandBuffer                             commandBuffer,
    VkPipelineStageFlagBits                     pipelineStage,
    VkBuffer                                    dstBuffer,
    VkDeviceSize                                dstOffset,
    uint32_t                                    marker)
{
    pfn_vkCmdWriteBufferMarkerAMD(
        commandBuffer,
        pipelineStage,
        dstBuffer,
        dstOffset,
        marker
    );
}

#endif /* VK_AMD_buffer_marker */

void vkExtInitInstance(VkInstance instance)
{
#ifdef VK_KHR_surface
    pfn_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR");
    pfn_vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR");
    pfn_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    pfn_vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    pfn_vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR");
#endif /* VK_KHR_surface */
#ifdef VK_KHR_swapchain
    pfn_vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
    pfn_vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetInstanceProcAddr(instance, "vkDestroySwapchainKHR");
    pfn_vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetInstanceProcAddr(instance, "vkGetSwapchainImagesKHR");
    pfn_vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetInstanceProcAddr(instance, "vkAcquireNextImageKHR");
    pfn_vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetInstanceProcAddr(instance, "vkQueuePresentKHR");
#endif /* VK_KHR_swapchain */
#ifdef VK_KHR_display
    pfn_vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    pfn_vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    pfn_vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vkGetInstanceProcAddr(instance, "vkGetDisplayPlaneSupportedDisplaysKHR");
    pfn_vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetDisplayModePropertiesKHR");
    pfn_vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)vkGetInstanceProcAddr(instance, "vkCreateDisplayModeKHR");
    pfn_vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)vkGetInstanceProcAddr(instance, "vkGetDisplayPlaneCapabilitiesKHR");
    pfn_vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateDisplayPlaneSurfaceKHR");
#endif /* VK_KHR_display */
#ifdef VK_KHR_display_swapchain
    pfn_vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)vkGetInstanceProcAddr(instance, "vkCreateSharedSwapchainsKHR");
#endif /* VK_KHR_display_swapchain */
#ifdef VK_KHR_xlib_surface
#ifndef VK_KHR_xlib_surface
    pfn_vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
    pfn_vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XLIB_KHR */
#endif /* VK_KHR_xlib_surface */
#ifdef VK_KHR_xcb_surface
#ifndef VK_KHR_xcb_surface
    pfn_vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
    pfn_vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XCB_KHR */
#endif /* VK_KHR_xcb_surface */
#ifdef VK_KHR_wayland_surface
#ifndef VK_KHR_wayland_surface
    pfn_vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWaylandSurfaceKHR");
    pfn_vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */
#endif /* VK_KHR_wayland_surface */
#ifdef VK_KHR_mir_surface
#ifndef VK_KHR_mir_surface
    pfn_vkCreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateMirSurfaceKHR");
    pfn_vkGetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_MIR_KHR */
#endif /* VK_KHR_mir_surface */
#ifdef VK_KHR_android_surface
#ifndef VK_KHR_android_surface
    pfn_vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateAndroidSurfaceKHR");
#endif /* VK_USE_PLATFORM_ANDROID_KHR */
#endif /* VK_KHR_android_surface */
#ifdef VK_KHR_win32_surface
#ifndef VK_KHR_win32_surface
    pfn_vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
    pfn_vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_win32_surface */
#ifdef VK_KHR_get_physical_device_properties2
    pfn_vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFeatures2KHR");
    pfn_vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2KHR");
    pfn_vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceFormatProperties2KHR");
    pfn_vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    pfn_vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    pfn_vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2KHR");
    pfn_vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
#endif /* VK_KHR_get_physical_device_properties2 */
#ifdef VK_KHR_maintenance1
    pfn_vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)vkGetInstanceProcAddr(instance, "vkTrimCommandPoolKHR");
#endif /* VK_KHR_maintenance1 */
#ifdef VK_KHR_external_memory_capabilities
    pfn_vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
#endif /* VK_KHR_external_memory_capabilities */
#ifdef VK_KHR_external_memory_win32
#ifndef VK_KHR_external_memory_win32
    pfn_vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetInstanceProcAddr(instance, "vkGetMemoryWin32HandleKHR");
    pfn_vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetMemoryWin32HandlePropertiesKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_memory_win32 */
#ifdef VK_KHR_external_memory_fd
    pfn_vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetInstanceProcAddr(instance, "vkGetMemoryFdKHR");
    pfn_vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetMemoryFdPropertiesKHR");
#endif /* VK_KHR_external_memory_fd */
#ifdef VK_KHR_external_semaphore_capabilities
    pfn_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
#endif /* VK_KHR_external_semaphore_capabilities */
#ifdef VK_KHR_external_semaphore_win32
#ifndef VK_KHR_external_semaphore_win32
    pfn_vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)vkGetInstanceProcAddr(instance, "vkImportSemaphoreWin32HandleKHR");
    pfn_vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)vkGetInstanceProcAddr(instance, "vkGetSemaphoreWin32HandleKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_semaphore_win32 */
#ifdef VK_KHR_external_semaphore_fd
    pfn_vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vkGetInstanceProcAddr(instance, "vkImportSemaphoreFdKHR");
    pfn_vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetInstanceProcAddr(instance, "vkGetSemaphoreFdKHR");
#endif /* VK_KHR_external_semaphore_fd */
#ifdef VK_KHR_push_descriptor
    pfn_vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetInstanceProcAddr(instance, "vkCmdPushDescriptorSetKHR");
#endif /* VK_KHR_push_descriptor */
#ifdef VK_KHR_descriptor_update_template
    pfn_vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetInstanceProcAddr(instance, "vkCreateDescriptorUpdateTemplateKHR");
    pfn_vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetInstanceProcAddr(instance, "vkDestroyDescriptorUpdateTemplateKHR");
    pfn_vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetInstanceProcAddr(instance, "vkUpdateDescriptorSetWithTemplateKHR");
    pfn_vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetInstanceProcAddr(instance, "vkCmdPushDescriptorSetWithTemplateKHR");
#endif /* VK_KHR_descriptor_update_template */
#ifdef VK_KHR_shared_presentable_image
    pfn_vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)vkGetInstanceProcAddr(instance, "vkGetSwapchainStatusKHR");
#endif /* VK_KHR_shared_presentable_image */
#ifdef VK_KHR_external_fence_capabilities
    pfn_vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
#endif /* VK_KHR_external_fence_capabilities */
#ifdef VK_KHR_external_fence_win32
#ifndef VK_KHR_external_fence_win32
    pfn_vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)vkGetInstanceProcAddr(instance, "vkImportFenceWin32HandleKHR");
    pfn_vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)vkGetInstanceProcAddr(instance, "vkGetFenceWin32HandleKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_fence_win32 */
#ifdef VK_KHR_external_fence_fd
    pfn_vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)vkGetInstanceProcAddr(instance, "vkImportFenceFdKHR");
    pfn_vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetInstanceProcAddr(instance, "vkGetFenceFdKHR");
#endif /* VK_KHR_external_fence_fd */
#ifdef VK_KHR_get_surface_capabilities2
    pfn_vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    pfn_vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormats2KHR");
#endif /* VK_KHR_get_surface_capabilities2 */
#ifdef VK_KHR_get_memory_requirements2
    pfn_vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetInstanceProcAddr(instance, "vkGetImageMemoryRequirements2KHR");
    pfn_vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetInstanceProcAddr(instance, "vkGetBufferMemoryRequirements2KHR");
    pfn_vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetInstanceProcAddr(instance, "vkGetImageSparseMemoryRequirements2KHR");
#endif /* VK_KHR_get_memory_requirements2 */
#ifdef VK_KHR_sampler_ycbcr_conversion
    pfn_vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetInstanceProcAddr(instance, "vkCreateSamplerYcbcrConversionKHR");
    pfn_vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetInstanceProcAddr(instance, "vkDestroySamplerYcbcrConversionKHR");
#endif /* VK_KHR_sampler_ycbcr_conversion */
#ifdef VK_KHR_bind_memory2
    pfn_vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)vkGetInstanceProcAddr(instance, "vkBindBufferMemory2KHR");
    pfn_vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)vkGetInstanceProcAddr(instance, "vkBindImageMemory2KHR");
#endif /* VK_KHR_bind_memory2 */
#ifdef VK_ANDROID_native_buffer
    pfn_vkGetSwapchainGrallocUsageANDROID = (PFN_vkGetSwapchainGrallocUsageANDROID)vkGetInstanceProcAddr(instance, "vkGetSwapchainGrallocUsageANDROID");
    pfn_vkAcquireImageANDROID = (PFN_vkAcquireImageANDROID)vkGetInstanceProcAddr(instance, "vkAcquireImageANDROID");
    pfn_vkQueueSignalReleaseImageANDROID = (PFN_vkQueueSignalReleaseImageANDROID)vkGetInstanceProcAddr(instance, "vkQueueSignalReleaseImageANDROID");
#endif /* VK_ANDROID_native_buffer */
#ifdef VK_EXT_debug_report
    pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
    pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
    pfn_vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT");
#endif /* VK_EXT_debug_report */
#ifdef VK_EXT_debug_marker
    pfn_vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetInstanceProcAddr(instance, "vkDebugMarkerSetObjectTagEXT");
    pfn_vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetInstanceProcAddr(instance, "vkDebugMarkerSetObjectNameEXT");
    pfn_vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerBeginEXT");
    pfn_vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerEndEXT");
    pfn_vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerInsertEXT");
#endif /* VK_EXT_debug_marker */
#ifdef VK_AMD_draw_indirect_count
    pfn_vkCmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)vkGetInstanceProcAddr(instance, "vkCmdDrawIndirectCountAMD");
    pfn_vkCmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetInstanceProcAddr(instance, "vkCmdDrawIndexedIndirectCountAMD");
#endif /* VK_AMD_draw_indirect_count */
#ifdef VK_AMD_shader_info
    pfn_vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)vkGetInstanceProcAddr(instance, "vkGetShaderInfoAMD");
#endif /* VK_AMD_shader_info */
#ifdef VK_NV_external_memory_capabilities
    pfn_vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#endif /* VK_NV_external_memory_capabilities */
#ifdef VK_NV_external_memory_win32
#ifndef VK_NV_external_memory_win32
    pfn_vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)vkGetInstanceProcAddr(instance, "vkGetMemoryWin32HandleNV");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_NV_external_memory_win32 */
#ifdef VK_KHX_device_group
    pfn_vkGetDeviceGroupPeerMemoryFeaturesKHX = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHX)vkGetInstanceProcAddr(instance, "vkGetDeviceGroupPeerMemoryFeaturesKHX");
    pfn_vkCmdSetDeviceMaskKHX = (PFN_vkCmdSetDeviceMaskKHX)vkGetInstanceProcAddr(instance, "vkCmdSetDeviceMaskKHX");
    pfn_vkCmdDispatchBaseKHX = (PFN_vkCmdDispatchBaseKHX)vkGetInstanceProcAddr(instance, "vkCmdDispatchBaseKHX");
    pfn_vkGetDeviceGroupPresentCapabilitiesKHX = (PFN_vkGetDeviceGroupPresentCapabilitiesKHX)vkGetInstanceProcAddr(instance, "vkGetDeviceGroupPresentCapabilitiesKHX");
    pfn_vkGetDeviceGroupSurfacePresentModesKHX = (PFN_vkGetDeviceGroupSurfacePresentModesKHX)vkGetInstanceProcAddr(instance, "vkGetDeviceGroupSurfacePresentModesKHX");
    pfn_vkGetPhysicalDevicePresentRectanglesKHX = (PFN_vkGetPhysicalDevicePresentRectanglesKHX)vkGetInstanceProcAddr(instance, "vkGetPhysicalDevicePresentRectanglesKHX");
    pfn_vkAcquireNextImage2KHX = (PFN_vkAcquireNextImage2KHX)vkGetInstanceProcAddr(instance, "vkAcquireNextImage2KHX");
#endif /* VK_KHX_device_group */
#ifdef VK_NN_vi_surface
#ifndef VK_NN_vi_surface
    pfn_vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)vkGetInstanceProcAddr(instance, "vkCreateViSurfaceNN");
#endif /* VK_USE_PLATFORM_VI_NN */
#endif /* VK_NN_vi_surface */
#ifdef VK_KHX_device_group_creation
    pfn_vkEnumeratePhysicalDeviceGroupsKHX = (PFN_vkEnumeratePhysicalDeviceGroupsKHX)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroupsKHX");
#endif /* VK_KHX_device_group_creation */
#ifdef VK_NVX_device_generated_commands
    pfn_vkCmdProcessCommandsNVX = (PFN_vkCmdProcessCommandsNVX)vkGetInstanceProcAddr(instance, "vkCmdProcessCommandsNVX");
    pfn_vkCmdReserveSpaceForCommandsNVX = (PFN_vkCmdReserveSpaceForCommandsNVX)vkGetInstanceProcAddr(instance, "vkCmdReserveSpaceForCommandsNVX");
    pfn_vkCreateIndirectCommandsLayoutNVX = (PFN_vkCreateIndirectCommandsLayoutNVX)vkGetInstanceProcAddr(instance, "vkCreateIndirectCommandsLayoutNVX");
    pfn_vkDestroyIndirectCommandsLayoutNVX = (PFN_vkDestroyIndirectCommandsLayoutNVX)vkGetInstanceProcAddr(instance, "vkDestroyIndirectCommandsLayoutNVX");
    pfn_vkCreateObjectTableNVX = (PFN_vkCreateObjectTableNVX)vkGetInstanceProcAddr(instance, "vkCreateObjectTableNVX");
    pfn_vkDestroyObjectTableNVX = (PFN_vkDestroyObjectTableNVX)vkGetInstanceProcAddr(instance, "vkDestroyObjectTableNVX");
    pfn_vkRegisterObjectsNVX = (PFN_vkRegisterObjectsNVX)vkGetInstanceProcAddr(instance, "vkRegisterObjectsNVX");
    pfn_vkUnregisterObjectsNVX = (PFN_vkUnregisterObjectsNVX)vkGetInstanceProcAddr(instance, "vkUnregisterObjectsNVX");
    pfn_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
#endif /* VK_NVX_device_generated_commands */
#ifdef VK_NV_clip_space_w_scaling
    pfn_vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)vkGetInstanceProcAddr(instance, "vkCmdSetViewportWScalingNV");
#endif /* VK_NV_clip_space_w_scaling */
#ifdef VK_EXT_direct_mode_display
    pfn_vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(instance, "vkReleaseDisplayEXT");
#endif /* VK_EXT_direct_mode_display */
#ifdef VK_EXT_acquire_xlib_display
#ifndef VK_EXT_acquire_xlib_display
    pfn_vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)vkGetInstanceProcAddr(instance, "vkAcquireXlibDisplayEXT");
    pfn_vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)vkGetInstanceProcAddr(instance, "vkGetRandROutputDisplayEXT");
#endif /* VK_USE_PLATFORM_XLIB_XRANDR_EXT */
#endif /* VK_EXT_acquire_xlib_display */
#ifdef VK_EXT_display_surface_counter
    pfn_vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#endif /* VK_EXT_display_surface_counter */
#ifdef VK_EXT_display_control
    pfn_vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)vkGetInstanceProcAddr(instance, "vkDisplayPowerControlEXT");
    pfn_vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)vkGetInstanceProcAddr(instance, "vkRegisterDeviceEventEXT");
    pfn_vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)vkGetInstanceProcAddr(instance, "vkRegisterDisplayEventEXT");
    pfn_vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)vkGetInstanceProcAddr(instance, "vkGetSwapchainCounterEXT");
#endif /* VK_EXT_display_control */
#ifdef VK_GOOGLE_display_timing
    pfn_vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)vkGetInstanceProcAddr(instance, "vkGetRefreshCycleDurationGOOGLE");
    pfn_vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)vkGetInstanceProcAddr(instance, "vkGetPastPresentationTimingGOOGLE");
#endif /* VK_GOOGLE_display_timing */
#ifdef VK_EXT_discard_rectangles
    pfn_vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)vkGetInstanceProcAddr(instance, "vkCmdSetDiscardRectangleEXT");
#endif /* VK_EXT_discard_rectangles */
#ifdef VK_EXT_hdr_metadata
    pfn_vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)vkGetInstanceProcAddr(instance, "vkSetHdrMetadataEXT");
#endif /* VK_EXT_hdr_metadata */
#ifdef VK_MVK_ios_surface
#ifndef VK_MVK_ios_surface
    pfn_vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)vkGetInstanceProcAddr(instance, "vkCreateIOSSurfaceMVK");
#endif /* VK_USE_PLATFORM_IOS_MVK */
#endif /* VK_MVK_ios_surface */
#ifdef VK_MVK_macos_surface
#ifndef VK_MVK_macos_surface
    pfn_vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
#endif /* VK_USE_PLATFORM_MACOS_MVK */
#endif /* VK_MVK_macos_surface */
#ifdef VK_EXT_sample_locations
    pfn_vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)vkGetInstanceProcAddr(instance, "vkCmdSetSampleLocationsEXT");
    pfn_vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
#endif /* VK_EXT_sample_locations */
#ifdef VK_EXT_validation_cache
    pfn_vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)vkGetInstanceProcAddr(instance, "vkCreateValidationCacheEXT");
    pfn_vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)vkGetInstanceProcAddr(instance, "vkDestroyValidationCacheEXT");
    pfn_vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)vkGetInstanceProcAddr(instance, "vkMergeValidationCachesEXT");
    pfn_vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)vkGetInstanceProcAddr(instance, "vkGetValidationCacheDataEXT");
#endif /* VK_EXT_validation_cache */
#ifdef VK_EXT_external_memory_host
    pfn_vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)vkGetInstanceProcAddr(instance, "vkGetMemoryHostPointerPropertiesEXT");
#endif /* VK_EXT_external_memory_host */
#ifdef VK_AMD_buffer_marker
    pfn_vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)vkGetInstanceProcAddr(instance, "vkCmdWriteBufferMarkerAMD");
#endif /* VK_AMD_buffer_marker */
}

void vkExtInitDevice(VkDevice device)
{
#ifdef VK_KHR_surface
    pfn_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)vkGetDeviceProcAddr(device, "vkDestroySurfaceKHR");
    pfn_vkGetPhysicalDeviceSurfaceSupportKHR = (PFN_vkGetPhysicalDeviceSurfaceSupportKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceSupportKHR");
    pfn_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
    pfn_vkGetPhysicalDeviceSurfaceFormatsKHR = (PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceFormatsKHR");
    pfn_vkGetPhysicalDeviceSurfacePresentModesKHR = (PFN_vkGetPhysicalDeviceSurfacePresentModesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfacePresentModesKHR");
#endif /* VK_KHR_surface */
#ifdef VK_KHR_swapchain
    pfn_vkCreateSwapchainKHR = (PFN_vkCreateSwapchainKHR)vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR");
    pfn_vkDestroySwapchainKHR = (PFN_vkDestroySwapchainKHR)vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR");
    pfn_vkGetSwapchainImagesKHR = (PFN_vkGetSwapchainImagesKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR");
    pfn_vkAcquireNextImageKHR = (PFN_vkAcquireNextImageKHR)vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR");
    pfn_vkQueuePresentKHR = (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(device, "vkQueuePresentKHR");
#endif /* VK_KHR_swapchain */
#ifdef VK_KHR_display
    pfn_vkGetPhysicalDeviceDisplayPropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceDisplayPropertiesKHR");
    pfn_vkGetPhysicalDeviceDisplayPlanePropertiesKHR = (PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceDisplayPlanePropertiesKHR");
    pfn_vkGetDisplayPlaneSupportedDisplaysKHR = (PFN_vkGetDisplayPlaneSupportedDisplaysKHR)vkGetDeviceProcAddr(device, "vkGetDisplayPlaneSupportedDisplaysKHR");
    pfn_vkGetDisplayModePropertiesKHR = (PFN_vkGetDisplayModePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetDisplayModePropertiesKHR");
    pfn_vkCreateDisplayModeKHR = (PFN_vkCreateDisplayModeKHR)vkGetDeviceProcAddr(device, "vkCreateDisplayModeKHR");
    pfn_vkGetDisplayPlaneCapabilitiesKHR = (PFN_vkGetDisplayPlaneCapabilitiesKHR)vkGetDeviceProcAddr(device, "vkGetDisplayPlaneCapabilitiesKHR");
    pfn_vkCreateDisplayPlaneSurfaceKHR = (PFN_vkCreateDisplayPlaneSurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateDisplayPlaneSurfaceKHR");
#endif /* VK_KHR_display */
#ifdef VK_KHR_display_swapchain
    pfn_vkCreateSharedSwapchainsKHR = (PFN_vkCreateSharedSwapchainsKHR)vkGetDeviceProcAddr(device, "vkCreateSharedSwapchainsKHR");
#endif /* VK_KHR_display_swapchain */
#ifdef VK_KHR_xlib_surface
#ifndef VK_KHR_xlib_surface
    pfn_vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateXlibSurfaceKHR");
    pfn_vkGetPhysicalDeviceXlibPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXlibPresentationSupportKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceXlibPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XLIB_KHR */
#endif /* VK_KHR_xlib_surface */
#ifdef VK_KHR_xcb_surface
#ifndef VK_KHR_xcb_surface
    pfn_vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateXcbSurfaceKHR");
    pfn_vkGetPhysicalDeviceXcbPresentationSupportKHR = (PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceXcbPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_XCB_KHR */
#endif /* VK_KHR_xcb_surface */
#ifdef VK_KHR_wayland_surface
#ifndef VK_KHR_wayland_surface
    pfn_vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateWaylandSurfaceKHR");
    pfn_vkGetPhysicalDeviceWaylandPresentationSupportKHR = (PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceWaylandPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WAYLAND_KHR */
#endif /* VK_KHR_wayland_surface */
#ifdef VK_KHR_mir_surface
#ifndef VK_KHR_mir_surface
    pfn_vkCreateMirSurfaceKHR = (PFN_vkCreateMirSurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateMirSurfaceKHR");
    pfn_vkGetPhysicalDeviceMirPresentationSupportKHR = (PFN_vkGetPhysicalDeviceMirPresentationSupportKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceMirPresentationSupportKHR");
#endif /* VK_USE_PLATFORM_MIR_KHR */
#endif /* VK_KHR_mir_surface */
#ifdef VK_KHR_android_surface
#ifndef VK_KHR_android_surface
    pfn_vkCreateAndroidSurfaceKHR = (PFN_vkCreateAndroidSurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateAndroidSurfaceKHR");
#endif /* VK_USE_PLATFORM_ANDROID_KHR */
#endif /* VK_KHR_android_surface */
#ifdef VK_KHR_win32_surface
#ifndef VK_KHR_win32_surface
    pfn_vkCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetDeviceProcAddr(device, "vkCreateWin32SurfaceKHR");
    pfn_vkGetPhysicalDeviceWin32PresentationSupportKHR = (PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceWin32PresentationSupportKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_win32_surface */
#ifdef VK_KHR_get_physical_device_properties2
    pfn_vkGetPhysicalDeviceFeatures2KHR = (PFN_vkGetPhysicalDeviceFeatures2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceFeatures2KHR");
    pfn_vkGetPhysicalDeviceProperties2KHR = (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceProperties2KHR");
    pfn_vkGetPhysicalDeviceFormatProperties2KHR = (PFN_vkGetPhysicalDeviceFormatProperties2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceFormatProperties2KHR");
    pfn_vkGetPhysicalDeviceImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceImageFormatProperties2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceImageFormatProperties2KHR");
    pfn_vkGetPhysicalDeviceQueueFamilyProperties2KHR = (PFN_vkGetPhysicalDeviceQueueFamilyProperties2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceQueueFamilyProperties2KHR");
    pfn_vkGetPhysicalDeviceMemoryProperties2KHR = (PFN_vkGetPhysicalDeviceMemoryProperties2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceMemoryProperties2KHR");
    pfn_vkGetPhysicalDeviceSparseImageFormatProperties2KHR = (PFN_vkGetPhysicalDeviceSparseImageFormatProperties2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSparseImageFormatProperties2KHR");
#endif /* VK_KHR_get_physical_device_properties2 */
#ifdef VK_KHR_maintenance1
    pfn_vkTrimCommandPoolKHR = (PFN_vkTrimCommandPoolKHR)vkGetDeviceProcAddr(device, "vkTrimCommandPoolKHR");
#endif /* VK_KHR_maintenance1 */
#ifdef VK_KHR_external_memory_capabilities
    pfn_vkGetPhysicalDeviceExternalBufferPropertiesKHR = (PFN_vkGetPhysicalDeviceExternalBufferPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceExternalBufferPropertiesKHR");
#endif /* VK_KHR_external_memory_capabilities */
#ifdef VK_KHR_external_memory_win32
#ifndef VK_KHR_external_memory_win32
    pfn_vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandleKHR");
    pfn_vkGetMemoryWin32HandlePropertiesKHR = (PFN_vkGetMemoryWin32HandlePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandlePropertiesKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_memory_win32 */
#ifdef VK_KHR_external_memory_fd
    pfn_vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdKHR");
    pfn_vkGetMemoryFdPropertiesKHR = (PFN_vkGetMemoryFdPropertiesKHR)vkGetDeviceProcAddr(device, "vkGetMemoryFdPropertiesKHR");
#endif /* VK_KHR_external_memory_fd */
#ifdef VK_KHR_external_semaphore_capabilities
    pfn_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalSemaphorePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceExternalSemaphorePropertiesKHR");
#endif /* VK_KHR_external_semaphore_capabilities */
#ifdef VK_KHR_external_semaphore_win32
#ifndef VK_KHR_external_semaphore_win32
    pfn_vkImportSemaphoreWin32HandleKHR = (PFN_vkImportSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreWin32HandleKHR");
    pfn_vkGetSemaphoreWin32HandleKHR = (PFN_vkGetSemaphoreWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreWin32HandleKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_semaphore_win32 */
#ifdef VK_KHR_external_semaphore_fd
    pfn_vkImportSemaphoreFdKHR = (PFN_vkImportSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkImportSemaphoreFdKHR");
    pfn_vkGetSemaphoreFdKHR = (PFN_vkGetSemaphoreFdKHR)vkGetDeviceProcAddr(device, "vkGetSemaphoreFdKHR");
#endif /* VK_KHR_external_semaphore_fd */
#ifdef VK_KHR_push_descriptor
    pfn_vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetKHR");
#endif /* VK_KHR_push_descriptor */
#ifdef VK_KHR_descriptor_update_template
    pfn_vkCreateDescriptorUpdateTemplateKHR = (PFN_vkCreateDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(device, "vkCreateDescriptorUpdateTemplateKHR");
    pfn_vkDestroyDescriptorUpdateTemplateKHR = (PFN_vkDestroyDescriptorUpdateTemplateKHR)vkGetDeviceProcAddr(device, "vkDestroyDescriptorUpdateTemplateKHR");
    pfn_vkUpdateDescriptorSetWithTemplateKHR = (PFN_vkUpdateDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(device, "vkUpdateDescriptorSetWithTemplateKHR");
    pfn_vkCmdPushDescriptorSetWithTemplateKHR = (PFN_vkCmdPushDescriptorSetWithTemplateKHR)vkGetDeviceProcAddr(device, "vkCmdPushDescriptorSetWithTemplateKHR");
#endif /* VK_KHR_descriptor_update_template */
#ifdef VK_KHR_shared_presentable_image
    pfn_vkGetSwapchainStatusKHR = (PFN_vkGetSwapchainStatusKHR)vkGetDeviceProcAddr(device, "vkGetSwapchainStatusKHR");
#endif /* VK_KHR_shared_presentable_image */
#ifdef VK_KHR_external_fence_capabilities
    pfn_vkGetPhysicalDeviceExternalFencePropertiesKHR = (PFN_vkGetPhysicalDeviceExternalFencePropertiesKHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceExternalFencePropertiesKHR");
#endif /* VK_KHR_external_fence_capabilities */
#ifdef VK_KHR_external_fence_win32
#ifndef VK_KHR_external_fence_win32
    pfn_vkImportFenceWin32HandleKHR = (PFN_vkImportFenceWin32HandleKHR)vkGetDeviceProcAddr(device, "vkImportFenceWin32HandleKHR");
    pfn_vkGetFenceWin32HandleKHR = (PFN_vkGetFenceWin32HandleKHR)vkGetDeviceProcAddr(device, "vkGetFenceWin32HandleKHR");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_KHR_external_fence_win32 */
#ifdef VK_KHR_external_fence_fd
    pfn_vkImportFenceFdKHR = (PFN_vkImportFenceFdKHR)vkGetDeviceProcAddr(device, "vkImportFenceFdKHR");
    pfn_vkGetFenceFdKHR = (PFN_vkGetFenceFdKHR)vkGetDeviceProcAddr(device, "vkGetFenceFdKHR");
#endif /* VK_KHR_external_fence_fd */
#ifdef VK_KHR_get_surface_capabilities2
    pfn_vkGetPhysicalDeviceSurfaceCapabilities2KHR = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceCapabilities2KHR");
    pfn_vkGetPhysicalDeviceSurfaceFormats2KHR = (PFN_vkGetPhysicalDeviceSurfaceFormats2KHR)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceFormats2KHR");
#endif /* VK_KHR_get_surface_capabilities2 */
#ifdef VK_KHR_get_memory_requirements2
    pfn_vkGetImageMemoryRequirements2KHR = (PFN_vkGetImageMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetImageMemoryRequirements2KHR");
    pfn_vkGetBufferMemoryRequirements2KHR = (PFN_vkGetBufferMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements2KHR");
    pfn_vkGetImageSparseMemoryRequirements2KHR = (PFN_vkGetImageSparseMemoryRequirements2KHR)vkGetDeviceProcAddr(device, "vkGetImageSparseMemoryRequirements2KHR");
#endif /* VK_KHR_get_memory_requirements2 */
#ifdef VK_KHR_sampler_ycbcr_conversion
    pfn_vkCreateSamplerYcbcrConversionKHR = (PFN_vkCreateSamplerYcbcrConversionKHR)vkGetDeviceProcAddr(device, "vkCreateSamplerYcbcrConversionKHR");
    pfn_vkDestroySamplerYcbcrConversionKHR = (PFN_vkDestroySamplerYcbcrConversionKHR)vkGetDeviceProcAddr(device, "vkDestroySamplerYcbcrConversionKHR");
#endif /* VK_KHR_sampler_ycbcr_conversion */
#ifdef VK_KHR_bind_memory2
    pfn_vkBindBufferMemory2KHR = (PFN_vkBindBufferMemory2KHR)vkGetDeviceProcAddr(device, "vkBindBufferMemory2KHR");
    pfn_vkBindImageMemory2KHR = (PFN_vkBindImageMemory2KHR)vkGetDeviceProcAddr(device, "vkBindImageMemory2KHR");
#endif /* VK_KHR_bind_memory2 */
#ifdef VK_ANDROID_native_buffer
    pfn_vkGetSwapchainGrallocUsageANDROID = (PFN_vkGetSwapchainGrallocUsageANDROID)vkGetDeviceProcAddr(device, "vkGetSwapchainGrallocUsageANDROID");
    pfn_vkAcquireImageANDROID = (PFN_vkAcquireImageANDROID)vkGetDeviceProcAddr(device, "vkAcquireImageANDROID");
    pfn_vkQueueSignalReleaseImageANDROID = (PFN_vkQueueSignalReleaseImageANDROID)vkGetDeviceProcAddr(device, "vkQueueSignalReleaseImageANDROID");
#endif /* VK_ANDROID_native_buffer */
#ifdef VK_EXT_debug_report
    pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetDeviceProcAddr(device, "vkCreateDebugReportCallbackEXT");
    pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetDeviceProcAddr(device, "vkDestroyDebugReportCallbackEXT");
    pfn_vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetDeviceProcAddr(device, "vkDebugReportMessageEXT");
#endif /* VK_EXT_debug_report */
#ifdef VK_EXT_debug_marker
    pfn_vkDebugMarkerSetObjectTagEXT = (PFN_vkDebugMarkerSetObjectTagEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectTagEXT");
    pfn_vkDebugMarkerSetObjectNameEXT = (PFN_vkDebugMarkerSetObjectNameEXT)vkGetDeviceProcAddr(device, "vkDebugMarkerSetObjectNameEXT");
    pfn_vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerBeginEXT");
    pfn_vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerEndEXT");
    pfn_vkCmdDebugMarkerInsertEXT = (PFN_vkCmdDebugMarkerInsertEXT)vkGetDeviceProcAddr(device, "vkCmdDebugMarkerInsertEXT");
#endif /* VK_EXT_debug_marker */
#ifdef VK_AMD_draw_indirect_count
    pfn_vkCmdDrawIndirectCountAMD = (PFN_vkCmdDrawIndirectCountAMD)vkGetDeviceProcAddr(device, "vkCmdDrawIndirectCountAMD");
    pfn_vkCmdDrawIndexedIndirectCountAMD = (PFN_vkCmdDrawIndexedIndirectCountAMD)vkGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCountAMD");
#endif /* VK_AMD_draw_indirect_count */
#ifdef VK_AMD_shader_info
    pfn_vkGetShaderInfoAMD = (PFN_vkGetShaderInfoAMD)vkGetDeviceProcAddr(device, "vkGetShaderInfoAMD");
#endif /* VK_AMD_shader_info */
#ifdef VK_NV_external_memory_capabilities
    pfn_vkGetPhysicalDeviceExternalImageFormatPropertiesNV = (PFN_vkGetPhysicalDeviceExternalImageFormatPropertiesNV)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceExternalImageFormatPropertiesNV");
#endif /* VK_NV_external_memory_capabilities */
#ifdef VK_NV_external_memory_win32
#ifndef VK_NV_external_memory_win32
    pfn_vkGetMemoryWin32HandleNV = (PFN_vkGetMemoryWin32HandleNV)vkGetDeviceProcAddr(device, "vkGetMemoryWin32HandleNV");
#endif /* VK_USE_PLATFORM_WIN32_KHR */
#endif /* VK_NV_external_memory_win32 */
#ifdef VK_KHX_device_group
    pfn_vkGetDeviceGroupPeerMemoryFeaturesKHX = (PFN_vkGetDeviceGroupPeerMemoryFeaturesKHX)vkGetDeviceProcAddr(device, "vkGetDeviceGroupPeerMemoryFeaturesKHX");
    pfn_vkCmdSetDeviceMaskKHX = (PFN_vkCmdSetDeviceMaskKHX)vkGetDeviceProcAddr(device, "vkCmdSetDeviceMaskKHX");
    pfn_vkCmdDispatchBaseKHX = (PFN_vkCmdDispatchBaseKHX)vkGetDeviceProcAddr(device, "vkCmdDispatchBaseKHX");
    pfn_vkGetDeviceGroupPresentCapabilitiesKHX = (PFN_vkGetDeviceGroupPresentCapabilitiesKHX)vkGetDeviceProcAddr(device, "vkGetDeviceGroupPresentCapabilitiesKHX");
    pfn_vkGetDeviceGroupSurfacePresentModesKHX = (PFN_vkGetDeviceGroupSurfacePresentModesKHX)vkGetDeviceProcAddr(device, "vkGetDeviceGroupSurfacePresentModesKHX");
    pfn_vkGetPhysicalDevicePresentRectanglesKHX = (PFN_vkGetPhysicalDevicePresentRectanglesKHX)vkGetDeviceProcAddr(device, "vkGetPhysicalDevicePresentRectanglesKHX");
    pfn_vkAcquireNextImage2KHX = (PFN_vkAcquireNextImage2KHX)vkGetDeviceProcAddr(device, "vkAcquireNextImage2KHX");
#endif /* VK_KHX_device_group */
#ifdef VK_NN_vi_surface
#ifndef VK_NN_vi_surface
    pfn_vkCreateViSurfaceNN = (PFN_vkCreateViSurfaceNN)vkGetDeviceProcAddr(device, "vkCreateViSurfaceNN");
#endif /* VK_USE_PLATFORM_VI_NN */
#endif /* VK_NN_vi_surface */
#ifdef VK_KHX_device_group_creation
    pfn_vkEnumeratePhysicalDeviceGroupsKHX = (PFN_vkEnumeratePhysicalDeviceGroupsKHX)vkGetDeviceProcAddr(device, "vkEnumeratePhysicalDeviceGroupsKHX");
#endif /* VK_KHX_device_group_creation */
#ifdef VK_NVX_device_generated_commands
    pfn_vkCmdProcessCommandsNVX = (PFN_vkCmdProcessCommandsNVX)vkGetDeviceProcAddr(device, "vkCmdProcessCommandsNVX");
    pfn_vkCmdReserveSpaceForCommandsNVX = (PFN_vkCmdReserveSpaceForCommandsNVX)vkGetDeviceProcAddr(device, "vkCmdReserveSpaceForCommandsNVX");
    pfn_vkCreateIndirectCommandsLayoutNVX = (PFN_vkCreateIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(device, "vkCreateIndirectCommandsLayoutNVX");
    pfn_vkDestroyIndirectCommandsLayoutNVX = (PFN_vkDestroyIndirectCommandsLayoutNVX)vkGetDeviceProcAddr(device, "vkDestroyIndirectCommandsLayoutNVX");
    pfn_vkCreateObjectTableNVX = (PFN_vkCreateObjectTableNVX)vkGetDeviceProcAddr(device, "vkCreateObjectTableNVX");
    pfn_vkDestroyObjectTableNVX = (PFN_vkDestroyObjectTableNVX)vkGetDeviceProcAddr(device, "vkDestroyObjectTableNVX");
    pfn_vkRegisterObjectsNVX = (PFN_vkRegisterObjectsNVX)vkGetDeviceProcAddr(device, "vkRegisterObjectsNVX");
    pfn_vkUnregisterObjectsNVX = (PFN_vkUnregisterObjectsNVX)vkGetDeviceProcAddr(device, "vkUnregisterObjectsNVX");
    pfn_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX = (PFN_vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceGeneratedCommandsPropertiesNVX");
#endif /* VK_NVX_device_generated_commands */
#ifdef VK_NV_clip_space_w_scaling
    pfn_vkCmdSetViewportWScalingNV = (PFN_vkCmdSetViewportWScalingNV)vkGetDeviceProcAddr(device, "vkCmdSetViewportWScalingNV");
#endif /* VK_NV_clip_space_w_scaling */
#ifdef VK_EXT_direct_mode_display
    pfn_vkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)vkGetDeviceProcAddr(device, "vkReleaseDisplayEXT");
#endif /* VK_EXT_direct_mode_display */
#ifdef VK_EXT_acquire_xlib_display
#ifndef VK_EXT_acquire_xlib_display
    pfn_vkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)vkGetDeviceProcAddr(device, "vkAcquireXlibDisplayEXT");
    pfn_vkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)vkGetDeviceProcAddr(device, "vkGetRandROutputDisplayEXT");
#endif /* VK_USE_PLATFORM_XLIB_XRANDR_EXT */
#endif /* VK_EXT_acquire_xlib_display */
#ifdef VK_EXT_display_surface_counter
    pfn_vkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#endif /* VK_EXT_display_surface_counter */
#ifdef VK_EXT_display_control
    pfn_vkDisplayPowerControlEXT = (PFN_vkDisplayPowerControlEXT)vkGetDeviceProcAddr(device, "vkDisplayPowerControlEXT");
    pfn_vkRegisterDeviceEventEXT = (PFN_vkRegisterDeviceEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDeviceEventEXT");
    pfn_vkRegisterDisplayEventEXT = (PFN_vkRegisterDisplayEventEXT)vkGetDeviceProcAddr(device, "vkRegisterDisplayEventEXT");
    pfn_vkGetSwapchainCounterEXT = (PFN_vkGetSwapchainCounterEXT)vkGetDeviceProcAddr(device, "vkGetSwapchainCounterEXT");
#endif /* VK_EXT_display_control */
#ifdef VK_GOOGLE_display_timing
    pfn_vkGetRefreshCycleDurationGOOGLE = (PFN_vkGetRefreshCycleDurationGOOGLE)vkGetDeviceProcAddr(device, "vkGetRefreshCycleDurationGOOGLE");
    pfn_vkGetPastPresentationTimingGOOGLE = (PFN_vkGetPastPresentationTimingGOOGLE)vkGetDeviceProcAddr(device, "vkGetPastPresentationTimingGOOGLE");
#endif /* VK_GOOGLE_display_timing */
#ifdef VK_EXT_discard_rectangles
    pfn_vkCmdSetDiscardRectangleEXT = (PFN_vkCmdSetDiscardRectangleEXT)vkGetDeviceProcAddr(device, "vkCmdSetDiscardRectangleEXT");
#endif /* VK_EXT_discard_rectangles */
#ifdef VK_EXT_hdr_metadata
    pfn_vkSetHdrMetadataEXT = (PFN_vkSetHdrMetadataEXT)vkGetDeviceProcAddr(device, "vkSetHdrMetadataEXT");
#endif /* VK_EXT_hdr_metadata */
#ifdef VK_MVK_ios_surface
#ifndef VK_MVK_ios_surface
    pfn_vkCreateIOSSurfaceMVK = (PFN_vkCreateIOSSurfaceMVK)vkGetDeviceProcAddr(device, "vkCreateIOSSurfaceMVK");
#endif /* VK_USE_PLATFORM_IOS_MVK */
#endif /* VK_MVK_ios_surface */
#ifdef VK_MVK_macos_surface
#ifndef VK_MVK_macos_surface
    pfn_vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)vkGetDeviceProcAddr(device, "vkCreateMacOSSurfaceMVK");
#endif /* VK_USE_PLATFORM_MACOS_MVK */
#endif /* VK_MVK_macos_surface */
#ifdef VK_EXT_sample_locations
    pfn_vkCmdSetSampleLocationsEXT = (PFN_vkCmdSetSampleLocationsEXT)vkGetDeviceProcAddr(device, "vkCmdSetSampleLocationsEXT");
    pfn_vkGetPhysicalDeviceMultisamplePropertiesEXT = (PFN_vkGetPhysicalDeviceMultisamplePropertiesEXT)vkGetDeviceProcAddr(device, "vkGetPhysicalDeviceMultisamplePropertiesEXT");
#endif /* VK_EXT_sample_locations */
#ifdef VK_EXT_validation_cache
    pfn_vkCreateValidationCacheEXT = (PFN_vkCreateValidationCacheEXT)vkGetDeviceProcAddr(device, "vkCreateValidationCacheEXT");
    pfn_vkDestroyValidationCacheEXT = (PFN_vkDestroyValidationCacheEXT)vkGetDeviceProcAddr(device, "vkDestroyValidationCacheEXT");
    pfn_vkMergeValidationCachesEXT = (PFN_vkMergeValidationCachesEXT)vkGetDeviceProcAddr(device, "vkMergeValidationCachesEXT");
    pfn_vkGetValidationCacheDataEXT = (PFN_vkGetValidationCacheDataEXT)vkGetDeviceProcAddr(device, "vkGetValidationCacheDataEXT");
#endif /* VK_EXT_validation_cache */
#ifdef VK_EXT_external_memory_host
    pfn_vkGetMemoryHostPointerPropertiesEXT = (PFN_vkGetMemoryHostPointerPropertiesEXT)vkGetDeviceProcAddr(device, "vkGetMemoryHostPointerPropertiesEXT");
#endif /* VK_EXT_external_memory_host */
#ifdef VK_AMD_buffer_marker
    pfn_vkCmdWriteBufferMarkerAMD = (PFN_vkCmdWriteBufferMarkerAMD)vkGetDeviceProcAddr(device, "vkCmdWriteBufferMarkerAMD");
#endif /* VK_AMD_buffer_marker */
}

