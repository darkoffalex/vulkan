#include "VkRenderer.h"
#include "ext_loader/vulkan_ext.h"

#include <iostream>

/**
 * Инициализация экземпляра
 * @param appName Наименования приложения
 * @param engineName Наименование движка
 * @param requireExtensions Запрашивать расширения (названия расширений)
 * @param requireValidationLayers Запрашивать слои (названия слоев)
 */
vk::UniqueInstance VkRenderer::initInstance(const std::string &appName,
        const std::string &engineName,
        const std::vector<const char *> &requireExtensions,
        const std::vector<const char *> &requireValidationLayers)
{
    // Структуры с информацией о приложении
    vk::ApplicationInfo applicationInfo(
            appName.c_str(),
            1,
            engineName.c_str(),
            1,
            VK_API_VERSION_1_2);

    // Структура для инициализации экземпляра Vulkan
    vk::InstanceCreateInfo instanceCreateInfo({},&applicationInfo);

    // Если были запрошены расширения
    if(!requireExtensions.empty()){
        if(vk::tools::CheckInstanceExtensionsSupported(requireExtensions)){
            instanceCreateInfo.setPpEnabledExtensionNames(requireExtensions.data());
            instanceCreateInfo.setEnabledExtensionCount(requireExtensions.size());
        }else{
            throw vk::ExtensionNotPresentError("Some of required extensions is unavailable");
        }
    }

    // Если были запрошены слои
    if(!requireValidationLayers.empty()){
        if(vk::tools::CheckInstanceLayersSupported(requireValidationLayers)){
            instanceCreateInfo.setPpEnabledLayerNames(requireValidationLayers.data());
            instanceCreateInfo.setEnabledLayerCount(requireValidationLayers.size());
        }else{
            throw vk::LayerNotPresentError("Some of required layers is unavailable");
        }
    }

    return vk::createInstanceUnique(instanceCreateInfo);
}

/**
 * Создание объекта для работы с debug-callback'ом
 * @param vulkanInstance Экземпляр Vulkan
 * @return Идентификатор объекта для работы с debug-callback'ом
 */
VkDebugReportCallbackEXT VkRenderer::createDebugReportCallback(const vk::UniqueInstance& vulkanInstance)
{
    // Конфигурация callback'а
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoExt{};
    vkDebugReportCallbackCreateInfoExt.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoExt.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoExt.pfnCallback = vk::tools::DebugVulkanCallback;

    // Создать объект и вернуть идентификатор
    return vulkanInstance->createDebugReportCallbackEXT(vkDebugReportCallbackCreateInfoExt);
}

/**
 * Создание поверхности отображения на окне
 * @param vulkanInstance Экземпляр Vulkan
 * @param hInstance экземпляр WinApi приложения
 * @param hWnd дескриптор окна WinApi
 * @return Поверхность для рисования на окне (smart pointer)
 */
vk::UniqueSurfaceKHR VkRenderer::createSurface(const vk::UniqueInstance& vulkanInstance, HINSTANCE hInstance, HWND hWnd)
{
    // Конфигурация поверхности
    VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKhr{};
    win32SurfaceCreateInfoKhr.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    win32SurfaceCreateInfoKhr.hwnd = hWnd;
    win32SurfaceCreateInfoKhr.hinstance = hInstance;
    win32SurfaceCreateInfoKhr.flags = 0;
    win32SurfaceCreateInfoKhr.pNext = nullptr;

    // Создать объект и вернуть smart pointer на него
    return vulkanInstance->createWin32SurfaceKHRUnique(win32SurfaceCreateInfoKhr);
}

/**
 * Инициализация устройства
 * @param instance Экземпляр Vulkan
 * @param surfaceKhr Поверхность отображения
 * @param requireExtensions Запрашивать расширения (названия расширений)
 * @param requireValidationLayers Запрашивать слои (названия слоев)
 * @return Указатель на созданный объект-обертку над устройством Vulkan
 */
