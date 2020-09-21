#include "VkRenderer.h"
#include "VkExtensionLoader/ExtensionLoader.h"

/**
 * Инициализация проходов рендеринга
 * @param colorAttachmentFormat Формат цветовых вложений
 * @param depthStencilAttachmentFormat Формат буфера глубины
 * @details Цветовые вложения - по сути изображения в которые шейдеры пишут информацию. У них могут быть форматы.
 * При создании проходов необходимо указать с каким форматом и размещением вложений происходит работа на конкретных этапах
 */
void VkRenderer::initRenderPassPrimary(const vk::Format &colorAttachmentFormat, const vk::Format &depthStencilAttachmentFormat)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize render pass. Device not ready");
    }
    // Проверяем готовность поверхности
    if(!surface_.get()){
        throw vk::InitializationFailedError("Can't initialize render pass. Surface not ready");
    }
    // Проверяем поддержку формата цветового вложения
    if(!device_.isFormatSupported(colorAttachmentFormat,surface_)){
        throw vk::FormatNotSupportedError("Can't initialize render pass. Color attachment format not supported");
    }
    // Проверяем поддержку формата вложений глубины
    if(!device_.isDepthStencilSupportedForFormat(depthStencilAttachmentFormat)){
        throw vk::FormatNotSupportedError("Can't initialize render pass. Depth-stencil attachment format not supported");
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
    depthStAttDesc.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;         // Трафарет. Начало под-прохода - очищать
    depthStAttDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;       // Трафарет. Конец под-прохода - не важно
    depthStAttDesc.initialLayout = vk::ImageLayout::eUndefined;             // Макет памяти изображения в начале - не важно
    depthStAttDesc.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;  // Макет памяти изображения в конце - буфер глубины-трафарета
    attachmentDescriptions.push_back(depthStAttDesc);

    // Ссылки на вложения
    // Они содержат индексы описаний вложений, они также совместимы с порядком вложений в кадровом буфере, который привязывается во время начала прохода
    // Также ссылка определяет макет памяти вложения, который используется во время под-прохода
    vk::AttachmentReference colorAttachmentReferences[1] = {{0,vk::ImageLayout::eColorAttachmentOptimal}};
    vk::AttachmentReference depthAttachmentReference{1,vk::ImageLayout::eDepthStencilAttachmentOptimal};

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
    externalToFirst.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
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
    firstToExternal.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
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

    // Создать проход рендеринга
    renderPassPrimary_ = device_.getLogicalDevice()->createRenderPassUnique(renderPassCreateInfo);
}

/**
 * Де-инициализация проходов рендеринга
 */
void VkRenderer::deInitRenderPassPrimary() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить проход и освободить smart-pointer
    device_.getLogicalDevice()->destroyRenderPass(renderPassPrimary_.get());
    renderPassPrimary_.release();
}

/**
 * Инициализация прохода для пост-обработки
 * @param colorAttachmentFormat Формат цветовых вложений
 * @details В отличии от основного прохода, проход пост-обработки пишет только в цветовое вложение
 */
void VkRenderer::initRenderPassPostProcess(const vk::Format &colorAttachmentFormat)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize render pass. Device not ready");
    }
    // Проверяем готовность поверхности
    if(!surface_.get()){
        throw vk::InitializationFailedError("Can't initialize render pass. Surface not ready");
    }
    // Проверяем поддержку формата цветового вложения
    if(!device_.isFormatSupported(colorAttachmentFormat,surface_)){
        throw vk::FormatNotSupportedError("Can't initialize render pass. Color attachment format not supported");
    }

    // Массив описаний вложений
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

    // Ссылки на вложения
    // Они содержат индексы описаний вложений, они также совместимы с порядком вложений в кадровом буфере, который привязывается во время начала прохода
    // Также ссылка определяет макет памяти вложения, который используется во время под-прохода
    vk::AttachmentReference colorAttachmentReferences[1] = {{0,vk::ImageLayout::eColorAttachmentOptimal}};

    // Описываем под-проход
    // Проход должен содержать в себе как минимум один под-проход
    vk::SubpassDescription subPassDescription{};
    subPassDescription.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;   // Тип конвейера - графический
    subPassDescription.colorAttachmentCount = 1;                               // Кол-во цветовых вложений
    subPassDescription.pColorAttachments = colorAttachmentReferences;          // Цветовые вложения
    subPassDescription.pDepthStencilAttachment = nullptr;                      // Вложение глубины-трафарета
    subPassDescription.inputAttachmentCount = 0;                               // Кол-во входных вложений (например, с предыдущих под-проходов)
    subPassDescription.pInputAttachments = nullptr;                            // Нет входных вложений на этом под-проходе
    subPassDescription.preserveAttachmentCount = 0;                            // Кол-во хранимых вложений (не используются в этом под-проходе, но хранятся для следующих)
    subPassDescription.pPreserveAttachments = nullptr;                         // Нет хранимых вложений на этом под-проходе
    subPassDescription.pResolveAttachments = nullptr;

    // Описываем зависимости (порядок) под-проходов
    // Хоть в проходе использован только один под-проход, существует также еще и неявный (внешний) под-проход
    std::vector<vk::SubpassDependency> subPassDependencies;

    // Переход от внешнего (неявного) под-прохода к первому (нулевому)
    vk::SubpassDependency externalToFirst;
    externalToFirst.srcSubpass = VK_SUBPASS_EXTERNAL;
    externalToFirst.dstSubpass = 0;
    externalToFirst.srcStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    externalToFirst.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    externalToFirst.srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    externalToFirst.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    externalToFirst.dependencyFlags = vk::DependencyFlagBits::eByRegion;
    subPassDependencies.push_back(externalToFirst);

    // Переход от первого (нулевого) ко внешнему (неявному) под-проходу
    vk::SubpassDependency firstToExternal;
    firstToExternal.srcSubpass = 0;
    firstToExternal.dstSubpass = VK_SUBPASS_EXTERNAL;
    firstToExternal.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    firstToExternal.dstStageMask = vk::PipelineStageFlagBits::eFragmentShader;
    firstToExternal.srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    firstToExternal.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
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

    // Создать проход рендеринга
    renderPassPostProcess_ = device_.getLogicalDevice()->createRenderPassUnique(renderPassCreateInfo);
}

/**
 * Де-инициализация прохода пост-обработки
 */
void VkRenderer::deInitRenderPassPostProcess() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить проход и освободить smart-pointer
    device_.getLogicalDevice()->destroyRenderPass(renderPassPostProcess_.get());
    renderPassPostProcess_.release();
}

/**
 * Инициализация swap-chain (цепочки показа)
 * Цепочка показа - набор сменяющихся изображений показываемых на поверхности отображения
 * @param surfaceFormat Формат поверхности
 * @param bufferCount Желаемое кол-во буферов изображений (0 для авто-определения)
 */
