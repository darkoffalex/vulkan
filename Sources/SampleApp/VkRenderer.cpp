#include "VkRenderer.h"
#include "ext_loader/vulkan_ext.h"

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
 * @param allowIntegratedDevices Позволять использование встроенной графики
 * @return Указатель на созданный объект-обертку над устройством Vulkan
 */
vk::tools::Device VkRenderer::initDevice(const vk::UniqueInstance &instance,
        const vk::UniqueSurfaceKHR &surfaceKhr,
        const std::vector<const char *> &requireExtensions,
        const std::vector<const char *> &requireValidationLayers,
        bool allowIntegratedDevices)
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
    colorAttDesc.format = colorAttachmentFormat;
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
    depthStAttDesc.format = depthStencilAttachmentFormat;
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

/**
 * Инициализация UBO буферов
 * @param device Объект-обертка устройства
 * @param uboViewProjection Указатель на объект UBO буфера матриц вида-проекции
 * @param uboModel  Указатель на объект UBO буфера матриц модели (на каждый меш своя матрица)
 * @param maxMeshes Максимальное кол-во возможных мешей
 */
void VkRenderer::initUboBuffers(const vk::tools::Device &device,
        vk::tools::Buffer *uboViewProjection,
        vk::tools::Buffer *uboModel,
        size_t maxMeshes)
{
    // Создаем UBO буферы для матриц вида и проекции
    *uboViewProjection = vk::tools::Buffer(&device, sizeof(glm::mat4)*2,vk::BufferUsageFlagBits::eUniformBuffer,vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
    if(!uboViewProjection->isReady()) throw vk::InitializationFailedError("Can't create view-projection UBO buffer");

    // Создаем UBO буферы для матриц модели (на каждый меш своя матрица)
    // В связи с тем что тип дескриптора предполагается как "динамический буфер", используем динамическое выравнивание для выяснения размеров буфера
    *uboModel = vk::tools::Buffer(&device, device.getDynamicallyAlignedUboBlockSize<glm::mat4>() * maxMeshes, vk::BufferUsageFlagBits::eUniformBuffer,vk::MemoryPropertyFlagBits::eHostVisible);
    if(!uboModel->isReady()) throw vk::InitializationFailedError("Can't create model UBO buffer");
}

/**
 * Создание дескрипторного пула, из которого будет выделены необходимые наборы
 * @param device Объект-обертка устройства
 * @param type Тип целевого набора, который будет выделятся из пула
 * @param maxSets Максимальное кол-вао выделяемых наборов
 * @return Объект дескрипторного пула (smart-pointer)
 */
vk::UniqueDescriptorPool VkRenderer::createDescriptorPool(
        const vk::tools::Device &device,
        const vk::tools::DescriptorSetType &type,
        size_t maxSets)
{
    // Определяем количество дескрипторов (и их тип) которые смогут храниться в выделенном из данного пула множестве дескрипторов (descriptorSet)
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes;

    switch(type)
    {
        default:
        case vk::tools::DescriptorSetType::eUBO:
        {
            descriptorPoolSizes = {
                    // Один дескриптор в наборе отвечает за обычный UBO буфер (привязывается единожды за кадр)
                    {vk::DescriptorType::eUniformBuffer, 1},
                    // Один дескриптор в наборе отвечает за динамический UBO буфер (может привязываться несколько раз за кадр со смещением в буфере)
                    {vk::DescriptorType::eUniformBufferDynamic, 1},
            };
        }
        case vk::tools::DescriptorSetType::eMeshMaterial:
        {
            descriptorPoolSizes = {
                    // Один дескриптор в наборе отвечает за текстуру совмещенную с текстурным семплером
                    {vk::DescriptorType::eCombinedImageSampler, 1}
            };
        }
    }

    // Создать дескрипторный пул и вернуть его
    vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
    descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
    descriptorPoolCreateInfo.maxSets = 2;
    return device.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
}

/**
 * Создать макет размещения дескрипторного набора
 * @param device Объект-обертка устройства
 * @param type Тип дескрипторного набора
 * @return Объект макета размещения набора дескрипторов (smart-pointer)
 */
vk::UniqueDescriptorSetLayout VkRenderer::createDescriptorSetLayout(const vk::tools::Device &device, const vk::tools::DescriptorSetType &type)
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings;

    switch(type)
    {
        // Если это дескрипторный набор для UBO буфера
        default:
        case vk::tools::DescriptorSetType::eUBO:
        {
            bindings = {
                    // Обычный UBO буфер
                    {
                            0,
                            vk::DescriptorType::eUniformBuffer,
                            1,
                            vk::ShaderStageFlagBits::eVertex,
                            nullptr,
                    },
                    // Динамический UBO буфер
                    // Привязывая набор с этим буфером можно указать динамическое смещение
                    {
                            1,
                            vk::DescriptorType::eUniformBufferDynamic,
                            1,
                            vk::ShaderStageFlagBits::eVertex,
                            nullptr,
                    },
            };
            break;
        }
        // Если это дескрипторный набор материала меша (descriptor set per mesh)
        case vk::tools::DescriptorSetType::eMeshMaterial:
        {
            bindings = {
                    // Обычный UBO буфер
                    {
                            0,
                            vk::DescriptorType::eCombinedImageSampler,
                            1,
                            vk::ShaderStageFlagBits::eFragment,
                            nullptr,
                    },
            };
            break;
        }
    }

    // Создать макет размещения дескрипторного набора
    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
    descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
    descriptorSetLayoutCreateInfo.pBindings = bindings.data();
    return device.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
}