vk::tools::Device VkRenderer::initDevice(const vk::UniqueInstance &instance,
        const vk::UniqueSurfaceKHR &surfaceKhr,
        const std::vector<const char *> &requireExtensions,
        const std::vector<const char *> &requireValidationLayers)
{
    vk::tools::Device device(
            instance,
            surfaceKhr,
            requireExtensions,
            requireValidationLayers);

    if(!device.isReady()){
        throw vk::InitializationFailedError("Can't initialize device");
    }

    return device;
}

/**
 * Создание прохода рендерера
 * @param surfaceKhr Поверхность отображения (для проверки поддержки форматов поверхностью)
 * @param colorAttachmentFormat Формат цветовых вложений
 * @param depthStencilAttachmentFormat Формат вложений глубины трафарета
 * @details Цветовые вложения - по сути изображения в которые шейдеры пишут информацию. У них могут быть форматы.
 * Для vulkan важно настроить доступность и формат вложений на этапе инициализации прохода
 * @return Проход рендеринга (smart pointer)
 */
vk::UniqueRenderPass VkRenderer::createRenderPass(const vk::tools::Device& device,
                                                  const vk::UniqueSurfaceKHR &surfaceKhr,
                                                  const vk::Format &colorAttachmentFormat,
                                                  const vk::Format &depthStencilAttachmentFormat)
{
    // Проверка поддержки форматов
    if(!device.isFormatSupported(colorAttachmentFormat,surfaceKhr)){
        throw vk::FormatNotSupportedError("Color attachment format not supported");
    }

    if(!device.isDepthStencilSupportedForFormat(depthStencilAttachmentFormat)){
        throw vk::FormatNotSupportedError("Depth-stencil attachment format not supported");
    }

    // Массив описаний вложений
    // Пока-что предполагается использование двух вложений
    // Вложения - изображения в которые происходит запись из шейдеров (цвет, глубина, трафарет).
    // Также вложение может быть передано в следующий под-проход для дальнейшего использования
    std::vector<vk::AttachmentDescription> attachmentDescriptions;

    // Описываем цветовое вложение
    vk::AttachmentDescription colorAttDesc{};
    colorAttDesc.setFormat(colorAttachmentFormat);
    colorAttDesc.samples = vk::SampleCountFlagBits::e1;                  // Один семпл на пиксель (без мульти-семплинга)
    colorAttDesc.loadOp = vk::AttachmentLoadOp::eClear;                  // Начало под-прохода - очищать вложение
    colorAttDesc.storeOp = vk::AttachmentStoreOp::eStore;                // Конец под-прохода - хранить для показа
    colorAttDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;        // Трафарет. Начало под-прохода - не важно
    colorAttDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;      // Трафарет. Конец под-прохода - не важно
    colorAttDesc.initialLayout = vk::ImageLayout::eUndefined;            // Макет памяти изображения в начале - не важно
    colorAttDesc.finalLayout = vk::ImageLayout::ePresentSrcKHR;          // Макет памяти изображения в конце - показ
    attachmentDescriptions.push_back(colorAttDesc);

    // Описываем вложение глубины-трафарета (Z-buffer + stencil)
    vk::AttachmentDescription depthStAttDesc{};
    depthStAttDesc.setFormat(colorAttachmentFormat);
    depthStAttDesc.samples = vk::SampleCountFlagBits::e1;                   // Один семпл на пиксель (без мульти-семплинга)
    depthStAttDesc.loadOp = vk::AttachmentLoadOp::eClear;                   // Цвет. Начало под-прохода - очищать вложение
    depthStAttDesc.storeOp = vk::AttachmentStoreOp::eDontCare;              // Цвет. Конец под-прохода - не важно (показывать не нужно)
    depthStAttDesc.stencilLoadOp = vk::AttachmentLoadOp::eClear;            // Трафарет. Начало под-прохода - очищать
    depthStAttDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;       // Трафарет. Конец под-прохода - не важно
    depthStAttDesc.initialLayout = vk::ImageLayout::eUndefined;             // Макет памяти изображения в начале - не важно
    depthStAttDesc.finalLayout = vk::ImageLayout::eDepthAttachmentOptimal;  // Макет памяти изображения в конце - буфер глубины-трафарета
    attachmentDescriptions.push_back(depthStAttDesc);

    // Ссылки на вложения
    // Если считать что вложения находятся в массиве, то ссылка содержит индекс соответствующего вложения
    // Также ссылка определяет макет памяти вложения, который используется во время под-прохода
    vk::AttachmentReference colorAttachmentReferences[1] = {{0,vk::ImageLayout::eColorAttachmentOptimal}};
    vk::AttachmentReference depthAttachmentReference{1,vk::ImageLayout::eDepthAttachmentOptimal};

    // Описываем под-проход
    // Проход должен содержать в себе как минимум один под-проход
    vk::SubpassDescription subPassDescription{};
    subPassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;   // Тип конвейера - графический
    subPassDescription.colorAttachmentCount = 1;                               // Кол-во цветовых вложений
    subPassDescription.pColorAttachments = colorAttachmentReferences;          // Цветовые вложения
    subPassDescription.pDepthStencilAttachment = &depthAttachmentReference;    // Вложение глубины-трафарета
    subPassDescription.inputAttachmentCount = 0;                               // Кол-во входных вложений (например, с предыдущих под-проходов)
    subPassDescription.pInputAttachments = nullptr;                            // Нет входных вложений на этом под-проходе
    subPassDescription.preserveAttachmentCount = 0;                            // Кол-во хранимых вложений (не используются в этом под-проходе, но хранятся для следующих)
    subPassDescription.pPreserveAttachments = nullptr;                         // Нет хранимых вложений на этом под-проходе
    subPassDescription.pResolveAttachments = nullptr;                          // Вложения для мульти-семплированной картинки (кол-во совпадает с кол-вом цветовых)

    // Описываем зависимости (порядок) под-проходов
    // Хоть в проходе использован только один под-проход, существует также еще и неявный (внешний) под-проход
    std::vector<vk::SubpassDependency> subPassDependencies;

    // Переход от внешнего (неявного) под-прохода к первому (нулевому)
    vk::SubpassDependency externalToFirst;
    externalToFirst.srcSubpass = VK_SUBPASS_EXTERNAL;
    externalToFirst.dstSubpass = 0;
    externalToFirst.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    externalToFirst.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    externalToFirst.srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
    externalToFirst.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    externalToFirst.dependencyFlags = vk::DependencyFlagBits::eByRegion;
    subPassDependencies.push_back(externalToFirst);

    // Переход от первого (нулевого) ко внешнему (неявному) под-проходу
    vk::SubpassDependency firstToExternal;
    firstToExternal.srcSubpass = 0;
    firstToExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
    firstToExternal.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    firstToExternal.dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    firstToExternal.srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    firstToExternal.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;
    firstToExternal.dependencyFlags = vk::DependencyFlagBits::eByRegion;
    subPassDependencies.push_back(firstToExternal);

    // Описание прохода
    vk::RenderPassCreateInfo renderPassCreateInfo{};
    renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
    renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subPassDescription;
    renderPassCreateInfo.dependencyCount = subPassDependencies.size();
    renderPassCreateInfo.pDependencies = subPassDependencies.data();

    // Создать проход и вернуть smart-pointer
    return device.getLogicalDevice()->createRenderPassUnique(renderPassCreateInfo);

}