void VkRenderer::initSwapChain(const vk::SurfaceFormatKHR &surfaceFormat, size_t bufferCount)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize swap-chain. Device not ready");
    }
    // Проверяем готовность поверхности
    if(!surface_.get()){
        throw vk::InitializationFailedError("Can't initialize swap-chain. Surface not ready");
    }
    // Проверить поддерживается ли формат поверхности
    if(!device_.isSurfaceFormatSupported(surfaceFormat,surface_)){
        throw vk::FormatNotSupportedError("Can't initialize swap-chain. Surface format not supported");
    }

    // Получить возможности устройства для поверхности
    auto capabilities = device_.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface_.get());

    // Если задано кол-во буферов
    if(bufferCount > 0){
        if(bufferCount < capabilities.minImageCount || bufferCount > capabilities.maxImageCount){
            throw vk::InitializationFailedError("Can't initialize swap-chain. Unsupported buffer count required. Please change it");
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
        auto presentModes = device_.getPhysicalDevice().getSurfacePresentModesKHR(surface_.get());
        for(const auto& presentModeEntry : presentModes){
            if(presentModeEntry == vk::PresentModeKHR::eMailbox){
                presentMode = presentModeEntry;
                break;
            }
        }
    }

    // Старый swap-chain
    const auto oldSwapChain = swapChainKhr_.get() ? swapChainKhr_.get() : nullptr;

    // Индексы семейств очередей участвующих в рендеринге и показе
    auto renderingQueueFamilies = device_.getQueueFamilyIndices();

    // Конфигурация swap-chain
    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.surface = surface_.get();
    swapChainCreateInfo.minImageCount = bufferCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = capabilities.currentExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = (device_.isPresentAndGfxQueueFamilySame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent);
    swapChainCreateInfo.queueFamilyIndexCount = (swapChainCreateInfo.imageSharingMode == vk::SharingMode::eConcurrent ? renderingQueueFamilies.size() : 0);
    swapChainCreateInfo.pQueueFamilyIndices = (swapChainCreateInfo.imageSharingMode == vk::SharingMode::eConcurrent ? renderingQueueFamilies.data() : nullptr);
    swapChainCreateInfo.preTransform = capabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.presentMode = presentMode;
    swapChainCreateInfo.clipped = true;
    swapChainCreateInfo.oldSwapchain = oldSwapChain;


    // Если у unique-pointer'а есть какой-то объект во владении - освобождаем его (это может быть старый swap-chain)
    swapChainKhr_.release();

    // Создаем swap-chain
    swapChainKhr_ = device_.getLogicalDevice()->createSwapchainKHRUnique(swapChainCreateInfo);

    // Уничтожить старый swap-chain если он был
    if(!oldSwapChain){
        device_.getLogicalDevice()->destroySwapchainKHR(oldSwapChain);
    }
}

/**
 * Де-инициализация swap-chain (цепочки показа)
 */
void VkRenderer::deInitSwapChain() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить swap-chain и освободить smart-pointer
    device_.getLogicalDevice()->destroySwapchainKHR(swapChainKhr_.get());
    swapChainKhr_.release();
}

/**
 * Инициализация кадровых буферов
 * @param colorAttachmentFormat Формат цветовых вложений
 * @param depthStencilAttachmentFormat Формат вложений глубины
 */
void VkRenderer::initFrameBuffersPrimary(const vk::Format &colorAttachmentFormat, const vk::Format &depthStencilAttachmentFormat)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Device not ready");
    }
    // Проверяем готовность поверхности
    if(!surface_.get()){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Surface not ready");
    }
    // Проверяем готовность swap-chain'а
    if(!swapChainKhr_.get()){
        throw vk::InitializationFailedError("Can't initialize frame-buffers. Swap-chain not ready");
    }
    // Проверяем готовность прохода для которого создаются буферы
    if(!renderPassPrimary_.get()){
        throw vk::InitializationFailedError("Can't initialize frame-buffers. Required render pass not ready");
    }

    // Получить изображения swap-chain'а
    auto swapChainImages = device_.getLogicalDevice()->getSwapchainImagesKHR(swapChainKhr_.get());

    // Получить возможности устройства для поверхности
    auto capabilities = device_.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface_.get());

    // Пройтись по всем изображениям и создать кадровый буфер для каждого
    for(const auto& swapChainImage : swapChainImages)
    {
        // Описываем вложения кадрового буфера
        // Порядок вложений должен совпадать с порядком вложений в описании прохода рендеринга
        std::vector<vk::resources::FrameBufferAttachmentInfo> attachmentsInfo = {
                // Для цветового вложения уже существует изображение swap-chain
                // По этой причине не нужно создавать его и выделять память, достаточно передать указатель на него
                {
                    &swapChainImage,
                    vk::ImageType::e2D,
                    colorAttachmentFormat,
                    {},
                    vk::ImageAspectFlagBits::eColor
                },
                // Для вложения глубины-трафарета отсутствует изображение, поэтому оно должно быть создано
                // Объект изображения и объект его памяти будут содержаться в самом вложении
                {
                    nullptr,
                    vk::ImageType::e2D,
                    depthStencilAttachmentFormat,
                    vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                    vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil
                }
        };

        // Создаем кадровый буфер со всеми необходимыми вложениями и добавляем его в массив
        frameBuffersPrimary_.emplace_back(vk::resources::FrameBuffer(
                &device_,
                renderPassPrimary_,
                {capabilities.currentExtent.width,capabilities.currentExtent.height,1},
                attachmentsInfo));
    }
}

/**
 * Де-инициализация кадровых буферов
 */
void VkRenderer::deInitFrameBuffersPrimary() noexcept
{
    // Очистка всех ресурсов Vulkan происходит в деструкторе объекта frame-buffer'а
    // Достаточно вызвать очистку массива
    frameBuffersPrimary_.clear();
}


/**
 * Инициализация кадровых буферов для пост-процессинга
 * @param colorAttachmentFormat Формат цветовых вложений
 *
 * @details Это итоговые кадровые буферы, которые используются для показа.
 */
void VkRenderer::initFrameBuffersPostProcess(const vk::Format &colorAttachmentFormat)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Device not ready");
    }
    // Проверяем готовность поверхности
    if(!surface_.get()){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Surface not ready");
    }
    // Проверяем готовность swap-chain'а
    if(!swapChainKhr_.get()){
        throw vk::InitializationFailedError("Can't initialize frame-buffers. Swap-chain not ready");
    }
    // Проверяем готовность прохода для которого создаются буферы
    if(!renderPassPostProcess_.get()){
        throw vk::InitializationFailedError("Can't initialize frame-buffers. Required render pass not ready");
    }

    // Получить изображения swap-chain'а
    auto swapChainImages = device_.getLogicalDevice()->getSwapchainImagesKHR(swapChainKhr_.get());

    // Получить возможности устройства для поверхности
    auto capabilities = device_.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface_.get());

    // Пройтись по всем изображениям и создать кадровый буфер для каждого
    for(const auto& swapChainImage : swapChainImages)
    {
        // Описываем вложения кадрового буфера
        // Порядок вложений должен совпадать с порядком вложений в описании прохода рендеринга
        std::vector<vk::resources::FrameBufferAttachmentInfo> attachmentsInfo = {
                // Для цветового вложения уже существует изображение swap-chain
                // По этой причине не нужно создавать его и выделять память, достаточно передать указатель на него
                {
                        &swapChainImage,
                        vk::ImageType::e2D,
                        colorAttachmentFormat,
                        {},
                        vk::ImageAspectFlagBits::eColor
                },
        };

        // Создаем кадровый буфер со всеми необходимыми вложениями и добавляем его в массив
        frameBuffersPostProcess_.emplace_back(vk::resources::FrameBuffer(
                &device_,
                renderPassPostProcess_,
                {capabilities.currentExtent.width,capabilities.currentExtent.height,1},
                attachmentsInfo));
    }
}

/**
 * Де-инициализация кадровых буферов для пост-процессинга
 */
void VkRenderer::deInitFrameBuffersPostProcess() noexcept
{
    // Очистка всех ресурсов Vulkan происходит в деструкторе объекта frame-buffer'а
    // Достаточно вызвать очистку массива
    frameBuffersPostProcess_.clear();
}