/**
 * Создать дескрипторный набор для UBO буфера
 * Несмотря на то, что у каждого меша может быть свое положение (своя матрица модели) можно использовать общий UBO набор с динамическим UBO дескриптором
 * @param device Объект-обертка устройства
 * @param layout Объект-обертка макета дескрипторного набора
 * @param pool Объект-обертка дескрипторного пула
 * @param uboViewProj UBO буфер вида-проекции
 * @param uboModel UBO для матриц модели мешей
 * @return Объект набора дескрипторов (smart-pointer)
 */
vk::UniqueDescriptorSet VkRenderer::allocateDescriptorSetUBO(
        const vk::tools::Device& device,
        const vk::UniqueDescriptorSetLayout& layout,
        const vk::UniqueDescriptorPool& pool,
        const vk::tools::Buffer& uboViewProj,
        const vk::tools::Buffer& uboModel)
{
    // Выделить из пула набор дескрипторов
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.descriptorPool = pool.get(),
    descriptorSetAllocateInfo.pSetLayouts = &(layout.get());
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    auto allocatedSets = device.getLogicalDevice()->allocateDescriptorSets(descriptorSetAllocateInfo);

    // Информация о буферах
    std::vector<vk::DescriptorBufferInfo> bufferInfos = {
            {uboViewProj.getBuffer().get(),0,VK_WHOLE_SIZE},
            {uboModel.getBuffer().get(),0,VK_WHOLE_SIZE}
    };

    // Связать дескрипторы с буферами (описание "записей")
    std::vector<vk::WriteDescriptorSet> writes = {
            {
                // Целевой набор
                allocatedSets[0],
                // Индекс привязки
                0,
                // Элемент массива (не используется)
                0,
                // Кол-во дескрипторов
                1,
                // Тип дескриптора (обычный UBO буфер)
                vk::DescriptorType::eUniformBuffer,
                // Изображение (не используется)
                nullptr,
                // Буфер (используется)
                bufferInfos.data() + 0,
                // Тексель-буфер (не используется)
                nullptr
            },
            {
                // Целевой набор
                allocatedSets[0],
                // Индекс привязки
                1,
                // Элемент массива (не используется)
                0,
                // Кол-во дескрипторов
                1,
                // Тип дескриптора (динамический UBO буфер со смещением)
                vk::DescriptorType::eUniformBufferDynamic,
                // Изображение (не используется)
                nullptr,
                // Буфер (используется)
                bufferInfos.data() + 1,
                // Тексель-буфер (не используется)
                nullptr
            }
    };

    // Обновление набора дескрипторов
    device.getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

    // Вернуть итоговый набор дескрипторов
    return vk::UniqueDescriptorSet(allocatedSets[0]);
}

/**
 * Создать графический конвейер
 * @param device Объект-обертка устройства
 * @param layout Объект макета размещения конвейера
 * @param frameBufferExtent Размеры (разрешение) кадрового буфера
 * @param renderPass Объект прохода рендеринга
 * @param vertexShaderCodeBytes Код вершинного шейдера (байты)
 * @param fragmentShaderCodeBytes Rод фрагментного шейдера (байты)
 * @return Объект графического конвейера (smart-pointer)
 */