/**
 * Создать объект цепочки показа
 * @param device Указатель на объект-обертку устройства
 * @param surfaceFormat Формат поверхности
 * @param surfaceKhr Поверхность отображения (для проверки поддержки форматов поверхностью)
 * @param bufferCount Кол-во буферов
 * @return Объект swap-chain (smart pointer)
 */
vk::UniqueSwapchainKHR VkRenderer::createSwapChain(
        const vk::tools::Device& device,
        const vk::SurfaceFormatKHR& surfaceFormat,
        const vk::UniqueSurfaceKHR& surfaceKhr,
        size_t bufferCount)
{
    // Проверить поддерживается ли формат поверхности
    if(!device.isSurfaceFormatSupported(surfaceFormat,surfaceKhr)){
        throw vk::FormatNotSupportedError("Surface format not supported");
    }

    // Получить возможности устройства для поверхности
    auto capabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(surfaceKhr.get());

    // Если задано кол-во буферов
    if(bufferCount > 0){
        if(bufferCount < capabilities.minImageCount || bufferCount > capabilities.maxImageCount){
            throw vk::InitializationFailedError("Unsupported buffer count required. Please change it");
        }
    }
    // Если нет - определить оптимальное кол-во
    else{
        bufferCount = (capabilities.minImageCount + 1) > capabilities.maxImageCount ? capabilities.maxImageCount : (capabilities.minImageCount + 1);
    }

    // Режим представления
    // Отвечает за синхронизацию с оконной (если показ на окне) системой и скоростью отображения новых кадров на поверхности
    // Режим FIFO один из простых
    vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;

    // Если кол-во буферов больше единицы, есть смысл выбрать более сложный режим (если его поддерживает поверхность)
    if(bufferCount > 1){
        auto presentModes = device.getPhysicalDevice().getSurfacePresentModesKHR(surfaceKhr.get());
        for(const auto& presentModeEntry : presentModes){
            if(presentModeEntry == vk::PresentModeKHR::eMailbox){
                presentMode = presentModeEntry;
                break;
            }
        }
    }

    // Создать swap-chain
    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.surface = surfaceKhr.get();
    swapChainCreateInfo.minImageCount = bufferCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = capabilities.currentExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = device.isPresentAndGfxQueueFamilySame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    swapChainCreateInfo.queueFamilyIndexCount = swapChainCreateInfo.imageSharingMode == vk::SharingMode::eConcurrent ? device.getQueueFamilyIndices().size() : 0;
    swapChainCreateInfo.pQueueFamilyIndices = swapChainCreateInfo.imageSharingMode == vk::SharingMode::eConcurrent ? device.getQueueFamilyIndices().data() : nullptr;
    swapChainCreateInfo.preTransform = capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = nullptr; //TODO: добавить параметр, сделать пересоздание swap-chain'а
    return device.getLogicalDevice()->createSwapchainKHRUnique(swapChainCreateInfo);
}