/**
 * Инициализация дескрипторов (наборов дескрипторов)
 * @param maxMeshes Максимальное кол-во одновременно отображающихся мешей (влияет на максимальное кол-во наборов для материала меша и прочего)
 */
void VkRenderer::initDescriptorPoolsAndLayouts(size_t maxMeshes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize descriptors. Device not ready");
    }

    // Д Е С К Р И П Т О Р Н Ы Е  П У Л Ы
    // Наборы дескрипторов выделяются из пулов. При создании пула, необходимо знать какие дескрипторы и сколько их
    // будут в наборах, которые будут выделяться из пула, а так же сколько таких наборов всего будет

    // Создать пул для набора камеры
    {
        // Размеры пула для наборов типа "UBO"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Один дескриптор в наборе отвечает за обычный UBO буфер (привязывается единожды за кадр)
                {vk::DescriptorType::eUniformBuffer, 1},
        };

        // Нам нужен один набор данного типа, он будет привязываться единожды за кадр (камера, вид, проекция)
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolCamera_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Создать пул для наборов мешей
    {
        // Размеры пула для наборов типа "материал меша"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Дескриптор матрицы модели
                {vk::DescriptorType::eUniformBuffer,1},
                // Дескриптор параметров отображения текстуры
                {vk::DescriptorType::eUniformBuffer,1},
                // Дескриптор для параметров материала
                {vk::DescriptorType::eUniformBuffer,1},
                // Дескриптор для текстуры/семплера
                {vk::DescriptorType::eCombinedImageSampler, 4},
                // Дескриптор для параметров использования текстур
                {vk::DescriptorType::eUniformBuffer, 1},
                // Дескриптор для буфера кол-ва костей скелетной анимации
                {vk::DescriptorType::eUniformBuffer, 1},
                // Дескриптор для буфера трансформаций костей скелетной анимации
                {vk::DescriptorType::eUniformBuffer, 1}
        };

        // Поскольку у каждого меша есть свой набор, то кол-во таких наборов ограничено кол-вом мешей
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = maxMeshes;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolMeshes_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Создать пул для набора описывающего источники света сцены
    {
        // Размеры пула для наборов типа "материал меша"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Дескриптор кол-ва источников
                {vk::DescriptorType::eUniformBuffer,1},
                // Дескриптор массива источников
                {vk::DescriptorType::eUniformBuffer,1},
        };

        // Нам нужен один набор данного типа, он будет привязываться единожды за кадр (кол-во источников и массив источников)
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolLightSources_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Создать пул для набора используемого в пост-обработке
    {
        // Размеры пула для наборов типа "материал меша"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Дескриптор для текстуры/семплера
                {vk::DescriptorType::eCombinedImageSampler, 1}
        };

        // Поскольку у каждого меша есть свой набор, то кол-во таких наборов ограничено кол-вом мешей
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = maxMeshes;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolImagesToPostProcess_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Р А З М Е Щ Е Н И Я  Н А Б О Р О В  Д Е С К Р И П Т О Р О В
    // Макет размещения набора подробно описывает какие конкретно типы дескрипторов будут в наборе, сколько их, на каком
    // этапе конвейера они доступы, а так же какой у них индекс привязки (binding) в шейдере

    // Макет размещения набора камеры
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // Обычный UBO буфер (для матриц вида, проекции и для положения камеры)
                {
                        0,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eVertex|vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutCamera_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // Макет размещения материала меша
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // UBO буфер для матриц
                {
                        0,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eVertex,
                        nullptr,
                },
                // UBO буфер для параметров отображения текстуры
                {
                        1,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eVertex,
                        nullptr,
                },
                // UBO буфер для параметров материала
                {
                        2,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
                // Текстурный семплер
                {
                        3,
                        vk::DescriptorType::eCombinedImageSampler,
                        4,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
                // UBO буфер для параметров использования текстур
                {
                        4,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
                // UBO буфер для кол-ва костей скелетной анимации
                {
                    5,
                    vk::DescriptorType::eUniformBuffer,
                    1,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry,
                    nullptr
                },
                // UBO буфер для матриц трансформаций костей скелетной анимации
                {
                    6,
                    vk::DescriptorType::eUniformBuffer,
                    1,
                    vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry,
                    nullptr
                }
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutMeshes_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // Макет размещения набора описывающего источники света сцены
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // Обычный UBO буфер (для значения кол-ва источников)
                {
                        0,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
                // Обычный UBO буфер (для массива источников)
                {
                        1,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutLightSources_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // Макет размещения набора используемого при пост-обработке (дескриптор буфера изображения)
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // Текстурный семплер
                {
                        0,
                        vk::DescriptorType::eCombinedImageSampler,
                        1,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutImagesToPostProcess_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }
}

/**
 * Де-инициализация дескрипторов
 */
void VkRenderer::deInitDescriptorPoolsAndLayouts() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить все выделенные наборы дескрипторов
    // Поскольку функция может быть вызвана в деструкторе важно гарантировать отсутствие исключений
    try{
        device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolCamera_.get());
        device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolMeshes_.get());
        device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolLightSources_.get());
        device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolImagesToPostProcess_.get());
    }
	catch(std::exception&){}

    // Уничтожить размещения дескрипторов
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutCamera_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutMeshes_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutLightSources_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutImagesToPostProcess_.get());
    descriptorSetLayoutCamera_.release();
    descriptorSetLayoutMeshes_.release();
    descriptorSetLayoutLightSources_.release();
    descriptorSetLayoutImagesToPostProcess_.release();

    // Уничтожить пулы дескрипторов
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolCamera_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolMeshes_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolLightSources_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolImagesToPostProcess_.get());
    descriptorPoolCamera_.release();
    descriptorPoolMeshes_.release();
    descriptorPoolLightSources_.release();
    descriptorPoolImagesToPostProcess_.release();
}

/**
 * Инициализация графического конвейера
 * @param vertexShaderCodeBytes Код вершинного шейдера
 * @param geometryShaderCodeBytes Код геометрического шейдера
 * @param fragmentShaderCodeBytes Код фрагментного шейдера
 */