vk::UniquePipeline VkRenderer::createGraphicsPipeline(
        const vk::tools::Device& device,
        const vk::UniquePipelineLayout& layout,
        const vk::Extent3D& frameBufferExtent,
        const vk::UniqueRenderPass& renderPass,
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes)
{

    // Э Т А П  В В О Д А  Д А Н Н Ы Х

    // Описываем первый привязываемый вершинный буфер
    std::vector<vk::VertexInputBindingDescription> vertexInputBindingDescriptions = {
            {
                    0,
                    sizeof(vk::tools::Vertex),
                    vk::VertexInputRate::eVertex
            }
    };

    // Описываем атрибуты вершин
    std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = {
            {
                    0,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    offsetof(vk::tools::Vertex, position)
            },
            {
                    1,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    offsetof(vk::tools::Vertex, color)
            },
            {
                    2,
                    0,
                    vk::Format::eR32G32Sfloat,
                    offsetof(vk::tools::Vertex, uv)
            },
            {
                    3,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    offsetof(vk::tools::Vertex, normal)
            },
    };

    vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = vertexInputBindingDescriptions.size();
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = vertexInputAttributeDescriptions.size();
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

    // Э Т А П  С Б О Р К И  П Р И М И Т И В О В

    vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
    pipelineInputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList; // Ожидаем обычные треугольники
    pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;                  // Без перегрузки примитивов

    // Ш Е Й Д Е Р Ы ( П Р О Г Р А М И Р У Е М Ы Е  С Т А Д И И)

    // Убеждаемся что шейдерный код был предоставлен
    if(vertexShaderCodeBytes.empty() || fragmentShaderCodeBytes.empty()){
        throw vk::InitializationFailedError("No shader code provided");
    }

    // Вершинный шейдер
    vk::ShaderModule shaderModuleVS = device.getLogicalDevice()->createShaderModule({
        {},
        vertexShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(vertexShaderCodeBytes.data())});

    // Фрагментный шейдер
    vk::ShaderModule shaderModuleFS = device.getLogicalDevice()->createShaderModule({
        {},
        fragmentShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(fragmentShaderCodeBytes.data())});

    // Описываем стадии
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,shaderModuleVS,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,shaderModuleFS,"main")
    };


    // V I E W  P O R T  &  S C I S S O R S

    // Настройки области отображения
    vk::Viewport viewport{};
    viewport.setX(0.0f);
    viewport.setY(0.0f);
    viewport.setWidth(frameBufferExtent.width);
    viewport.setHeight(frameBufferExtent.height);
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    // Настройки обрезки
    vk::Rect2D scissors{};
    scissors.offset.x = 0;
    scissors.offset.y = 0;
    scissors.extent.width = frameBufferExtent.width;
    scissors.extent.height = frameBufferExtent.height;

    // Описываем кол-во областей вида и обрезки
    vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
    pipelineViewportStateCreateInfo.viewportCount = 1;
    pipelineViewportStateCreateInfo.pViewports = &viewport;
    pipelineViewportStateCreateInfo.scissorCount = 1;
    pipelineViewportStateCreateInfo.pScissors = &scissors;

    // Р А С Т Е Р И З А Ц И Я

    // Основные параметры растеризации
    vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo{};
    pipelineRasterizationStateCreateInfo.depthClampEnable = false;               // Отбрасывать бесконечно далекие объекты
    pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = false;        // Не отключать этап растеризации
    pipelineRasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;   // Закрашивать полигоны
    pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    pipelineRasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack; // Отсекать задние грани
    pipelineRasterizationStateCreateInfo.frontFace = vk::FrontFace::eClockwise;  // Передние грани описываются по часовой
    pipelineRasterizationStateCreateInfo.depthBiasEnable = false;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    // Параметры теста глубины
    vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
    pipelineDepthStencilStateCreateInfo.depthTestEnable = true;                        // Тест глубины включен
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = true;                       // Запись в тест глубины включена
    pipelineDepthStencilStateCreateInfo.depthCompareOp = vk::CompareOp::eLessOrEqual;  // Функция сравнения (меньше или равно)
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;                 // Тест границ глубины отеключен
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = false;                     // Тест трафарета отключен
    pipelineDepthStencilStateCreateInfo.back.failOp = vk::StencilOp::eKeep;            // В случае провала теста трафарета для задних граней
    pipelineDepthStencilStateCreateInfo.back.passOp = vk::StencilOp::eKeep;            // В случае прохождения теста трафарета для задних граней
    pipelineDepthStencilStateCreateInfo.back.compareOp = vk::CompareOp::eAlways;       // Функция теста трафарета для задних граней
    pipelineDepthStencilStateCreateInfo.front.failOp = vk::StencilOp::eKeep;           // В случае провала теста трафарета для лицевых граней
    pipelineDepthStencilStateCreateInfo.front.passOp = vk::StencilOp::eKeep;           // В случае прохождения теста трафарета для лицевых граней
    pipelineDepthStencilStateCreateInfo.front.compareOp = vk::CompareOp::eAlways;      // Функция теста трафарета для лицевых граней

    // Параметры стадии мульти-семплинга (пока не используем  мульти-семплинг)
    vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo{};
    pipelineMultisampleStateCreateInfo.sampleShadingEnable = false;
    pipelineMultisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
    pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
    pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
    pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = false;
    pipelineMultisampleStateCreateInfo.alphaToOneEnable = false;

    // С М Е Ш И В А Н И Е  Ц В Е Т О В  ( C O L O R  B L E N D I N G )

    // Описываем параметры смешивания для единственного цветового вложения
    vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState{};
    pipelineColorBlendAttachmentState.blendEnable = true;
    pipelineColorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    pipelineColorBlendAttachmentState.colorBlendOp = vk::BlendOp::eAdd;                          // Для цветов - аддитивное смешивание (сложение) по правилам определенным ниже
    pipelineColorBlendAttachmentState.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;          // Множитель исходного цвета равен альфе
    pipelineColorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;  // Множитель второго цвета равен 1 минус альфа исходного
    pipelineColorBlendAttachmentState.alphaBlendOp = vk::BlendOp::eAdd;                          // Для альфа - аддитивное смешивание (сложение) пр правила определенным ниже
    pipelineColorBlendAttachmentState.srcAlphaBlendFactor = vk::BlendFactor::eOne;               // Множитель исходной альфы равен единице
    pipelineColorBlendAttachmentState.dstColorBlendFactor = vk::BlendFactor::eZero;              // Множитель второй альфы равен нулю

    // Описываем общие настройки смешивания
    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
    pipelineColorBlendStateCreateInfo.logicOpEnable = false;
    pipelineColorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

    // К О Н В Е Й Е Р

    // Создать объект конвейера
    vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
    graphicsPipelineCreateInfo.stageCount = shaderStages.size();
    graphicsPipelineCreateInfo.pStages = shaderStages.data();
    graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
    graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
    graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
    graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
    graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
    graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
    graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
    graphicsPipelineCreateInfo.layout = layout.get();
    graphicsPipelineCreateInfo.renderPass = renderPass.get();
    graphicsPipelineCreateInfo.subpass = 0;
    auto pipeline = device.getLogicalDevice()->createGraphicsPipeline(nullptr,graphicsPipelineCreateInfo);

    // Уничтожить шейдерные модули (конвейер создан, они не нужны)
    device.getLogicalDevice()->destroyShaderModule(shaderModuleVS);
    device.getLogicalDevice()->destroyShaderModule(shaderModuleFS);

    // Вернуть unique smart pointer
    return vk::UniquePipeline(pipeline);
}