/**
 * Инициализировать кадровые буферы
 * @param frameBuffers Указатель на массив кадровых буферов
 * @param device Объект-обертка устройства
 * @param surfaceKhr Поверхность отображения (для получения разрешения и прочего)
 * @param swapChain Объект цепочки показа (swap-chain)
 * @param renderPass Объект прохода рендеринга (каждый кадровый буфер связан с проходом)
 * @param colorAttachmentFormat Формат цветовых вложений (соответствует тому, что был использован при создании прохода)
 * @param depthStencilAttachmentFormat Формат вложений глубины трафарета (соответствует тому, что был использован при создании прохода)
 */
void VkRenderer::initFrameBuffers(std::vector<vk::tools::FrameBuffer> *frameBuffers, const vk::tools::Device &device,
        const vk::UniqueSurfaceKHR &surfaceKhr, const vk::UniqueSwapchainKHR &swapChain,
        const vk::UniqueRenderPass &renderPass, const vk::Format &colorAttachmentFormat,
        const vk::Format &depthStencilAttachmentFormat)
{
    // Получить изображения
    auto swapChainImages = device.getLogicalDevice()->getSwapchainImagesKHR(swapChain.get());

    // Получить возможности устройства для поверхности
    auto capabilities = device.getPhysicalDevice().getSurfaceCapabilitiesKHR(surfaceKhr.get());

    // Пройтись по всем изображениям и создать кадровый буфер для каждого
    for(const auto& swapChainImage : swapChainImages)
    {
        frameBuffers->emplace_back(vk::tools::FrameBuffer(
                &device,
                swapChainImage,
                colorAttachmentFormat,
                depthStencilAttachmentFormat,
                {capabilities.currentExtent.width,capabilities.currentExtent.height,1},
                renderPass));
    }
}

/** C O N S T R U C T O R - D E S T R U C T O R **/

/**
 * Конструктор
 * @param hInstance экземпляр WinApi приложения
 * @param hWnd дескриптор окна WinApi
 */