void VkRenderer::initPipelinePrimary(
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& geometryShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Device not ready");
    }
    // Проверяем готовность основного прохода рендеринга
    if(!renderPassPrimary_.get()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Render pass not ready");
    }

    // М А К Е Т  Р А З М Е Щ Е Н И Я  К О Н В Е Й Е Р А

    // Массив макетов размещения дескрипторов
    // ВНИМАНИЕ! Порядок следования наборов в шейдере (индексы дескрипторных наборов) зависит от порядка указания
    // макетов размещения в данном массиве.
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
            descriptorSetLayoutCamera_.get(),
            descriptorSetLayoutLightSources_.get(),
            descriptorSetLayoutMeshes_.get()
    };

    // Создать макет размещения конвейера
    pipelineLayoutPrimary_ = device_.getLogicalDevice()->createPipelineLayoutUnique({
        {},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data()
    });

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
                    static_cast<uint32_t>(offsetof(vk::tools::Vertex, position))
            },
            {
                    1,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    static_cast<uint32_t>(offsetof(vk::tools::Vertex, color))
            },
            {
                    2,
                    0,
                    vk::Format::eR32G32Sfloat,
                    static_cast<uint32_t>(offsetof(vk::tools::Vertex, uv))
            },
            {
                    3,
                    0,
                    vk::Format::eR32G32B32Sfloat,
                    static_cast<uint32_t>(offsetof(vk::tools::Vertex, normal))
            },
            {
                    4,
                    0,
                    vk::Format::eR32G32B32A32Sint,
                    static_cast<uint32_t>(offsetof(vk::tools::Vertex, boneIndices))
            },
            {
                    5,
                    0,
                    vk::Format::eR32G32B32A32Sfloat,
                    static_cast<uint32_t>(offsetof(vk::tools::Vertex, weights))
            }
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
    if(vertexShaderCodeBytes.empty() || fragmentShaderCodeBytes.empty() || geometryShaderCodeBytes.empty()){
        throw vk::InitializationFailedError("No shader code provided");
    }

    // Вершинный шейдер
    vk::ShaderModule shaderModuleVs = device_.getLogicalDevice()->createShaderModule({
            {},
            vertexShaderCodeBytes.size(),
            reinterpret_cast<const uint32_t*>(vertexShaderCodeBytes.data())});

    // Геометрический шейдер
    vk::ShaderModule shaderModuleGs = device_.getLogicalDevice()->createShaderModule({
            {},
            geometryShaderCodeBytes.size(),
            reinterpret_cast<const uint32_t*>(geometryShaderCodeBytes.data())});

    // Фрагментный шейдер
    vk::ShaderModule shaderModuleFs = device_.getLogicalDevice()->createShaderModule({
            {},
            fragmentShaderCodeBytes.size(),
            reinterpret_cast<const uint32_t*>(fragmentShaderCodeBytes.data())});

    // Описываем стадии
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,shaderModuleVs,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eGeometry,shaderModuleGs,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,shaderModuleFs,"main")
    };

    // V I E W  P O R T  &  S C I S S O R S

    // Получить разрешение view-port'а
    auto viewPortExtent = frameBuffersPrimary_[0].getExtent();

    // Настройки области отображения (статическая настройка, на случай отключения динамического состояния)
    vk::Viewport viewport{};
    viewport.setX(0.0f);
    viewport.setWidth(static_cast<float>(viewPortExtent.width));
    viewport.setY(inputDataInOpenGlStyle_ ? static_cast<float>(viewPortExtent.height) : 0.0f);
    viewport.setHeight(inputDataInOpenGlStyle_ ? -static_cast<float>(viewPortExtent.height) : static_cast<float>(viewPortExtent.height));
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    // Настройки обрезки
    vk::Rect2D scissors{};
    scissors.offset.x = 0;
    scissors.offset.y = 0;
    scissors.extent.width = viewPortExtent.width;
    scissors.extent.height = viewPortExtent.height;

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
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;                 // Тест границ глубины отключен
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

    // Д И Н А М И Ч. С О С Т О Я Н И Я

    // Динамически (при помощи команд) будет изменяться пока-что только view-port
    std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };

    // Конфигурация динамических состояний
    vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
    pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

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
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
    graphicsPipelineCreateInfo.layout = pipelineLayoutPrimary_.get();
    graphicsPipelineCreateInfo.renderPass = renderPassPrimary_.get();
    graphicsPipelineCreateInfo.subpass = 0;
    auto pipeline = device_.getLogicalDevice()->createGraphicsPipeline(nullptr,graphicsPipelineCreateInfo);

    // Уничтожить шейдерные модули (конвейер создан, они не нужны)
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleVs);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleFs);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleGs);

    // Вернуть unique smart pointer
    pipelinePrimary_ = vk::UniquePipeline(pipeline);
}

/**
 * Де-инициализация графического конвейера
 */
void VkRenderer::deInitPipelinePrimary() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить конвейер
    device_.getLogicalDevice()->destroyPipeline(pipelinePrimary_.get());
    pipelinePrimary_.release();

    // Уничтожить размещение конвейера
    device_.getLogicalDevice()->destroyPipelineLayout(pipelineLayoutPrimary_.get());
    pipelineLayoutPrimary_.release();
}

/**
 * Инициализация конвейера пост-обработки
 * @param vertexShaderCodeBytes
 * @param fragmentShaderCodeBytes
 */
void VkRenderer::initPipelinePostProcess(const std::vector<unsigned char> &vertexShaderCodeBytes,
                                         const std::vector<unsigned char> &fragmentShaderCodeBytes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Device not ready");
    }
    // Проверяем готовность основного прохода рендеринга
    if(!renderPassPostProcess_.get()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Render pass not ready");
    }

    // М А К Е Т  Р А З М Е Щ Е Н И Я  К О Н В Е Й Е Р А

    // Массив макетов размещения дескрипторов
    // ВНИМАНИЕ! Порядок следования наборов в шейдере (индексы дескрипторных наборов) зависит от порядка указания
    // макетов размещения в данном массиве.
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
            descriptorSetLayoutImagesToPostProcess_.get(),
    };

    // Создать макет размещения конвейера
    pipelineLayoutPostProcess_ = device_.getLogicalDevice()->createPipelineLayoutUnique({
        {},
        static_cast<uint32_t>(descriptorSetLayouts.size()),
        descriptorSetLayouts.data()
    });

    // Э Т А П  В В О Д А  Д А Н Н Ы Х

    // На этапе пост-процессинга не используется геометрия (буферы вершин и индексов)
    // Рисуется только квадрат (либо треугольник) на весь экран, вершины которого формируются напрямую в вершинном шейдере
    vk::PipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
    pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
    pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

    // Э Т А П  С Б О Р К И  П Р И М И Т И В О В

    vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo{};
    pipelineInputAssemblyStateCreateInfo.topology = vk::PrimitiveTopology::eTriangleList; // Ожидаем обычные треугольники
    pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;

    // Ш Е Й Д Е Р Ы ( П Р О Г Р А М И Р У Е М Ы Е  С Т А Д И И)

    // Убеждаемся что шейдерный код был предоставлен
    if(vertexShaderCodeBytes.empty() || fragmentShaderCodeBytes.empty()){
        throw vk::InitializationFailedError("No shader code provided");
    }

    // Вершинный шейдер
    vk::ShaderModule shaderModuleVs = device_.getLogicalDevice()->createShaderModule({
        {},
        vertexShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(vertexShaderCodeBytes.data())});

    // Фрагментный шейдер
    vk::ShaderModule shaderModuleFs = device_.getLogicalDevice()->createShaderModule({
        {},
        fragmentShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(fragmentShaderCodeBytes.data())});

    // Описываем стадии
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,shaderModuleVs,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,shaderModuleFs,"main")
    };

    // V I E W  P O R T  &  S C I S S O R S

    // Получить разрешение view-port'а
    auto viewPortExtent = frameBuffersPrimary_[0].getExtent();

    // Настройки области отображения (статическая настройка, на случай отключения динамического состояния)
    vk::Viewport viewport{};
    viewport.setX(0.0f);
    viewport.setWidth(static_cast<float>(viewPortExtent.width));
    viewport.setY(inputDataInOpenGlStyle_ ? static_cast<float>(viewPortExtent.height) : 0.0f);
    viewport.setHeight(inputDataInOpenGlStyle_ ? -static_cast<float>(viewPortExtent.height) : static_cast<float>(viewPortExtent.height));
    viewport.setMinDepth(0.0f);
    viewport.setMaxDepth(1.0f);

    // Настройки обрезки
    vk::Rect2D scissors{};
    scissors.offset.x = 0;
    scissors.offset.y = 0;
    scissors.extent.width = viewPortExtent.width;
    scissors.extent.height = viewPortExtent.height;

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
    pipelineRasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eNone; // Отсекать задние грани
    pipelineRasterizationStateCreateInfo.frontFace = vk::FrontFace::eClockwise;  // Передние грани описываются по часовой
    pipelineRasterizationStateCreateInfo.depthBiasEnable = false;
    pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
    pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
    pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

    // Параметры теста глубины (не использовать тест глубины)
    vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
    pipelineDepthStencilStateCreateInfo.depthTestEnable = false;                      // Тест глубины включен
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = false;                     // Запись в тест глубины включена
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;                // Тест границ глубины отключен
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = false;                    // Тест трафарета отключен

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
    pipelineColorBlendAttachmentState.blendEnable = false;
    pipelineColorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

    // Описываем общие настройки смешивания
    vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo{};
    pipelineColorBlendStateCreateInfo.logicOpEnable = false;
    pipelineColorBlendStateCreateInfo.logicOp = vk::LogicOp::eCopy;
    pipelineColorBlendStateCreateInfo.attachmentCount = 1;
    pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;

    // Д И Н А М И Ч. С О С Т О Я Н И Я

    // Динамически (при помощи команд) будет изменяться пока-что только view-port
    std::vector<vk::DynamicState> dynamicStates = {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
    };

    // Конфигурация динамических состояний
    vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
    pipelineDynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
    pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();

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
    graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
    graphicsPipelineCreateInfo.layout = pipelineLayoutPostProcess_.get();
    graphicsPipelineCreateInfo.renderPass = renderPassPostProcess_.get();
    graphicsPipelineCreateInfo.subpass = 0;
    auto pipeline = device_.getLogicalDevice()->createGraphicsPipeline(nullptr,graphicsPipelineCreateInfo);

    // Уничтожить шейдерные модули (конвейер создан, они не нужны)
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleVs);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleFs);

    // Вернуть unique smart pointer
    pipelinePostProcess_ = vk::UniquePipeline(pipeline);
}