/** C O N S T R U C T O R - D E S T R U C T O R **/

/**
 * Конструктор
 * @param hInstance Экземпляр WinApi приложения
 * @param hWnd Дескриптор окна WinApi
 * @param maxMeshes Максимальное кол-во мешей
 */
VkRenderer::VkRenderer(HINSTANCE hInstance,
        HWND hWnd,
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes,
        size_t maxMeshes):
debugReportCallback_(VK_NULL_HANDLE),
isEnabled_(true),
isCommandsReady_(false)
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

    // Выделение UBO буферов
    initUboBuffers(device_,&uboBufferViewProjection_,&uboBufferModel_,maxMeshes);
    std::cout << "UBO-buffers allocated (" << uboBufferViewProjection_.getSize() << " and " << uboBufferModel_.getSize() << ")." << std::endl;

    // Создать текстурный семплер по умолчанию
    textureSamplerDefault_ = vk::tools::createImageSampler(device_.getLogicalDevice().get(), vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, 4);
    std::cout << "Default texture sampler created." << std::endl;

    // Создание пулов дескрипторов
    // Поскольку в UBO наборе используется динамический UBO дескриптор для матриц модели, можно обойтись и одним UBO набором
    // В случае с набором материала, в нем используются дескрипторы текстур без динамического смещения, посему на каждый меш по набору
    descriptorPoolUBO_ = createDescriptorPool(device_,vk::tools::DescriptorSetType::eUBO, 1);
    descriptorPoolMeshMaterial_ = createDescriptorPool(device_,vk::tools::DescriptorSetType::eMeshMaterial, maxMeshes);
    std::cout << "Descriptor pools created." << std::endl;

    // Создаем размещение набором дескрипторов
    // Макет размещения описывает какие дескрипторы и сколько их, будет задействовано в конкретном наборе
    descriptorSetLayoutUBO_ = createDescriptorSetLayout(device_,vk::tools::DescriptorSetType::eUBO);
    descriptorSetLayoutMeshMaterial_ = createDescriptorSetLayout(device_, vk::tools::DescriptorSetType::eMeshMaterial);
    std::cout << "Descriptor set layouts created." << std::endl;

    // Создать UBO дескрипторный набор, и связать дескрипторы с ресурсами (буферами для матриц вида-проекции и модели)
    descriptorSetUBO_ = allocateDescriptorSetUBO(device_, descriptorSetLayoutUBO_, descriptorPoolUBO_, uboBufferViewProjection_, uboBufferModel_);
    std::cout << "UBO descriptor set allocated." << std::endl;

    // Создать макет размещения графического конвейера
    std::vector<vk::DescriptorSetLayout> layouts = {descriptorSetLayoutUBO_.get(),descriptorSetLayoutMeshMaterial_.get()};
    pipelineLayout_ = device_.getLogicalDevice()->createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo({},layouts.size(),layouts.data()));
    std::cout << "Pipeline layout created." << std::endl;

    // Создать проход рендеринга
    pipeline_ = createGraphicsPipeline(device_,pipelineLayout_,frameBuffers_[0].getExtent(),mainRenderPass_,vertexShaderCodeBytes,fragmentShaderCodeBytes);
    std::cout << "Graphics pipeline created." << std::endl;

    // Создать примитивы синхронизации (семафоры)
    semaphoreReadyToPresent_ = device_.getLogicalDevice()->createSemaphoreUnique({});
    semaphoreReadyToRender_ = device_.getLogicalDevice()->createSemaphoreUnique({});
    std::cout << "Synchronization semaphores created." << std::endl;
}