VkRenderer::VkRenderer(HINSTANCE hInstance, HWND hWnd):
debugReportCallback_(VK_NULL_HANDLE)
{
    // Инициализация экземпляра Vulkan
    this->vulkanInstance_ = initInstance("My Application",
            "My engine",
            { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME},
            { "VK_LAYER_LUNARG_standard_validation"});
    std::cout << "Vulkan instance created." << std::endl;

    // Загрузка функций расширений (получение адресов)
    vkExtInitInstance(static_cast<VkInstance>(vulkanInstance_.get()));

    // Создание debug report callback'а (создается только в том случае, если расширение VK_EXT_DEBUG_REPORT_EXTENSION_NAME использовано)
    this->debugReportCallback_ = createDebugReportCallback(vulkanInstance_);
    std::cout << "Report callback object created." << std::endl;

    // Создание поверхности отображения на окне
    this->surface_ = createSurface(this->vulkanInstance_,hInstance,hWnd);
    std::cout << "Surface created." << std::endl;

    // Создание устройства
    device_ = initDevice(this->vulkanInstance_,this->surface_,{VK_KHR_SWAPCHAIN_EXTENSION_NAME},{"VK_LAYER_LUNARG_standard_validation"});
    std::cout << "Device initialized (" << device_.getPhysicalDevice().getProperties().deviceName << ")" << std::endl;

    // Создание прохода
    mainRenderPass_ = createRenderPass(device_,this->surface_,vk::Format::eB8G8R8A8Unorm, vk::Format::eD32SfloatS8Uint);
    std::cout << "Render pass created." << std::endl;

    // Создание цепочки показа (swap-chain)
    swapChainKhr_ = createSwapChain(device_,{vk::Format::eB8G8R8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear},this->surface_);
    std::cout << "Swap-chain created." << std::endl;

    // Создание кадровых буферов
    initFrameBuffers(&frameBuffers_,device_,surface_,swapChainKhr_,mainRenderPass_,vk::Format::eB8G8R8A8Unorm,vk::Format::eD32SfloatS8Uint);
    std::cout << "Frame-buffers initialized (" << frameBuffers_.size() << ")." << std::endl;

    // Выделение кадровых буферов
    // Командный буфер может быть и один, но в таком случае придется ожидать его выполнения перед тем, как начинать запись
    // в очередное изображение swap-chain'а (что не есть оптимально). Поэтому лучше использовать отдельные буферы, для каждого
    // изображения swap-chain'а (по сути это копии одного и того же буфера)
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(),vk::CommandBufferLevel::ePrimary,frameBuffers_.size());
    commandBuffers_ = device_.getLogicalDevice()->allocateCommandBuffers(allocInfo);
    std::cout << "Command-buffers allocated (" << commandBuffers_.size() << ")." << std::endl;
}

/**
 * Деструктор
 */
VkRenderer::~VkRenderer()
{
    // Освобождение командных буферов
    device_.getLogicalDevice()->freeCommandBuffers(device_.getCommandGfxPool().get(),commandBuffers_);
    commandBuffers_.clear();
    std::cout << "Command-buffers freed." << std::endl;

    // Уничтожение кадровых буферов
    frameBuffers_.clear();
    std::cout << "Frame-buffers destroyed." << std::endl;

    // Уничтожение цепочки показа (swap-chain)
    device_.getLogicalDevice()->destroySwapchainKHR(swapChainKhr_.get());
    swapChainKhr_.release();
    std::cout << "Swap-chain destroyed." << std::endl;

    // Уничтожение прохода
    device_.getLogicalDevice()->destroyRenderPass(mainRenderPass_.get());
    mainRenderPass_.release();
    std::cout << "Render pass destroyed." << std::endl;

    // Уничтожение устройства
    device_.destroyVulkanResources();
    std::cout << "Device destroyed." << std::endl;

    // Уничтожение поверхности
    vulkanInstance_->destroySurfaceKHR(surface_.get());
    surface_.release();
    std::cout << "Surface destroyed." << std::endl;

    // Уничтожение debug report callback'а
    vulkanInstance_->destroyDebugReportCallbackEXT(debugReportCallback_);
    debugReportCallback_ = VK_NULL_HANDLE;
    std::cout << "Report callback object destroyed." << std::endl;

    // Уничтожение (явное) экземпляра Vulkan
    vulkanInstance_->destroy();
    vulkanInstance_.release();
    std::cout << "Vulkan instance destroyed." << std::endl;
}