/**
 * Де-инициализация конвейера пост-обработки
 */
void VkRenderer::deInitPipelinePostProcess() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить конвейер
    device_.getLogicalDevice()->destroyPipeline(pipelinePostProcess_.get());
    pipelinePostProcess_.release();

    // Уничтожить размещение конвейера
    device_.getLogicalDevice()->destroyPipelineLayout(pipelineLayoutPostProcess_.get());
    pipelineLayoutPostProcess_.release();
}


/**
 * Освобождение геометрических буферов
 */
void VkRenderer::freeGeometryBuffers()
{
    // Пройтись по всем буферам и уничтожить выделенные ресурсы
    // Объекты без ресурсов Vulkan могут быть корректно уничтожены
    for(const auto& bufferPtrEntry : geometryBuffers_)
    {
        bufferPtrEntry->destroyVulkanResources();
    }
}

/**
 * Освобождение текстурных буферов
 *
 * @details Перед де-инициализацией объекта рендерера, перед уничтожением устройства, будет правильным очистить все
 * текстурные буферы которые когда либо выделялись.
 */
void VkRenderer::freeTextureBuffers()
{
    // Пройтись по всем буферам и уничтожить выделенные ресурсы
    // Объекты без ресурсов Vulkan могут быть корректно уничтожены
    for(const auto& bufferPtrEntry : textureBuffers_)
    {
        bufferPtrEntry->destroyVulkanResources();
    }
}

/**
 * Очистка ресурсов мешей
 */
void VkRenderer::freeMeshes()
{
    // Пройтись по всем мешам и уничтожить выделенные ресурсы
    // Объекты без ресурсов Vulkan могут быть корректно уничтожены
    for(const auto& meshPtrEntry : sceneMeshes_)
    {
        meshPtrEntry->destroyVulkanResources();
    }
}

/** C O N S T R U C T O R - D E S T R U C T O R **/

/**
 * Конструктор
 * @param hInstance Экземпляр WinApi приложения
 * @param hWnd Дескриптор окна WinApi
 * @param vertexShaderCodeBytes Код вершинного шейдера (байты)
 * @param geometryShaderCodeBytes Код геометрического шейдера (байты)
 * @param fragmentShaderCodeBytes Rод фрагментного шейдера (байты)
 * @param maxMeshes Максимальное кол-во мешей
 */
VkRenderer::VkRenderer(HINSTANCE hInstance,
        HWND hWnd,
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& geometryShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes,
        const std::vector<unsigned char>& vertexShaderCodeBytesPp,
        const std::vector<unsigned char>& fragmentShaderCodeBytesPp,
        size_t maxMeshes):