/**
 * Деструктор
 */
VkRenderer::~VkRenderer()
{
    // Остановка рендеринга
    this->stopRendering();

    // Удалить примитивы синхронизации
    device_.getLogicalDevice()->destroySemaphore(semaphoreReadyToRender_.get());
    device_.getLogicalDevice()->destroySemaphore(semaphoreReadyToPresent_.get());
    semaphoreReadyToRender_.release();
    semaphoreReadyToPresent_.release();
    std::cout << "Synchronization semaphores destroyed." << std::endl;

    // Уничтожение прохода рендеринга
    device_.getLogicalDevice()->destroyPipeline(pipeline_.get());
    pipeline_.release();
    std::cout << "Graphics pipeline destroyed." << std::endl;

    // Уничтожение макета размещения графического конвейера
    device_.getLogicalDevice()->destroyPipelineLayout(pipelineLayout_.get());
    pipelineLayout_.release();
    std::cout << "Pipeline layout destroyed." << std::endl;

    // Освобождение UBO набора дескрипторов
    device_.getLogicalDevice()->freeDescriptorSets(descriptorPoolUBO_.get(),1,&(descriptorSetUBO_.get()));
    descriptorSetUBO_.release();
    std::cout << "UBO descriptor set freed." << std::endl;

    // Уничтожение макета размещения наборов дескрипторов
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutUBO_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutMeshMaterial_.get());
    descriptorSetLayoutUBO_.release();
    descriptorSetLayoutMeshMaterial_.release();
    std::cout << "Descriptor set layouts destroyed." << std::endl;

    // Уничтожение пулов дескрипторов
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolUBO_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolMeshMaterial_.get());
    descriptorPoolUBO_.release();
    descriptorPoolMeshMaterial_.release();
    std::cout << "Descriptor pools destroyed." << std::endl;

    // Уничтожение текстурного семплера по умолчанию
    device_.getLogicalDevice()->destroySampler(textureSamplerDefault_.get());
    textureSamplerDefault_.release();
    std::cout << "Default texture sampler destroyed." << std::endl;

    // Освобождение UBO буферов
    uboBufferViewProjection_.destroyVulkanResources();
    uboBufferModel_.destroyVulkanResources();
    std::cout << "UBO-buffers freed." << std::endl;

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

/**
 * Остановка рендеринга
 */
void VkRenderer::stopRendering()
{
    // Ожидаем завершения всех команд
    device_.getLogicalDevice()->waitIdle();
    // Отключаем рендеринг
    isEnabled_ = false;
}

/**
 * Рендеринг кадра
 */
void VkRenderer::draw()
{
    // Если рендеринг не включен - выход
    if(!isEnabled_){
        return;
    }

    // П О Д Г О Т О В К А  К О М А Н Д

    // Если командные буферы не готовы - заполнить их командами
    if(!isCommandsReady_)
    {
        // Описываем очистку вложений
        std::vector<vk::ClearValue> clearValues(2);
        clearValues[0].color = vk::ClearColorValue( std::array<float, 4>({ 1.0f, 0.0f, 0.0f, 1.0f }));
        clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

        // Описываем начало прохода
        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderPass = mainRenderPass_.get();
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = frameBuffers_[0].getExtent().width;
        renderPassBeginInfo.renderArea.extent.height = frameBuffers_[0].getExtent().height;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        // Подготовка команд (запись в командные буферы)
        for(size_t i = 0; i < commandBuffers_.size(); ++i)
        {
            // Начинаем работу с командным буфером (запись команд)
            vk::CommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
            commandBufferBeginInfo.pNext = nullptr;
            commandBuffers_[i].begin(commandBufferBeginInfo);

            // Сменить целевой кадровый буфер и начать работу с проходом (это очистит вложения)
            renderPassBeginInfo.framebuffer = frameBuffers_[i].getVulkanFrameBuffer().get();
            commandBuffers_[i].beginRenderPass(renderPassBeginInfo,vk::SubpassContents::eInline);

            // Привязать графический конвейер
            commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eGraphics,pipeline_.get());

            //TODO: привязка наборов дескрипторов и вершинных буферов

            // Завершение прохода добавит неявное преобразование памяти кадрового буфера в VK_IMAGE_LAYOUT_PRESENT_SRC_KHR для представления содержимого
            commandBuffers_[i].endRenderPass();

            // Завершаем работу с командным буфером
            commandBuffers_[i].end();
        }

        // Командные буфер готовы
        isCommandsReady_ = true;
    }

    // О Т П Р А В К А  К О М А Н Д  И  П О К А З

    // Индекс доступного изображения
    size_t availableImageIndex = 0;

    // Получить индекс доступного для рендеринга изображения и взвести семафор готовности к рендерингу
    device_.getLogicalDevice()->acquireNextImageKHR(
            swapChainKhr_.get(),
            10000,
            semaphoreReadyToRender_.get(),
            {},
            &availableImageIndex);

    // Семафоры, которые будут ожидаться конвейером
    std::vector<vk::Semaphore> waitSemaphores = {semaphoreReadyToRender_.get()};

    // Семафоры, которые будут взводиться конвейером после прохождения конвейера
    std::vector<vk::Semaphore> signalSemaphores = {semaphoreReadyToPresent_.get()};

    // Стадии, на которых конвейер будет приостанавливаться, чтобы ожидать своего семафора
    std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

    // Отправить команды на выполнение
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;                                       // Кол-во командных буферов
    submitInfo.pCommandBuffers = &(commandBuffers_[availableImageIndex]);    // Командные буферы
    submitInfo.waitSemaphoreCount = waitSemaphores.size();                   // Кол-во семафоров, которые будет ожидать конвейер
    submitInfo.pWaitSemaphores = waitSemaphores.data();                      // Семафоры ожидания
    submitInfo.pWaitDstStageMask = waitStages.data();                        // Этапы конвейера, на которых будет ожидание
    submitInfo.signalSemaphoreCount = signalSemaphores.size();               // Кол-во семафоров, которые будут взведены после выполнения
    submitInfo.pSignalSemaphores = signalSemaphores.data();                  // Семафоры взведения
    device_.getGraphicsQueue().submit({submitInfo}, nullptr); // Отправка командного буфера на выполнение

    // Инициировать показ (когда картинка будет готова)
    vk::PresentInfoKHR presentInfoKhr{};
    presentInfoKhr.waitSemaphoreCount = signalSemaphores.size();              // Кол-во семафоров, которые будут ожидаться
    presentInfoKhr.pWaitSemaphores = signalSemaphores.data();                 // Семафоры, которые ожидаются
    presentInfoKhr.swapchainCount = 1;                                        // Кол-во цепочек показа
    presentInfoKhr.pSwapchains = &(swapChainKhr_.get());                      // Цепочка показа
    presentInfoKhr.pImageIndices = &availableImageIndex;                      // Индекс показываемого изображения
    device_.getPresentQueue().presentKHR(presentInfoKhr);                     // Осуществить показ
}