isEnabled_(true),
isCommandsReady_(false),
inputDataInOpenGlStyle_(true),
useValidation_(true)
{
    // Инициализация экземпляра Vulkan
    std::vector<const char*> instanceExtensionNames = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    std::vector<const char*> instanceValidationLayerNames = {};

    if(useValidation_){
        instanceExtensionNames.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        instanceValidationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
    }

    this->vulkanInstance_ = vk::tools::CreateVulkanInstance("My Application",
            "My engine",
            1,1,
            instanceExtensionNames,
            instanceValidationLayerNames);
    std::cout << "Vulkan instance created." << std::endl;

    // Загрузка функций расширений (получение адресов)
    vkExtInitInstance(static_cast<VkInstance>(vulkanInstance_.get()));

    // Создание debug report callback'а (создается только в том случае, если расширение VK_EXT_DEBUG_REPORT_EXTENSION_NAME использовано)
    if(useValidation_){
        debugReportCallbackExt_ = vulkanInstance_->createDebugReportCallbackEXTUnique(vk::DebugReportCallbackCreateInfoEXT({vk::DebugReportFlagBitsEXT::eError|vk::DebugReportFlagBitsEXT::eWarning},vk::tools::DebugVulkanCallback));
        std::cout << "Report callback object created." << std::endl;
    }

    // Создание поверхности отображения на окне
    this->surface_ = vulkanInstance_->createWin32SurfaceKHRUnique(vk::Win32SurfaceCreateInfoKHR({},hInstance,hWnd));
    std::cout << "Surface created." << std::endl;

    // Создание устройства
    std::vector<const char*> deviceExtensionNames = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME
    };

    std::vector<const char*> deviceValidationLayerNames = {};

    if(useValidation_){
        deviceValidationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
    }

    device_ = vk::tools::Device(vulkanInstance_,surface_,deviceExtensionNames,deviceValidationLayerNames, false);
    std::cout << "Device initialized (" << device_.getPhysicalDevice().getProperties().deviceName << ")" << std::endl;

    // Инициализация прохода/проходов рендеринга
    this->initRenderPassPrimary(vk::Format::eB8G8R8A8Unorm, vk::Format::eD32SfloatS8Uint);
    this->initRenderPassPostProcess(vk::Format::eB8G8R8A8Unorm);
    std::cout << "Render passes initialized." << std::endl;

    // Инициализация цепочки показа (swap-chain)
    this->initSwapChain({vk::Format::eB8G8R8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear});
    std::cout << "Swap-chain created." << std::endl;

    // Создание основных кадровых буферов
    this->initFrameBuffersPrimary(vk::Format::eB8G8R8A8Unorm, vk::Format::eD32SfloatS8Uint);
    std::cout << "Primary frame-buffers initialized (" << frameBuffersPrimary_.size() << ") [" << frameBuffersPrimary_[0].getExtent().width << " x " << frameBuffersPrimary_[0].getExtent().height << "]" << std::endl;

    // Создание кадровых буферов для пост-обработки
    this->initFrameBuffersPostProcess(vk::Format::eB8G8R8A8Unorm);
    std::cout << "Post process frame-buffers initialized (" << frameBuffersPostProcess_.size() << ") [" << frameBuffersPostProcess_[0].getExtent().width << " x " << frameBuffersPostProcess_[0].getExtent().height << "]" << std::endl;

    // Выделение командных буферов
    // Командный буфер может быть и один, но в таком случае придется ожидать его выполнения перед тем, как начинать запись
    // в очередное изображение swap-chain'а (что не есть оптимально). Поэтому лучше использовать отдельные буферы, для каждого
    // изображения swap-chain'а (по сути это копии одного и того же буфера)
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(), vk::CommandBufferLevel::ePrimary, frameBuffersPrimary_.size());
    commandBuffers_ = device_.getLogicalDevice()->allocateCommandBuffers(allocInfo);
    std::cout << "Command-buffers allocated (" << commandBuffers_.size() << ")." << std::endl;

    // Создать текстурный семплер по умолчанию
    textureSamplerDefault_ = vk::tools::CreateImageSampler(device_.getLogicalDevice().get(), vk::Filter::eLinear,vk::SamplerAddressMode::eRepeat, 2);
    std::cout << "Default texture sampler created." << std::endl;

    // Инициализация дескрипторных пулов и наборов
    this->initDescriptorPoolsAndLayouts(maxMeshes);
    std::cout << "Descriptor pool and layouts initialized." << std::endl;

    // Создание камеры (UBO буферов и дескрипторных наборов)
    glm::float32 aspectRatio = static_cast<glm::float32>(frameBuffersPrimary_[0].getExtent().width) / static_cast<glm::float32>(frameBuffersPrimary_[0].getExtent().height);
    camera_ = vk::scene::Camera(
            &device_,
            descriptorPoolCamera_,
            descriptorSetLayoutCamera_,
            glm::vec3(0.0f,0.0f,0.0f),
            glm::vec3(0.0f,0.0f,0.0f),
            aspectRatio,
            vk::scene::CameraProjectionType::ePerspective);
    std::cout << "Camera created." << std::endl;

    // Создание объекта набора источников света сцены (UBO буферов и дескрипторных наборов)
    lightSourceSet_ = vk::scene::LightSourceSet(&device_,descriptorPoolLightSources_,descriptorSetLayoutLightSources_,100);
    std::cout << "Light source set created." << std::endl;

    // Создать основной проход рендеринга
    this->initPipelinePrimary(vertexShaderCodeBytes, geometryShaderCodeBytes, fragmentShaderCodeBytes);
    std::cout << "Main graphics pipeline created." << std::endl;

    // Создать проход рендеринга для пост-обраюотки
    this->initPipelinePostProcess(vertexShaderCodeBytesPp,fragmentShaderCodeBytesPp);
    std::cout << "Post-process graphics pipeline created." << std::endl;

    // Создать примитивы синхронизации (семафоры)
    semaphoreReadyToPresent_ = device_.getLogicalDevice()->createSemaphoreUnique({});
    semaphoreReadyToRender_ = device_.getLogicalDevice()->createSemaphoreUnique({});
    std::cout << "Synchronization semaphores created." << std::endl;

    // Создать ресурсы по умолчанию
    unsigned char blackPixel[4] = {0,0,0,255};
    blackPixelTexture_ = this->createTextureBuffer(blackPixel,1,1,4,false,false);
    std::cout << "Default resources created." << std::endl;
}

/**
 * Деструктор
 */
VkRenderer::~VkRenderer()
{
    // Остановка рендеринга
    this->setRenderingStatus(false);

    // Очистка ресурсов по умолчанию
    blackPixelTexture_->destroyVulkanResources();
    std::cout << "Default resources destroyed." << std::endl;

    // Удалить примитивы синхронизации
    device_.getLogicalDevice()->destroySemaphore(semaphoreReadyToRender_.get());
    device_.getLogicalDevice()->destroySemaphore(semaphoreReadyToPresent_.get());
    semaphoreReadyToRender_.release();
    semaphoreReadyToPresent_.release();
    std::cout << "Synchronization semaphores destroyed." << std::endl;

    // Уничтожение конвейера пост-обоаботки
    this->deInitPipelinePostProcess();
    std::cout << "Post processing pipeline destroyed" << std::endl;

    // Уничтожение основного прохода рендеринга
    this->deInitPipelinePrimary();
    std::cout << "Main graphics pipeline destroyed." << std::endl;

    // Очистить все ресурсы мешей
    this->freeMeshes();
    std::cout << "All allocated meshes data freed." << std::endl;

    // Очистить ресурсы набора источников света
    this->lightSourceSet_.destroyVulkanResources();
    std::cout << "Light source set destroyed." << std::endl;

    // Очистить ресурсы камеры
    this->camera_.destroyVulkanResources();
    std::cout << "Camera destroyed." << std::endl;

    // Де-инициализация дескрипторов
    this->deInitDescriptorPoolsAndLayouts();
    std::cout << "Descriptor pool and layouts de-initialized" << std::endl;

    // Уничтожение текстурного семплера по умолчанию
    device_.getLogicalDevice()->destroySampler(textureSamplerDefault_.get());
    textureSamplerDefault_.release();
    std::cout << "Default texture sampler destroyed." << std::endl;

    // Освобождение командных буферов
    device_.getLogicalDevice()->freeCommandBuffers(device_.getCommandGfxPool().get(),commandBuffers_);
    commandBuffers_.clear();
    std::cout << "Command-buffers freed." << std::endl;

    // Де-инициализация кадровых буферов для пост-процессинга
    this->deInitFrameBuffersPostProcess();
    std::cout << "Post-process frame-buffers destroyed." << std::endl;

    // Де-инициализация основных кадровых буферов
    this->deInitFrameBuffersPrimary();
    std::cout << "Primary frame-buffers destroyed." << std::endl;

    // Уничтожение цепочки показа (swap-chain)
    this->deInitSwapChain();
    std::cout << "Swap-chain destroyed." << std::endl;

    // Де-инициализация прохода
    this->deInitRenderPassPrimary();
    this->deInitRenderPassPostProcess();
    std::cout << "Render passes destroyed." << std::endl;

    this->freeTextureBuffers();
    std::cout << "All allocated texture buffers freed." << std::endl;

    // Очистить все выделенные буферы геометрии
    this->freeGeometryBuffers();
    std::cout << "All allocated geometry buffers freed." << std::endl;

    // Уничтожение устройства
    device_.destroyVulkanResources();
    std::cout << "Device destroyed." << std::endl;

    // Уничтожение поверхности
    vulkanInstance_->destroySurfaceKHR(surface_.get());
    surface_.release();
    std::cout << "Surface destroyed." << std::endl;

    // Уничтожение debug report callback'а (если был создан)
    if(useValidation_){
        vulkanInstance_->destroyDebugReportCallbackEXT(debugReportCallbackExt_.get());
        debugReportCallbackExt_.release();
        std::cout << "Report callback object destroyed." << std::endl;
    }

    // Уничтожение (явное) экземпляра Vulkan
    vulkanInstance_->destroy();
    vulkanInstance_.release();
    std::cout << "Vulkan instance destroyed." << std::endl;
}

/**
 * Сменить статус рендеринга
 * @param isEnabled Выполняется ли рендеринг
 */
void VkRenderer::setRenderingStatus(bool isEnabled) noexcept
{
    // Если мы выключаем рендеринг
    if (!isEnabled && isEnabled_) {
        // Ожидаем завершения всех команд
        // Поскольку функция может быть вызвана в деструкторе важно гарантировать отсутствие исключений
        try{
	        device_.getLogicalDevice()->waitIdle();
        }
        catch (std::exception&) {}
    }

    // Сменить статус
    isEnabled_ = isEnabled;
}

/**
 * Вызывается когда поверхность отображения изменилась
 */
void VkRenderer::onSurfaceChanged()
{
    // Приостановить рендеринг
    this->setRenderingStatus(false);

    // Командные буферы чисто логически не зависят от swap-chain, но их кол-во определяется кол-вом изображений swap-chain
    // Кол-во изображений swap-chain может зависеть от особенностей поверхности
    device_.getLogicalDevice()->freeCommandBuffers(device_.getCommandGfxPool().get(),commandBuffers_);
    commandBuffers_.clear();
    std::cout << "Command-buffers freed." << std::endl;

    // Де-инициализация кадровых буферов для пост-процессинга
    this->deInitFrameBuffersPostProcess();
    std::cout << "Post-process frame-buffers destroyed." << std::endl;

    // Де-инициализация основных кадровых буферов
    this->deInitFrameBuffersPrimary();
    std::cout << "Primary frame-buffers destroyed." << std::endl;

    // Ре-инициализация swap-chain (старый swap-chain уничтожается)
    this->initSwapChain({vk::Format::eB8G8R8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear});
    std::cout << "Swap-chain re-created." << std::endl;

    // Создание основных кадровых буферов
    this->initFrameBuffersPrimary(vk::Format::eB8G8R8A8Unorm, vk::Format::eD32SfloatS8Uint);
    std::cout << "Primary frame-buffers initialized (" << frameBuffersPrimary_.size() << ") [" << frameBuffersPrimary_[0].getExtent().width << " x " << frameBuffersPrimary_[0].getExtent().height << "]" << std::endl;

    // Создание кадровых буферов для пост-обработки
    this->initFrameBuffersPostProcess(vk::Format::eB8G8R8A8Unorm);
    std::cout << "Post process frame-buffers initialized (" << frameBuffersPostProcess_.size() << ") [" << frameBuffersPostProcess_[0].getExtent().width << " x " << frameBuffersPostProcess_[0].getExtent().height << "]" << std::endl;

    // Изменить пропорции камеры
    camera_.setAspectRatio(static_cast<glm::float32>(frameBuffersPrimary_[0].getExtent().width) / static_cast<glm::float32>(frameBuffersPrimary_[0].getExtent().height));

    // Инициализация командных буферов
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(), vk::CommandBufferLevel::ePrimary, frameBuffersPrimary_.size());
    commandBuffers_ = device_.getLogicalDevice()->allocateCommandBuffers(allocInfo);
    std::cout << "Command-buffers allocated (" << commandBuffers_.size() << ")." << std::endl;

    // Командные буферы нужно обновить
    isCommandsReady_ = false;

    // Возобновить рендеринг
    this->setRenderingStatus(true);
}

/**
 * Создание геометрического буфера
 * @param vertices Массив вершин
 * @param indices Массив индексов
 * @return Shared smart pointer на объект буфера
 */
vk::resources::GeometryBufferPtr VkRenderer::createGeometryBuffer(const std::vector<vk::tools::Vertex> &vertices, const std::vector<uint32_t> &indices)
{
    auto buffer = std::make_shared<vk::resources::GeometryBuffer>(&device_,vertices,indices);
    geometryBuffers_.push_back(buffer);
    return buffer;
}

/**
 * Создать текстурный буфер
 * @param imageBytes Байты изображения
 * @param width Ширина изображения
 * @param height Высота изображения
 * @param bpp Байт на пиксель
 * @param generateMip Генерация мип-уровней текстуры
 * @param sRgb Использовать цветовое пространство sRGB (гамма-коррекция)
 * @return Shared smart pointer на объект буфера
 */
vk::resources::TextureBufferPtr VkRenderer::createTextureBuffer(const unsigned char *imageBytes, uint32_t width, uint32_t height, uint32_t bpp, bool generateMip, bool sRgb)
{
    auto buffer = std::make_shared<vk::resources::TextureBuffer>(&device_,&textureSamplerDefault_,imageBytes,width,height,bpp,generateMip,sRgb);
    textureBuffers_.push_back(buffer);
    return buffer;
}

/**
 * Добавление меша на сцену
 * @param geometryBuffer Геометрический буфер
 * @param textureSet Текстурный набор
 * @param materialSettings Параметры материала меша
 * @param textureMapping Параметры отображения текстуры
 * @return Shared smart pointer на объект меша
 */
vk::scene::MeshPtr VkRenderer::addMeshToScene(
        const vk::resources::GeometryBufferPtr& geometryBuffer,
        const vk::scene::MeshTextureSet& textureSet,
        const vk::scene::MeshMaterialSettings& materialSettings,
        const vk::scene::MeshTextureMapping& textureMapping)
{
    // Создание меша
    auto mesh = std::make_shared<vk::scene::Mesh>(&device_,descriptorPoolMeshes_,descriptorSetLayoutMeshes_,geometryBuffer, blackPixelTexture_, textureSet, materialSettings, textureMapping);

    // Добавляем в список мешей сцены
    sceneMeshes_.push_back(mesh);

    // Необходимо обновить командные буферы (изменились отображаемые меши)
    // Для этого ставим рендеринг на паузу и дожидаемся исполнения всех команд в очередях
    this->isEnabled_ = false;
    device_.getGraphicsQueue().waitIdle();
    device_.getPresentQueue().waitIdle();

    // Очищаем командные буферы от старых команд
    for(auto& cmdBuffer : commandBuffers_){
        cmdBuffer.reset({});
    }

    // Даем знать что командные буферы нужно обновить при следующем вызове draw
    isCommandsReady_ = false;

    // Возобновляем рендеринг
    this->isEnabled_ = true;

    return mesh;
}

/**
 * Удалить меш со сцены
 * @param meshPtr Shared smart pointer на объект меша
 */
void VkRenderer::removeMeshFromScene(const vk::scene::MeshPtr& meshPtr)
{
    // Приостановить рендеринг
    this->isEnabled_ = false;
    device_.getGraphicsQueue().waitIdle();
    device_.getPresentQueue().waitIdle();

    // Удаляем меш из списка
    sceneMeshes_.erase(std::remove_if(sceneMeshes_.begin(), sceneMeshes_.end(), [&](const vk::scene::MeshPtr& meshEntryPtr){
        return meshPtr.get() == meshEntryPtr.get();
    }), sceneMeshes_.end());

    // Очищаем командные буферы от старых команд
    for(auto& cmdBuffer : commandBuffers_){
        cmdBuffer.reset({});
    }

    // Даем знать что командные буферы нужно обновить при следующем вызове draw
    isCommandsReady_ = false;

    // Возобновляем рендеринг
    this->isEnabled_ = true;

    // Очищаем выделенные ресурсы меша
    meshPtr->destroyVulkanResources();
}

/**
 * Добавить источник света на сцену
 * @param type Тип источника света
 * @param position Положение источника света
 * @param color Цвет источника света
 * @param attenuationLinear Линейный коэффициент затухания
 * @param attenuationQuadratic Квадратичный коэффициент затухания
 * @param cutOffAngle Внутренний угол отсечения света (для типа eSpot)
 * @param cutOffOuterAngle Внешний угол отсечения света (для типа eSpot)
 * @return Shared smart pointer на объект источника
 */
vk::scene::LightSourcePtr VkRenderer::addLightToScene(
        const vk::scene::LightSourceType &type, const glm::vec3 &position, const glm::vec3 &color,
        glm::float32 attenuationLinear, glm::float32 attenuationQuadratic, glm::float32 cutOffAngle,
        glm::float32 cutOffOuterAngle)
{
    if(lightSourceSet_.isReady())
    {
        return lightSourceSet_.addLightSource(type,
                position,color,attenuationLinear,attenuationQuadratic,cutOffAngle,cutOffOuterAngle);
    }

    return nullptr;
}

/**
 * Удалить источник света со сцены
 * @param lightSourcePtr
 */
void VkRenderer::removeLightFromScene(const vk::scene::LightSourcePtr &lightSourcePtr)
{
    if(lightSourceSet_.isReady())
    {
        lightSourceSet_.removeLightSource(lightSourcePtr);
    }
}

/**
 * Доступ к камере
 * @return Константный указатель на объект камеры
 */
vk::scene::Camera* VkRenderer::getCameraPtr(){
    return &camera_;
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
        clearValues[0].color = vk::ClearColorValue( std::array<float, 4>({ 0.0f, 0.0f, 0.0f, 1.0f }));
        clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

        // Описываем начало прохода
        vk::RenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.pNext = nullptr;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = frameBuffersPrimary_[0].getExtent().width;
        renderPassBeginInfo.renderArea.extent.height = frameBuffersPrimary_[0].getExtent().height;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();

        // Размеры области вида
        auto viewPortExtent = frameBuffersPrimary_[0].getExtent();

        // Область вида - динамическое состояние
        vk::Viewport viewport{};
        viewport.setX(0.0f);
        viewport.setWidth(static_cast<float>(viewPortExtent.width));
        viewport.setY(inputDataInOpenGlStyle_ ? static_cast<float>(viewPortExtent.height) : 0.0f);
        viewport.setHeight(inputDataInOpenGlStyle_ ? -static_cast<float>(viewPortExtent.height) : static_cast<float>(viewPortExtent.height));
        viewport.setMinDepth(0.0f);
        viewport.setMaxDepth(1.0f);

        // Параметры ножниц (динамическое состояние)
        vk::Rect2D scissors{};
        scissors.offset.x = 0;
        scissors.offset.y = 0;
        scissors.extent.width = viewPortExtent.width;
        scissors.extent.height = viewPortExtent.height;


        // Подготовка команд (запись в командные буферы)
        for(size_t i = 0; i < commandBuffers_.size(); ++i)
        {
            // Начинаем работу с командным буфером (запись команд)
            vk::CommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
            commandBufferBeginInfo.pNext = nullptr;
            commandBuffers_[i].begin(commandBufferBeginInfo);

            /// Основной проход

            /*
            // Сменить целевой кадровый буфер и начать работу с проходом (это очистит вложения)
            renderPassBeginInfo.renderPass = renderPassPrimary_.get();
            renderPassBeginInfo.framebuffer = frameBuffersPrimary_[i].getVulkanFrameBuffer().get();
            commandBuffers_[i].beginRenderPass(renderPassBeginInfo,vk::SubpassContents::eInline);

            // Привязать графический конвейер
            commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelinePrimary_.get());

            // Установка view-port'а и ножниц
            commandBuffers_[i].setViewport(0,1,&viewport);
            commandBuffers_[i].setScissor(0,1,&scissors);

            // Привязать набор дескрипторов камеры (матрицы вида и проекции)
            commandBuffers_[i].bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipelineLayoutPrimary_.get(),
                    0,
                    {camera_.getDescriptorSet(),lightSourceSet_.getDescriptorSet()},{});

            for(const auto& meshPtr : sceneMeshes_)
            {
                if(meshPtr->isReady() && meshPtr->getGeometryBuffer()->isReady())
                {
                    // Привязать наборы дескрипторов меша (матрица модели, свойства материала, текстуры и прочее)
                    commandBuffers_[i].bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics,
                            pipelineLayoutPrimary_.get(),
                            2,
                            {meshPtr->getDescriptorSet()},{});

                    // Буферы вершин и индексов
                    vk::DeviceSize offsets[1] = {0};
                    auto vBuffer = meshPtr->getGeometryBuffer()->getVertexBuffer().getBuffer().get();
                    auto iBuffer = meshPtr->getGeometryBuffer()->getIndexBuffer().getBuffer().get();

                    if(meshPtr->getGeometryBuffer()->isIndexed()) {
                        commandBuffers_[i].bindVertexBuffers(0,1,&vBuffer,offsets);
                        commandBuffers_[i].bindIndexBuffer(iBuffer,{},vk::IndexType::eUint32);
                        commandBuffers_[i].drawIndexed(meshPtr->getGeometryBuffer()->getIndexCount(),1,0,0,0);
                    } else {
                        commandBuffers_[i].bindVertexBuffers(0,1,&vBuffer,offsets);
                        commandBuffers_[i].draw(meshPtr->getGeometryBuffer()->getVertexCount(),1,0,0);
                    }
                }
            }

            // Завершение прохода добавит неявное преобразование памяти кадрового буфера в VK_IMAGE_LAYOUT_PRESENT_SRC_KHR для представления содержимого
            commandBuffers_[i].endRenderPass();
            */

            /// Пост-обработка

            // Сменить целевой кадровый буфер и начать работу с проходом (это очистит вложения)
            renderPassBeginInfo.renderPass = renderPassPostProcess_.get();
            renderPassBeginInfo.framebuffer = frameBuffersPostProcess_[i].getVulkanFrameBuffer().get();
            commandBuffers_[i].beginRenderPass(renderPassBeginInfo,vk::SubpassContents::eInline);

            // Привязать графический конвейер
            commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eGraphics, pipelinePostProcess_.get());

            // Установка view-port'а и ножниц
            commandBuffers_[i].setViewport(0,1,&viewport);
            commandBuffers_[i].setScissor(0,1,&scissors);

            // Привязать набор дескрипторов камеры (матрицы вида и проекции)
            // TODO: здесь будет привязка дескриптора (передача изображения предыдущего прохода в качестве вложения)
//            commandBuffers_[i].bindDescriptorSets(
//                    vk::PipelineBindPoint::eGraphics,
//                    pipelineLayoutPostProcess_.get(),
//                    0,
//                    {},{});

            // Отрисовка треугольника
            commandBuffers_[i].draw(3,1,0,0);

            // Завершаем работать с потоком
            commandBuffers_[i].endRenderPass();

            // Завершаем работу с командным буфером
            commandBuffers_[i].end();
        }

        // Командные буфер готовы
        isCommandsReady_ = true;
    }

    // О Т П Р А В К А  К О М А Н Д  И  П О К А З

    // Индекс доступного изображения
    uint32_t availableImageIndex = 0;

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
    device_.getGraphicsQueue().submit({submitInfo}, nullptr);   // Отправка командного буфера на выполнение

    // Инициировать показ (когда картинка будет готова)
    vk::PresentInfoKHR presentInfoKhr{};
    presentInfoKhr.waitSemaphoreCount = signalSemaphores.size();             // Кол-во семафоров, которые будут ожидаться
    presentInfoKhr.pWaitSemaphores = signalSemaphores.data();                // Семафоры, которые ожидаются
    presentInfoKhr.swapchainCount = 1;                                       // Кол-во цепочек показа
    presentInfoKhr.pSwapchains = &(swapChainKhr_.get());                     // Цепочка показа
    presentInfoKhr.pImageIndices = &availableImageIndex;                     // Индекс показываемого изображения
    (void)device_.getPresentQueue().presentKHR(presentInfoKhr);              // Осуществить показ
}
