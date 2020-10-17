#include "VkRenderer.h"
#include "VkExtensionLoader/ExtensionLoader.h"

/**
 * Инициализация проходов рендеринга
 * @param colorAttachmentFormat Формат цветовых вложений
 * @param depthStencilAttachmentFormat Формат буфера глубины
 * @details Цветовые вложения - по сути изображения в которые шейдеры пишут информацию. У них могут быть форматы.
 * При создании проходов необходимо указать с каким форматом и размещением вложений происходит работа на конкретных этапах
 */
void VkRenderer::initRenderPasses(const vk::Format &colorAttachmentFormat, const vk::Format &depthStencilAttachmentFormat)
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
    mainRenderPass_ = device_.getLogicalDevice()->createRenderPassUnique(renderPassCreateInfo);
}

/**
 * Де-инициализация проходов рендеринга
 */
void VkRenderer::deInitRenderPasses() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить проход и освободить smart-pointer
    device_.getLogicalDevice()->destroyRenderPass(mainRenderPass_.get());
    mainRenderPass_.release();
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
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment|vk::ImageUsageFlagBits::eTransferDst;
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
void VkRenderer::initFrameBuffers(const vk::Format &colorAttachmentFormat, const vk::Format &depthStencilAttachmentFormat)
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
    if(!mainRenderPass_.get()){
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
        frameBuffers_.emplace_back(vk::resources::FrameBuffer(
                &device_,
                mainRenderPass_,
                {capabilities.currentExtent.width,capabilities.currentExtent.height,1},
                attachmentsInfo));
    }
}

/**
 * Де-инициализация кадровых буферов
 */
void VkRenderer::deInitFrameBuffers() noexcept
{
    // Очистка всех ресурсов Vulkan происходит в деструкторе объекта frame-buffer'а
    // Достаточно вызвать очистку массива
    frameBuffers_.clear();
}

/**
 * Инициализация изображения хранящего результат трассировки лучами сцены
 * @param colorAttachmentFormat Формат цветового вложения
 */
void VkRenderer::initRtOffscreenBuffer(const vk::Format &colorAttachmentFormat)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Device not ready");
    }
    // Проверяем готовность поверхности
    if(!surface_.get()){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Surface not ready");
    }

    // Получить возможности устройства для поверхности
    auto capabilities = device_.getPhysicalDevice().getSurfaceCapabilitiesKHR(surface_.get());

    // Создать изображение
    rtOffscreenBufferImage_ = vk::tools::Image(
            &device_,
            vk::ImageType::e2D,
            colorAttachmentFormat,
            {capabilities.currentExtent.width,capabilities.currentExtent.height,1},
            vk::ImageUsageFlagBits::eSampled|vk::ImageUsageFlagBits::eStorage|vk::ImageUsageFlagBits::eTransferSrc,
            vk::ImageAspectFlagBits::eColor,vk::MemoryPropertyFlagBits::eDeviceLocal,
            vk::SharingMode::eExclusive,vk::ImageLayout::ePreinitialized);

    // Перевести макет размещения изображения в general.
    // Именно такой layout нужно при использовании изображения в качестве storage дескриптора)

    // Выделить командный буфер для исполнения команды копирования
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.commandPool = device_.getCommandGfxPool().get();
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    auto cmdBuffers = device_.getLogicalDevice()->allocateCommandBuffers(commandBufferAllocateInfo);

    // Барьер памяти для перевода макета размещения
    vk::ImageMemoryBarrier rtOffscreenImageLayoutTransition{};
    rtOffscreenImageLayoutTransition.image = rtOffscreenBufferImage_.getVulkanImage().get();
    rtOffscreenImageLayoutTransition.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
    rtOffscreenImageLayoutTransition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    rtOffscreenImageLayoutTransition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    rtOffscreenImageLayoutTransition.oldLayout = vk::ImageLayout::ePreinitialized;
    rtOffscreenImageLayoutTransition.newLayout = vk::ImageLayout::eGeneral;
    rtOffscreenImageLayoutTransition.srcAccessMask = vk::AccessFlagBits::eMemoryWrite;
    rtOffscreenImageLayoutTransition.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;

    // Записать команду в буфер
    cmdBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,vk::PipelineStageFlagBits::eAllCommands,{},{},{},rtOffscreenImageLayoutTransition);
    cmdBuffers[0].end();

    // Отправить команду в очередь и подождать выполнения
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = cmdBuffers.size();
    submitInfo.pCommandBuffers = cmdBuffers.data();
    device_.getGraphicsQueue().submit({submitInfo},{});
    device_.getGraphicsQueue().waitIdle();
}

/**
 * Де-инициализация кадрового буфера для трассировки лучей
 */
void VkRenderer::deInitRtOffscreenBuffer() noexcept
{
	try
	{
        rtOffscreenBufferImage_.destroyVulkanResources();
	}
    catch (std::exception&) {}
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
        // Размеры пула для наборов типа "параметры камеры"
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

    // Создать пул для набора используемого в трассировке лучей
    {
        // Размеры пула для наборов типа "набор для трассировки лучей"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Дескриптор структуры ускорения верхнего уровня
                {vk::DescriptorType::eAccelerationStructureKHR, 1},
                // Дескриптор структуры итогового изображения (результат трассировки)
                {vk::DescriptorType::eStorageImage, 1},

                // Дескрипторы storage-буферов хранящих индексы (массив дескрипторов),
                {vk::DescriptorType::eStorageBuffer, static_cast<uint32_t>(maxMeshes)},
                // Дескрипторы storage-буферов хранящих вершины (массив дескрипторов)
                {vk::DescriptorType::eStorageBuffer, static_cast<uint32_t>(maxMeshes)},
                // Дескрипторы uniform-бферов хранящих матрицы трансфомацияя (массив дескрипторов)
                {vk::DescriptorType::eStorageBuffer, static_cast<uint32_t>(maxMeshes)}

        };

        // Нам нужен один набор данного типа, он будет привязываться единожды за кадр
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolRayTracing_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Создать пул для набора используемого для счетчика кадров
    {
        // Размеры пула для наборов типа "набор для счетчика кадров"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Дескриптор номера текущего кадра
                {vk::DescriptorType::eUniformBuffer, 1},
        };

        // Нам нужен один набор данного типа, он будет привязываться единожды за кадр
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolCreateInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
        descriptorPoolFrameCounter_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
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
                        vk::ShaderStageFlagBits::eVertex|vk::ShaderStageFlagBits::eFragment|vk::ShaderStageFlagBits::eClosestHitKHR|vk::ShaderStageFlagBits::eAnyHitKHR|vk::ShaderStageFlagBits::eRaygenKHR,
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
                        vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eAnyHitKHR | vk::ShaderStageFlagBits::eRaygenKHR,
                        nullptr,
                },
                // Обычный UBO буфер (для массива источников)
                {
                        1,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eClosestHitKHR | vk::ShaderStageFlagBits::eAnyHitKHR | vk::ShaderStageFlagBits::eRaygenKHR,
                        nullptr,
                },
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutLightSources_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // Макет размещения набора используемого в трассировке лучей
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // Структура ускорения верхнего уровня
                {
                        0,
                        vk::DescriptorType::eAccelerationStructureKHR,
                        1,
                        vk::ShaderStageFlagBits::eRaygenKHR|vk::ShaderStageFlagBits::eClosestHitKHR|vk::ShaderStageFlagBits::eAnyHitKHR,
                        nullptr,
                },
                // Хранимое изображение, результат трассировки
                {
                        1,
                        vk::DescriptorType::eStorageImage,
                        1,
                        vk::ShaderStageFlagBits::eRaygenKHR,
                        nullptr,
                },
                // Харнимый буфер индексов (массив дескрипторов)
                {
                    2,
                    vk::DescriptorType::eStorageBuffer,
                    static_cast<uint32_t>(maxMeshes),
                    vk::ShaderStageFlagBits::eRaygenKHR|vk::ShaderStageFlagBits::eClosestHitKHR|vk::ShaderStageFlagBits::eAnyHitKHR,
                    nullptr
                },
                // Харнимый буфер вершин (массив дескрипторов)
                {
                    3,
                    vk::DescriptorType::eStorageBuffer,
                    static_cast<uint32_t>(maxMeshes),
                    vk::ShaderStageFlagBits::eRaygenKHR|vk::ShaderStageFlagBits::eClosestHitKHR|vk::ShaderStageFlagBits::eAnyHitKHR,
                    nullptr
                },
                // Хранимый буфер матриц модели (массив дескрипторов)
                {
                    4,
                    vk::DescriptorType::eStorageBuffer,
                    static_cast<uint32_t>(maxMeshes),
                    vk::ShaderStageFlagBits::eRaygenKHR|vk::ShaderStageFlagBits::eClosestHitKHR|vk::ShaderStageFlagBits::eAnyHitKHR,
                    nullptr
                }
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutRayTracing_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // Макет размещения набора используемого для счетчика кадров
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // Буфер хранящий индекс текущего кадра
                {
                        0,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eRaygenKHR|vk::ShaderStageFlagBits::eClosestHitKHR|vk::ShaderStageFlagBits::eAnyHitKHR,
                        nullptr,
                },
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutFrameCounter_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
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
        device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolRayTracing_.get());
        device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolFrameCounter_.get());
    }
	catch(std::exception&){}

    // Уничтожить размещения дескрипторов
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutCamera_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutMeshes_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutLightSources_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutRayTracing_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutFrameCounter_.get());
    descriptorSetLayoutCamera_.release();
    descriptorSetLayoutMeshes_.release();
    descriptorSetLayoutLightSources_.release();
    descriptorSetLayoutRayTracing_.release();
    descriptorSetLayoutFrameCounter_.release();

    // Уничтожить пулы дескрипторов
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolCamera_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolMeshes_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolLightSources_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolRayTracing_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolFrameCounter_.get());
    descriptorPoolCamera_.release();
    descriptorPoolMeshes_.release();
    descriptorPoolLightSources_.release();
    descriptorPoolRayTracing_.release();
    descriptorPoolFrameCounter_.release();

    // Освободить объект дескрипторного набора для трассировки лучей
    // Сам дескрипторный набор был удален во время resetDescriptorPool соответствующего пула
    if(rtDescriptorSetReady_){
        rtDescriptorSet_.release();
    }
}

/**
 * Инициализация графического конвейера
 * @param vertexShaderCodeBytes Код вершинного шейдера
 * @param geometryShaderCodeBytes Код геометрического шейдера
 * @param fragmentShaderCodeBytes Код фрагментного шейдера
 */
void VkRenderer::initPipeline(
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& geometryShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Device not ready");
    }
    // Проверяем готовность основного прохода рендеринга
    if(!mainRenderPass_.get()){
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
    pipelineLayout_ = device_.getLogicalDevice()->createPipelineLayoutUnique({
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
    auto viewPortExtent = frameBuffers_[0].getExtent();

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
            vk::DynamicState::eViewport
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
    graphicsPipelineCreateInfo.layout = pipelineLayout_.get();
    graphicsPipelineCreateInfo.renderPass = mainRenderPass_.get();
    graphicsPipelineCreateInfo.subpass = 0;
    auto pipeline = device_.getLogicalDevice()->createGraphicsPipeline(nullptr,graphicsPipelineCreateInfo);

    // Уничтожить шейдерные модули (конвейер создан, они не нужны)
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleVs);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleFs);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleGs);

    // Вернуть unique smart pointer
    pipeline_ = vk::UniquePipeline(pipeline.value);
}

/**
 * Де-инициализация графического конвейера
 */
void VkRenderer::deInitPipeline() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить конвейер
    device_.getLogicalDevice()->destroyPipeline(pipeline_.get());
    pipeline_.release();

    // Уничтожить размещение конвейера
    device_.getLogicalDevice()->destroyPipelineLayout(pipelineLayout_.get());
    pipelineLayout_.release();
}

/**
 * Инициализация конвейера трассировки лучей
 * @param rayGenShaderCodeBytes Код шейдера генерации лучей
 * @param rayMissShaderCodeBytes Код шейдера промаха лучей
 * @param rayMissShadowShaderCodeBytes Код шейдера промаха луча для теней
 * @param rayHitShaderCodeBytes Код шейдера попадания луча
 */
void VkRenderer::initRtPipeline(
        const std::vector<unsigned char> &rayGenShaderCodeBytes,
        const std::vector<unsigned char> &rayMissShaderCodeBytes,
        const std::vector<unsigned char> &rayMissShadowShaderCodeBytes,
        const std::vector<unsigned char> &rayHitShaderCodeBytes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Device not ready");
    }

    // М А К Е Т  Р А З М Е Щ Е Н И Я  К О Н В Е Й Е Р А

    // Массив макетов размещения дескрипторов
    // ВНИМАНИЕ! Порядок следования наборов в шейдере (индексы дескрипторных наборов) зависит от порядка указания
    // макетов размещения в данном массиве.
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
            descriptorSetLayoutRayTracing_.get(),
            descriptorSetLayoutCamera_.get(),
            descriptorSetLayoutLightSources_.get(),
            descriptorSetLayoutFrameCounter_.get()
    };

    // Создать макет размещения конвейера
    rtPipelineLayout_ = device_.getLogicalDevice()->createPipelineLayoutUnique(
            {
                {},
                static_cast<uint32_t>(descriptorSetLayouts.size()),
                descriptorSetLayouts.data()
            });

    // Ш Е Й Д Е Р Ы ( П Р О Г Р А М И Р У Е М Ы Е  С Т А Д И И)

    // Убеждаемся что шейдерный код был предоставлен
    if(rayGenShaderCodeBytes.empty() || rayMissShaderCodeBytes.empty() || rayMissShadowShaderCodeBytes.empty() || rayHitShaderCodeBytes.empty()){
        throw vk::InitializationFailedError("No shader code provided");
    }

    // Шейдер генерации луча
    vk::ShaderModule shaderModuleRg = device_.getLogicalDevice()->createShaderModule({
            {},
            rayGenShaderCodeBytes.size(),
            reinterpret_cast<const uint32_t*>(rayGenShaderCodeBytes.data())});

    // Шейдер промаха луча
    vk::ShaderModule shaderModuleRm = device_.getLogicalDevice()->createShaderModule({
            {},
            rayMissShaderCodeBytes.size(),
            reinterpret_cast<const uint32_t*>(rayMissShaderCodeBytes.data())});

    // Шейдер промаха луча для теней
    vk::ShaderModule shaderModuleRms = device_.getLogicalDevice()->createShaderModule({
        {},
        rayMissShadowShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(rayMissShadowShaderCodeBytes.data())});

    // Шейдер попадания луча
    vk::ShaderModule shaderModuleRh = device_.getLogicalDevice()->createShaderModule({
            {},
            rayHitShaderCodeBytes.size(),
            reinterpret_cast<const uint32_t*>(rayHitShaderCodeBytes.data())});

    // Описываем стадии
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eRaygenKHR,shaderModuleRg,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eMissKHR,shaderModuleRm,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eMissKHR,shaderModuleRms,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eClosestHitKHR,shaderModuleRh,"main")
    };

    // Ш Е Й Д Е Р Н Ы Е  Г Р У П П Ы

    // Группа генерации лучей (ссылка на шейдерный модуль 0)
    rtShaderGroups_.emplace_back(vk::RayTracingShaderGroupTypeKHR::eGeneral,0,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR);
    // Группа промаха лучей (ссылка на шейдерный модуль 1)
    rtShaderGroups_.emplace_back(vk::RayTracingShaderGroupTypeKHR::eGeneral,1,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR);
    // Группа промаха лучей для теней
    rtShaderGroups_.emplace_back(vk::RayTracingShaderGroupTypeKHR::eGeneral,2,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR);
    // Группа попадания лучей (ближнее пересечение + любое пересечение)
    // Данную группу составляют шейдеры "ближайшего попадания","любого попадания","пересечения с геометрией"
    // Мы используем встроенный шейдер пересечения с треугольником для типа eTrianglesHitGroup, поэтому указывается только closestHit и anyHit (если нужно)
    rtShaderGroups_.emplace_back(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup,VK_SHADER_UNUSED_KHR,3,VK_SHADER_UNUSED_KHR,VK_SHADER_UNUSED_KHR);

    // О П И С А Н И Е  К О Н В Е Й Е Р А  Т Р А С С И Р О В К И

    vk::RayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfoKhr{};
    rayTracingPipelineCreateInfoKhr.setStageCount(shaderStages.size());
    rayTracingPipelineCreateInfoKhr.setPStages(shaderStages.data());
    rayTracingPipelineCreateInfoKhr.setGroupCount(rtShaderGroups_.size());
    rayTracingPipelineCreateInfoKhr.setPGroups(rtShaderGroups_.data());
    rayTracingPipelineCreateInfoKhr.setMaxRecursionDepth(2);
    rayTracingPipelineCreateInfoKhr.setLayout(rtPipelineLayout_.get());
    auto pipeline = device_.getLogicalDevice()->createRayTracingPipelineKHR(nullptr,rayTracingPipelineCreateInfoKhr);

    // Уничтожить шейдерные модули (конвейер создан, они не нужны)
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleRg);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleRm);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleRms);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleRh);

    // Получить указатель
    rtPipeline_ = vk::UniquePipeline(static_cast<vk::Pipeline&>(pipeline.value));
}

/**
 * Де-инициализация конвейера ray tracing
 */
void VkRenderer::deInitRtPipeline() noexcept
{
    // Проверяем готовность устройства
    assert(device_.isReady());

    // Уничтожить конвейер
    device_.getLogicalDevice()->destroyPipeline(rtPipeline_.get());
    rtPipeline_.release();

    // Уничтожить размещение конвейера
    device_.getLogicalDevice()->destroyPipelineLayout(rtPipelineLayout_.get());
    rtPipelineLayout_.release();
}

/**
 * Инициализация таблицы связи шейдеров
 * @details Таблица связей шейдеров описывает какие шейдеры будут срабатывать при промахе/генерации/пересечении луча
 * с геометрией какой-то конкретной hit-группы. По сути это схема процесса трассировки
 */
void VkRenderer::initRtShaderBindingTable()
{
    // Получить свойства трассировки у физического устройства
    auto properties = device_.getPhysicalDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR>();
    const auto& rtProperties = properties.get<vk::PhysicalDeviceRayTracingPropertiesKHR>();

    // Количество шейдерных групп (гурппы инициализируются на этапе инициализации конвейера трассировки)
    auto groupCount = static_cast<uint32_t>(rtShaderGroups_.size());
    // Размер единичного идентификатора группы
    uint32_t groupHandleSize = rtProperties.shaderGroupHandleSize;
    // Выравнивание (необходимо для выяснения размера буфера таблицы SBT)
    uint32_t baseAlignment = rtProperties.shaderGroupBaseAlignment;
    // Размер буфере таблицы SBT
    uint32_t sbtSize = groupCount * baseAlignment;

    // Массив байт - хранилище идентификаторов шейдерных групп
    std::vector<uint8_t> shaderGroupHandleStorage(sbtSize);
    // Получить шейдерные группы (заполнить shaderGroupHandleStorage)
    device_.getLogicalDevice()->getRayTracingShaderGroupHandlesKHR(rtPipeline_.get(),0,groupCount,sbtSize,shaderGroupHandleStorage.data());

    // Создать буфер для таблицы SBT
    rtSbtTableBuffer_ = vk::tools::Buffer(
            &device_,
            sbtSize,
            vk::BufferUsageFlagBits::eTransferSrc,
            vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

    // Скопировать данные из shaderGroupHandleStorage в буфер SBT
    void* pMappedSbt = rtSbtTableBuffer_.mapMemory(0);
    auto* pData  = reinterpret_cast<uint8_t*>(pMappedSbt);
    for(uint32_t g = 0; g < groupCount; g++){
        memcpy(pData, shaderGroupHandleStorage.data() + g * groupHandleSize, groupHandleSize);
        pData += baseAlignment;
    }
    rtSbtTableBuffer_.unmapMemory();
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
 * @param rayGenShaderCodeBytes Код шейдера генерации луча (байты)
 * @param rayMissShaderCodeBytes Код шейдера промаха луча (байты)
 * @param rayMissShadowShaderCodeBytes Код шейдера промаха луча для теней (байты)
 * @param rayHitShaderCodeBytes Rод шейдера попадания луча(байты)
 * @param maxMeshes Максимальное кол-во мешей
 */
VkRenderer::VkRenderer(HINSTANCE hInstance,
        HWND hWnd,
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& geometryShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes,
        const std::vector<unsigned char>& rayGenShaderCodeBytes,
        const std::vector<unsigned char>& rayMissShaderCodeBytes,
        const std::vector<unsigned char>& rayMissShadowShaderCodeBytes,
        const std::vector<unsigned char>& rayHitShaderCodeBytes,
        uint32_t maxMeshes):
isEnabled_(true),
isCommandsReady_(false),
inputDataInOpenGlStyle_(true),
useValidation_(true),
rtTopLevelAccelerationStructureReady_(false),
rtDescriptorSetReady_(false),
maxMeshes_(maxMeshes),
frameCounter_(0)
{
    // Инициализация экземпляра Vulkan
    std::vector<const char*> instanceExtensionNames = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
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
            VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
            VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME,
            VK_EXT_ROBUSTNESS_2_EXTENSION_NAME,

            // Расширения для аппаратной трассировки лучей
            VK_KHR_RAY_TRACING_EXTENSION_NAME,
            VK_KHR_MAINTENANCE3_EXTENSION_NAME,
            VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME,
            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME
    };

    std::vector<const char*> deviceValidationLayerNames = {};

    if(useValidation_){
        deviceValidationLayerNames.push_back("VK_LAYER_KHRONOS_validation");
    }

    device_ = vk::tools::Device(vulkanInstance_,surface_,deviceExtensionNames,deviceValidationLayerNames, false);
    std::cout << "Device initialized (" << device_.getPhysicalDevice().getProperties().deviceName << ")" << std::endl;

    // Инициализация прохода/проходов рендеринга
    this->initRenderPasses(vk::Format::eB8G8R8A8Unorm, vk::Format::eD32SfloatS8Uint);
    std::cout << "Render passes initialized." << std::endl;

    // Инициализация цепочки показа (swap-chain)
    this->initSwapChain({vk::Format::eB8G8R8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear});
    std::cout << "Swap-chain created." << std::endl;

    // Создание кадровых буферов
    this->initFrameBuffers(vk::Format::eB8G8R8A8Unorm,vk::Format::eD32SfloatS8Uint);
    std::cout << "Frame-buffers initialized (" << frameBuffers_.size() << ") [" << frameBuffers_[0].getExtent().width << " x " << frameBuffers_[0].getExtent().height << "]" << std::endl;

    // Создание буфера изображения для трассировки лучей
    this->initRtOffscreenBuffer(vk::Format::eB8G8R8A8Unorm);
    std::cout << "Ray tracing offscreen buffer initialized" << std::endl;

    // Выделение командных буферов
    // Командный буфер может быть и один, но в таком случае придется ожидать его выполнения перед тем, как начинать запись
    // в очередное изображение swap-chain'а (что не есть оптимально). Поэтому лучше использовать отдельные буферы, для каждого
    // изображения swap-chain'а (по сути это копии одного и того же буфера)
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(),vk::CommandBufferLevel::ePrimary,frameBuffers_.size());
    commandBuffers_ = device_.getLogicalDevice()->allocateCommandBuffers(allocInfo);
    std::cout << "Command-buffers allocated (" << commandBuffers_.size() << ")." << std::endl;

    // Создать текстурный семплер по умолчанию
    textureSamplerDefault_ = vk::tools::CreateImageSampler(device_.getLogicalDevice().get(), vk::Filter::eLinear,vk::SamplerAddressMode::eRepeat, 2);
    std::cout << "Default texture sampler created." << std::endl;

    // Инициализация дескрипторных пулов и наборов
    this->initDescriptorPoolsAndLayouts(maxMeshes_);
    std::cout << "Descriptor pool and layouts initialized." << std::endl;

    // Инициализация дескрипторного набора для счетчика кадров
    this->initFrameCounter();
    std::cout << "Frame counter initialized." << std::endl;

    // Создание камеры (UBO буферов и дескрипторных наборов)
    glm::float32 aspectRatio = static_cast<glm::float32>(frameBuffers_[0].getExtent().width) / static_cast<glm::float32>(frameBuffers_[0].getExtent().height);
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

    // Создать графический конвейер
    this->initPipeline(vertexShaderCodeBytes,geometryShaderCodeBytes,fragmentShaderCodeBytes);
    std::cout << "Graphics pipeline created." << std::endl;

	// Создать конвейер трассировки лучей
    this->initRtPipeline(rayGenShaderCodeBytes, rayMissShaderCodeBytes, rayMissShadowShaderCodeBytes, rayHitShaderCodeBytes);
    std::cout << "Ray tracing pipeline created." << std::endl;

    // Инициализировать буфер таблицы SBT
    this->initRtShaderBindingTable();
    std::cout << "Shader binding table buffer initialized." << std::endl;

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

    // Уничтожение буфера таблицы SBT
    rtSbtTableBuffer_.destroyVulkanResources();
    std::cout << "Shader binding table buffer destroyed." << std::endl;

	// Уничтожение конвейера трассировки лучей
    this->deInitRtPipeline();
    std::cout << "Ray tracing pipeline destroyed" << std::endl;
	
    // Уничтожение графического конвейера
    this->deInitPipeline();
    std::cout << "Graphics pipeline destroyed." << std::endl;

    // Очистить все ресурсы мешей
    this->freeMeshes();
    std::cout << "All allocated meshes data freed." << std::endl;

    // Очистить ресурсы набора источников света
    this->lightSourceSet_.destroyVulkanResources();
    std::cout << "Light source set destroyed." << std::endl;

    // Очистить ресурсы камеры
    this->camera_.destroyVulkanResources();
    std::cout << "Camera destroyed." << std::endl;

    // Де-инициализация кадрового счетчика
    this->deInitFrameCounter();
    std::cout << "Frame counter de-initialized." << std::endl;

    // Де-инициализация дескрипторов
    this->deInitDescriptorPoolsAndLayouts();
    std::cout << "Descriptor pool and layouts de-initialized." << std::endl;

    // Уничтожение текстурного семплера по умолчанию
    device_.getLogicalDevice()->destroySampler(textureSamplerDefault_.get());
    textureSamplerDefault_.release();
    std::cout << "Default texture sampler destroyed." << std::endl;

    // Освобождение командных буферов
    device_.getLogicalDevice()->freeCommandBuffers(device_.getCommandGfxPool().get(),commandBuffers_);
    commandBuffers_.clear();
    std::cout << "Command-buffers freed." << std::endl;

    // Уничтожения буфера изображения для трассировки лучей
    this->deInitRtOffscreenBuffer();
    std::cout << "Ray tracing offscreen buffer destroyed." << std::endl;

    // Уничтожение кадровых буферов
    this->deInitFrameBuffers();
    std::cout << "Frame-buffers destroyed." << std::endl;

    // Уничтожение цепочки показа (swap-chain)
    this->deInitSwapChain();
    std::cout << "Swap-chain destroyed." << std::endl;

    // Де-инициализация прохода
    this->deInitRenderPasses();
    std::cout << "Render pass destroyed." << std::endl;

    this->freeTextureBuffers();
    std::cout << "All allocated texture buffers freed." << std::endl;

    // Очистить все выделенные буферы геометрии
    this->freeGeometryBuffers();
    std::cout << "All allocated geometry buffers freed." << std::endl;

    // Очистить структуру ускорения верхнего уровня (если была создана)
    this->rtDeInitTopLevelAccelerationStructure();
    std::cout << "TLAS destroyed." << std::endl;

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

    // Де-инициализация дескрипторного набора для трассировки лучей
    this->rtDeInitDescriptorSet();
    std::cout << "Ray tracing descriptor set freed." << std::endl;

    // Де-инициализация изображения кадрового буфера для трассировки лучей
    this->rtOffscreenBufferImage_.destroyVulkanResources();
    std::cout << "Ray tracing offscreen buffer destroyed." << std::endl;

    // Де-инициализация кадровых буферов
    this->deInitFrameBuffers();
    std::cout << "Frame-buffers destroyed." << std::endl;

    // Ре-инициализация swap-chain (старый swap-chain уничтожается)
    this->initSwapChain({vk::Format::eB8G8R8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear});
    std::cout << "Swap-chain re-created." << std::endl;

    // Инициализация кадровых буферов
    this->initFrameBuffers(vk::Format::eB8G8R8A8Unorm,vk::Format::eD32SfloatS8Uint);
    std::cout << "Frame-buffers initialized (" << frameBuffers_.size() << ") [" << frameBuffers_[0].getExtent().width << " x " << frameBuffers_[0].getExtent().height << "]" << std::endl;

    // Инициализация изображения кадрового буфера для трассировки лучей
    this->initRtOffscreenBuffer(vk::Format::eB8G8R8A8Unorm);
    std::cout << "Ray tracing offscreen buffer initialized" << std::endl;

    // Пересоздание дескрипторного набора для трассировки лучей
    this->rtPrepareDescriptorSet();
    std::cout << "Ray tracing descriptor set re-created" << std::endl;

    // Изменить пропорции камеры
    camera_.setAspectRatio(static_cast<glm::float32>(frameBuffers_[0].getExtent().width)/static_cast<glm::float32>(frameBuffers_[0].getExtent().height));

    // Инициализация командных буферов
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(),vk::CommandBufferLevel::ePrimary,frameBuffers_.size());
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

            // Настройка view-port'а через команду (смена динамического состояния конвейера)
            auto viewPortExtent = frameBuffers_[0].getExtent();
            vk::Viewport viewport{};
            viewport.setX(0.0f);
            viewport.setWidth(static_cast<float>(viewPortExtent.width));
            viewport.setY(inputDataInOpenGlStyle_ ? static_cast<float>(viewPortExtent.height) : 0.0f);
            viewport.setHeight(inputDataInOpenGlStyle_ ? -static_cast<float>(viewPortExtent.height) : static_cast<float>(viewPortExtent.height));
            viewport.setMinDepth(0.0f);
            viewport.setMaxDepth(1.0f);
            commandBuffers_[i].setViewport(0,1,&viewport);

            // Привязать набор дескрипторов камеры (матрицы вида и проекции)
            commandBuffers_[i].bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipelineLayout_.get(),
                    0,
                    {camera_.getDescriptorSet(),lightSourceSet_.getDescriptorSet()},{});

            for(const auto& meshPtr : sceneMeshes_)
            {
                if(meshPtr->isReady() && meshPtr->getGeometryBuffer()->isReady())
                {
                    // Привязать наборы дескрипторов меша (матрица модели, свойства материала, текстуры и прочее)
                    commandBuffers_[i].bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics,
                            pipelineLayout_.get(),
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

    // О Б Н О В Л Е Н И Е  С Ч Е Т Ч И К А  К А Д Р О В

    if(frameCounter_ < UINT_MAX) frameCounter_++;
    else frameCounter_ = 0;
    this->updateFrameCounter();
}

/**
 * Рендеринг кадра (трассировка)
 */
void VkRenderer::raytrace()
{
    // Если рендеринг не включен - выход
    if(!isEnabled_){
        return;
    }

    // П О Д Г О Т О В К А  К О М А Н Д

    // Если командные буферы не готовы - заполнить их командами
    if(!isCommandsReady_)
    {
        // Получить свойства трассировки у физического устройства
        auto properties = device_.getPhysicalDevice().getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR>();
        const auto& rtProperties = properties.get<vk::PhysicalDeviceRayTracingPropertiesKHR>();

        // Смещения в убфере таблицы SBT
        vk::DeviceSize progSize = rtProperties.shaderGroupBaseAlignment; // Размер идентификатора программы
        vk::DeviceSize rayGenOffset     = 0u * progSize;                   // Генерация лучей
        vk::DeviceSize missOffset       = 1u * progSize;                   // Промах, промах для тени
        vk::DeviceSize hitGroupOffset   = 3u * progSize;                   // Hit-группа

        // Полный размер буфера таблицы SBT
        vk::DeviceSize sbtSize = progSize * static_cast<vk::DeviceSize>(rtShaderGroups_.size());

        // Подготовка команд (запись в командные буферы)
        for(size_t i = 0; i < commandBuffers_.size(); ++i)
        {
            // Т р а с с и р о в к а  с ц е н ы

            // Начинаем работу с командным буфером (запись команд)
            vk::CommandBufferBeginInfo commandBufferBeginInfo{};
            commandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
            commandBufferBeginInfo.pNext = nullptr;
            commandBuffers_[i].begin(commandBufferBeginInfo);

            // Привязать конвейер трассировки
            commandBuffers_[i].bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, rtPipeline_.get());
            // Привязать дескрипторный набор используемый в трассировке (TLAS + буфер изображения + параметры камеры)
            commandBuffers_[i].bindDescriptorSets(
                    vk::PipelineBindPoint::eRayTracingKHR,
                    rtPipelineLayout_.get(),
                    0,
                    {rtDescriptorSet_.get(),camera_.getDescriptorSet(),lightSourceSet_.getDescriptorSet(),frameCounterDescriptorSet_.get()},{});


            // Области буфера таблицы SBT
            const vk::StridedBufferRegionKHR rayGenShaderBindingTable = {rtSbtTableBuffer_.getBuffer().get(), rayGenOffset, progSize, sbtSize};
            const vk::StridedBufferRegionKHR missShaderBindingTable   = {rtSbtTableBuffer_.getBuffer().get(), missOffset, progSize, sbtSize};
            const vk::StridedBufferRegionKHR hitShaderBindingTable    = {rtSbtTableBuffer_.getBuffer().get(), hitGroupOffset, progSize, sbtSize};
            const vk::StridedBufferRegionKHR callableShaderBindingTable;

            // Размер изображения (должен совпадать с кадровым буфером, поэтому можно использовать его)
            auto width = frameBuffers_[i].getExtent().width;
            auto height = frameBuffers_[i].getExtent().height;

            // Трассировка сцены
            commandBuffers_[i].traceRaysKHR(&rayGenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,&callableShaderBindingTable,width,height,1);

            // К о п и р о в а н и е  в  и з о б р а ж е н и е  s w a p  - c h a i n

            // Превести макет размещения изображения swap-chain в vk::ImageLayout::eTransferDstOptimal (для копирования данных в него)
            vk::ImageMemoryBarrier swapChainImageLayoutTransition{};
            swapChainImageLayoutTransition.image = frameBuffers_[i].getAttachmentImages()[0].getVulkanImage().get();
            swapChainImageLayoutTransition.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            swapChainImageLayoutTransition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            swapChainImageLayoutTransition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            swapChainImageLayoutTransition.oldLayout = vk::ImageLayout::eUndefined;
            swapChainImageLayoutTransition.newLayout = vk::ImageLayout::eTransferDstOptimal;
            swapChainImageLayoutTransition.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            swapChainImageLayoutTransition.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
            commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,vk::PipelineStageFlagBits::eAllCommands,{},{},{},swapChainImageLayoutTransition);

            // Превести макет размещения изображения трассировки в vk::ImageLayout::eTransferSrcOptimal (для копирования данных из него)
            vk::ImageMemoryBarrier rtOffscreenImageLayoutTransition{};
            rtOffscreenImageLayoutTransition.image = rtOffscreenBufferImage_.getVulkanImage().get();
            rtOffscreenImageLayoutTransition.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            rtOffscreenImageLayoutTransition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            rtOffscreenImageLayoutTransition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            rtOffscreenImageLayoutTransition.oldLayout = vk::ImageLayout::eGeneral;
            rtOffscreenImageLayoutTransition.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            rtOffscreenImageLayoutTransition.srcAccessMask = vk::AccessFlagBits::eMemoryWrite;
            rtOffscreenImageLayoutTransition.dstAccessMask = vk::AccessFlagBits::eTransferRead;
            commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,vk::PipelineStageFlagBits::eAllCommands,{},{},{},rtOffscreenImageLayoutTransition);

            // Копировать изображение
            vk::ImageCopy copyRegion{};
            copyRegion.srcSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
            copyRegion.srcOffset = vk::Offset3D(0,0,0);
            copyRegion.dstSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1};
            copyRegion.dstOffset = vk::Offset3D(0,0,0);
            copyRegion.extent = vk::Extent3D(width,height,1);
            commandBuffers_[i].copyImage(
                    rtOffscreenBufferImage_.getVulkanImage().get(),vk::ImageLayout::eTransferSrcOptimal,
                    frameBuffers_[i].getAttachmentImages()[0].getVulkanImage().get(),vk::ImageLayout::eTransferDstOptimal,
                    {copyRegion});

            // Превести макет размещения изображения swap-chain в состояние для показа
            vk::ImageMemoryBarrier swapChainImageLayoutTransitionPresent{};
            swapChainImageLayoutTransitionPresent.image = frameBuffers_[i].getAttachmentImages()[0].getVulkanImage().get();
            swapChainImageLayoutTransitionPresent.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            swapChainImageLayoutTransitionPresent.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            swapChainImageLayoutTransitionPresent.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            swapChainImageLayoutTransitionPresent.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            swapChainImageLayoutTransitionPresent.newLayout = vk::ImageLayout::ePresentSrcKHR;
            swapChainImageLayoutTransitionPresent.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            swapChainImageLayoutTransitionPresent.dstAccessMask = vk::AccessFlagBits::eMemoryRead;
            commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eAllCommands, {}, {}, {}, swapChainImageLayoutTransitionPresent);

            // Превести макет размещения изображения трассировки в прежнее состояние (для записи в него)
            vk::ImageMemoryBarrier rtOffscreenImageLayoutTransitionBack{};
            rtOffscreenImageLayoutTransitionBack.image = rtOffscreenBufferImage_.getVulkanImage().get();
            rtOffscreenImageLayoutTransitionBack.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};
            rtOffscreenImageLayoutTransitionBack.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            rtOffscreenImageLayoutTransitionBack.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            rtOffscreenImageLayoutTransitionBack.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            rtOffscreenImageLayoutTransitionBack.newLayout = vk::ImageLayout::eGeneral;
            rtOffscreenImageLayoutTransitionBack.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            rtOffscreenImageLayoutTransitionBack.dstAccessMask = vk::AccessFlagBits::eMemoryWrite;
            commandBuffers_[i].pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,vk::PipelineStageFlagBits::eAllCommands,{},{},{},rtOffscreenImageLayoutTransitionBack);

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
    std::vector<vk::PipelineStageFlags> waitStages = {vk::PipelineStageFlagBits::eRayTracingShaderKHR};

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

    // О Б Н О В Л Е Н И Е  С Ч Е Т Ч И К А  К А Д Р О В

    if(frameCounter_ < UINT_MAX) frameCounter_++;
    else frameCounter_ = 0;
    this->updateFrameCounter();
}

/**
 * Построение структуры ускорения верхнего уровня
 * @param buildFlags Флаги построения структуры
 */
void VkRenderer::rtBuildTopLevelAccelerationStructure(const vk::BuildAccelerationStructureFlagsKHR &buildFlags)
{
    // Описание типа геометрии которая используется для построения TLAS
    // Структура верхнего уровня не использует геометрию, но хранит в себе набор instance'ов отдельных мешей
    // Каждый instance ссылается на соответствующий BLAS и обладает своей матрицей определяющей положение геометрии в пространстве
    std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR> geometryTypeInfos;
    vk::AccelerationStructureCreateGeometryTypeInfoKHR asCreateGeometryTypeInfo;
    asCreateGeometryTypeInfo.setGeometryType(vk::GeometryTypeKHR::eInstances);
    asCreateGeometryTypeInfo.setMaxPrimitiveCount(static_cast<uint32_t>(sceneMeshes_.size()));
    asCreateGeometryTypeInfo.setAllowsTransforms(VK_TRUE);
    geometryTypeInfos.push_back(asCreateGeometryTypeInfo);

    // Создание структуры ускорения
    {
        // 1 - Создать сам идентификатор структуры
        vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
        asCreateInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
        asCreateInfo.setFlags(buildFlags);
        asCreateInfo.setMaxGeometryCount(static_cast<uint32_t>(geometryTypeInfos.size()));
        asCreateInfo.setPGeometryInfos(geometryTypeInfos.data());
        rtTopLevelAccelerationStructureKhr_ = device_.getLogicalDevice()->createAccelerationStructureKHRUnique(asCreateInfo);

        // 2 - Получить требования к памяти
        vk::AccelerationStructureMemoryRequirementsInfoKHR memReqInfo{};
        memReqInfo.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice);
        memReqInfo.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject);
        memReqInfo.setAccelerationStructure(rtTopLevelAccelerationStructureKhr_.get());
        auto memReq = device_.getLogicalDevice()->getAccelerationStructureMemoryRequirementsKHR(memReqInfo);

        // 3 - Выделение памяти
        vk::MemoryAllocateInfo memoryAllocateInfo{};
        memoryAllocateInfo.setAllocationSize(memReq.memoryRequirements.size);
        memoryAllocateInfo.setMemoryTypeIndex(static_cast<uint32_t>(device_.getMemoryTypeIndex(memReq.memoryRequirements.memoryTypeBits,vk::MemoryPropertyFlagBits::eDeviceLocal)));
        rtTopLevelAccelerationStructureMemory_ = device_.getLogicalDevice()->allocateMemoryUnique(memoryAllocateInfo);

        // 4 - связать память и BLAS
        vk::BindAccelerationStructureMemoryInfoKHR bindInfo{};
        bindInfo.setAccelerationStructure(rtTopLevelAccelerationStructureKhr_.get());
        bindInfo.setMemory(rtTopLevelAccelerationStructureMemory_.get());
        bindInfo.setMemoryOffset(0);
        device_.getLogicalDevice()->bindAccelerationStructureMemoryKHR({bindInfo});
    }

    // Получить требования к памяти рабочего буфера (используемого для построения BLAS)
    vk::AccelerationStructureMemoryRequirementsInfoKHR memReqInfoScratch{};
    memReqInfoScratch.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice);
    memReqInfoScratch.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch);
    memReqInfoScratch.setAccelerationStructure(rtTopLevelAccelerationStructureKhr_.get());
    auto memReqScratch = device_.getLogicalDevice()->getAccelerationStructureMemoryRequirementsKHR(memReqInfoScratch);

    // Создать рабочий буфер
    vk::tools::Buffer scratchBuffer(&device_,
            memReqScratch.memoryRequirements.size,
            vk::BufferUsageFlagBits::eRayTracingKHR|vk::BufferUsageFlagBits::eShaderDeviceAddress,
            vk::MemoryPropertyFlagBits::eDeviceLocal);

    // Адрес рабочего буфера
    vk::DeviceAddress scratchBufferAddress = device_.getLogicalDevice()->getBufferAddress({scratchBuffer.getBuffer().get()});

    // Instances
    {
        // Набор instance'ов для каждого меша сцены
        std::vector<vk::AccelerationStructureInstanceKHR> instances;

        // Пройтись по добавленным мешам сцены и создать instance
        for(uint32_t i = 0; i < static_cast<uint32_t>(sceneMeshes_.size()); i++)
        {
            // Получить адрес структуры ускорения нижнего уровня (BLAS) у геометрического буфера меша
            auto blasAddress = device_.getLogicalDevice()->getAccelerationStructureAddressKHR({sceneMeshes_[i]->getGeometryBuffer()->getAccelerationStructure().get()});

            // Трансформация текущего instance
            vk::TransformMatrixKHR matrixKhr{};
            // Матрица, в отличии от матриц используемых в остальных частях приложения, не column-major, а row-major
            // Поэтому нужно транспонировать матрицу модели меша, перед использованием
            auto modelMatTranspose = glm::transpose(sceneMeshes_[i]->getModelMatrix());
            // TransformMatrixKHR хранит только 12 значений, соответствуя матрице to a 4x3
            // Поскольку последний ряд всегда (0,0,0,1), его можно не передавать.
            // Матрица row-major, и мы копируем первые 12 значений в matrixKhr из modelMatTranspose
            memcpy(reinterpret_cast<void*>(&matrixKhr), &modelMatTranspose, sizeof(vk::TransformMatrixKHR)); /// void??

            // Заполнить структуру описывающую одиночный instance
            vk::AccelerationStructureInstanceKHR instanceKhr{};
            instanceKhr.setTransform(matrixKhr);
            instanceKhr.setInstanceCustomIndex(i);
            instanceKhr.setMask(0xFF);
            instanceKhr.setInstanceShaderBindingTableRecordOffset(0); // Hit-группа
            instanceKhr.setFlags(vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable);
            instanceKhr.setAccelerationStructureReference(blasAddress);

            // Добавить в массив
            instances.push_back(instanceKhr);
        }

        // Теперь необходимо создать буфер instance'ов, который будет использован структурой и в котором будет информация массива instances
        // Буфер должен находится в памяти устройства, по этой причине используем временный буфер для переноса данных
        vk::DeviceSize bufferSize = instances.size() * sizeof(vk::AccelerationStructureInstanceKHR);

        // Создать временный буфер (память хоста)
        vk::tools::Buffer stagingInstanceBuffer = vk::tools::Buffer(&device_,
                bufferSize,
                vk::BufferUsageFlagBits::eTransferSrc,
                vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

        // Создать основной буфер (память устройства)
        rtTopLevelAccelerationStructureInstanceBuffer_ = vk::tools::Buffer(&device_,
                bufferSize,
                vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eRayTracingKHR|vk::BufferUsageFlagBits::eShaderDeviceAddress,
                vk::MemoryPropertyFlagBits::eDeviceLocal);

        // Заполнить временный буфер
        auto pStagingVertexBufferData = stagingInstanceBuffer.mapMemory(0,instances.size() * sizeof(vk::AccelerationStructureInstanceKHR));
        memcpy(pStagingVertexBufferData, reinterpret_cast<void*>(instances.data()),bufferSize);
        stagingInstanceBuffer.unmapMemory();

        // Копировать из временного буфера в основной
        device_.copyBuffer(stagingInstanceBuffer.getBuffer().get(),rtTopLevelAccelerationStructureInstanceBuffer_.getBuffer().get(),bufferSize);

        // Очищаем временный буфер (не обязательно, все равно очистится, но можно для ясности)
        stagingInstanceBuffer.destroyVulkanResources();
    }

    // Адрес буфера instance'ов
    vk::DeviceAddress instanceBufferAddress = device_.getLogicalDevice()->getBufferAddress({rtTopLevelAccelerationStructureInstanceBuffer_.getBuffer().get()});

    // Выделить командный буфер для исполнения команды построения
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
    commandBufferAllocateInfo.commandBufferCount = 1;
    commandBufferAllocateInfo.commandPool = device_.getCommandComputePool().get();
    commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    auto cmdBuffers = device_.getLogicalDevice()->allocateCommandBuffers(commandBufferAllocateInfo);

    // Начало записи команд в буфер
    cmdBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

    // Необходимо убедиться что буфер instance'ов был скопирован до начала построения TLAS
    // Для этого используем барьер
    vk::MemoryBarrier memoryBarrierInstanceBufferReady{};
    memoryBarrierInstanceBufferReady.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
    memoryBarrierInstanceBufferReady.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
    cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,{},{memoryBarrierInstanceBufferReady},{},{});

    // Информация для построения структуры
    vk::AccelerationStructureGeometryDataKHR geometryDataKhr{};
    geometryDataKhr.instances.setArrayOfPointers(VK_FALSE);
    geometryDataKhr.instances.data.setDeviceAddress(instanceBufferAddress);

    vk::AccelerationStructureGeometryKHR topAsGeometry{};
    topAsGeometry.setGeometryType(vk::GeometryTypeKHR::eInstances);
    topAsGeometry.setGeometry(geometryDataKhr);

    const vk::AccelerationStructureGeometryKHR* pGeometry = &topAsGeometry;
    vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo{};
    asBuildGeometryInfo.setType(vk::AccelerationStructureTypeKHR::eTopLevel);
    asBuildGeometryInfo.setFlags(buildFlags);
    asBuildGeometryInfo.setUpdate(VK_FALSE);
    asBuildGeometryInfo.setSrcAccelerationStructure(nullptr);
    asBuildGeometryInfo.setDstAccelerationStructure(rtTopLevelAccelerationStructureKhr_.get());
    asBuildGeometryInfo.setGeometryArrayOfPointers(VK_FALSE);
    asBuildGeometryInfo.setGeometryCount(1);
    asBuildGeometryInfo.setPpGeometries(&pGeometry);
    asBuildGeometryInfo.scratchData.setDeviceAddress(scratchBufferAddress);

    // Массив указателей на смещения
    vk::AccelerationStructureBuildOffsetInfoKHR buildOffsetInfoKhr{static_cast<uint32_t>(sceneMeshes_.size()),0,0,0};
    const vk::AccelerationStructureBuildOffsetInfoKHR* pBuildOffset = &buildOffsetInfoKhr;

    // Барьер памяти
    vk::MemoryBarrier memoryBarrier{};
    memoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
    memoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);

    // Запись команды построения TLAS, барьер и завершение
    cmdBuffers[0].buildAccelerationStructureKHR({asBuildGeometryInfo},pBuildOffset);
    cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,{},{memoryBarrier},{},{});
    cmdBuffers[0].end();

    // Отправить команду в очередь и подождать выполнения
    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = cmdBuffers.size();
    submitInfo.pCommandBuffers = cmdBuffers.data();
    device_.getComputeQueue().submit({submitInfo},{});
    device_.getComputeQueue().waitIdle();

    // Очистить рабочий буфер
    scratchBuffer.destroyVulkanResources();

    // Структура ускорения верхнего уровня готова к использованию
    rtTopLevelAccelerationStructureReady_ = true;
}

/**
 * Уничтожение структуры ускорения верхнего уровня (для трассировки лучей)
 */
void VkRenderer::rtDeInitTopLevelAccelerationStructure() noexcept
{
    if(rtTopLevelAccelerationStructureReady_)
    {
        // Поскольку функция может быть вызвана в деструкторе важно гарантировать отсутствие исключений
        try {
            device_.getLogicalDevice()->destroyAccelerationStructureKHR(rtTopLevelAccelerationStructureKhr_.get());
            rtTopLevelAccelerationStructureKhr_.release();

            device_.getLogicalDevice()->freeMemory(rtTopLevelAccelerationStructureMemory_.get());
            rtTopLevelAccelerationStructureMemory_.release();

            rtTopLevelAccelerationStructureInstanceBuffer_.destroyVulkanResources();
        }
        catch (std::exception&) {}
    }
}

/**
 * Подготовка дескрипторного набора для трассировки лучей
 */
void VkRenderer::rtPrepareDescriptorSet()
{
    if(!rtDescriptorSetReady_ && rtTopLevelAccelerationStructureReady_)
    {
        // Выделить дескрипторный набор
        vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
        descriptorSetAllocateInfo.descriptorPool = descriptorPoolRayTracing_.get();
        descriptorSetAllocateInfo.pSetLayouts = &(descriptorSetLayoutRayTracing_.get());
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        auto allocatedSets = device_.getLogicalDevice()->allocateDescriptorSets(descriptorSetAllocateInfo);
        rtDescriptorSet_ = vk::UniqueDescriptorSet(allocatedSets[0]);

        // Описываем связи дескрипторов с буферами (описание "записей")
        std::vector<vk::WriteDescriptorSet> writes = {};

        // T L A S

        vk::WriteDescriptorSetAccelerationStructureKHR accelerationStructureInfo{};
        accelerationStructureInfo.setAccelerationStructureCount(1);
        accelerationStructureInfo.setPAccelerationStructures(&(rtTopLevelAccelerationStructureKhr_.get()));

        vk::WriteDescriptorSet writeAs{};
        writeAs.setDstSet(rtDescriptorSet_.get());
        writeAs.setDstBinding(0);
        writeAs.setDstArrayElement(0);
        writeAs.setDescriptorCount(1);
        writeAs.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR);
        writeAs.setPNext(&accelerationStructureInfo);
        writes.push_back(writeAs);

        // И з о б р а ж е н и е - р е з у л ь т а т

        vk::DescriptorImageInfo offscreenImageInfo{};
        offscreenImageInfo.setImageView(rtOffscreenBufferImage_.getImageView().get());
        offscreenImageInfo.setImageLayout(vk::ImageLayout::eGeneral);

        vk::WriteDescriptorSet writeImg{};
        writeImg.setDstSet(rtDescriptorSet_.get());
        writeImg.setDstBinding(1);
        writeImg.setDstArrayElement(0);
        writeImg.setDescriptorCount(1);
        writeImg.setDescriptorType(vk::DescriptorType::eStorageImage);
        writeImg.setPImageInfo(&offscreenImageInfo);
        writes.push_back(writeImg);

        // Г е о м е т р и ч е с к и е  +  U B O  б у ф е р ы  м е ш е й

        std::vector<vk::DescriptorBufferInfo> indexBufferInfos;
        std::vector<vk::DescriptorBufferInfo> vertexBufferInfos;
        std::vector<vk::DescriptorBufferInfo> uboBufferInfos;

        // Пустышка (нулевой дескриптор)
        vk::DescriptorBufferInfo dummyBufferInfo{
                {},
                0,
                VK_WHOLE_SIZE
        };

        // Дабы избежать ошибки слоев валидации нужно обновлять ВСЕ дескрипторы (кол-во задано в descriptor set layout)
        // В данном случае кол-во равно максимальному количеству мешей
        for(uint32_t i = 0; i < maxMeshes_; i++)
        {
            // Если это реальный меш
            if(i < sceneMeshes_.size())
            {
                const auto& sceneMesh = sceneMeshes_[i];
                indexBufferInfos.emplace_back(
                        sceneMesh->getGeometryBuffer()->getIndexBuffer().getBuffer().get(),
                        0,
                        VK_WHOLE_SIZE);

                vertexBufferInfos.emplace_back(
                        sceneMesh->getGeometryBuffer()->getVertexBuffer().getBuffer().get(),
                        0,
                        VK_WHOLE_SIZE);

                uboBufferInfos.emplace_back(
                        sceneMesh->getModelMatrixUbo().getBuffer().get(),
                        0,
                        VK_WHOLE_SIZE);
            }
            // В противном случае использовать пустышку (null-descriptor)
            else
            {
                indexBufferInfos.push_back(dummyBufferInfo);
                vertexBufferInfos.push_back(dummyBufferInfo);
                uboBufferInfos.push_back(dummyBufferInfo);
            }
        }

        vk::WriteDescriptorSet writeIndexBuffers{};
        writeIndexBuffers.setDstSet(rtDescriptorSet_.get());
        writeIndexBuffers.setDstBinding(2);
        writeIndexBuffers.setDstArrayElement(0);
        writeIndexBuffers.setDescriptorCount(static_cast<uint32_t>(indexBufferInfos.size()));
        writeIndexBuffers.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        writeIndexBuffers.setPBufferInfo(indexBufferInfos.data());
        writes.push_back(writeIndexBuffers);

        vk::WriteDescriptorSet writeVertexBuffers{};
        writeVertexBuffers.setDstSet(rtDescriptorSet_.get());
        writeVertexBuffers.setDstBinding(3);
        writeVertexBuffers.setDstArrayElement(0);
        writeVertexBuffers.setDescriptorCount(static_cast<uint32_t>(vertexBufferInfos.size()));
        writeVertexBuffers.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        writeVertexBuffers.setPBufferInfo(vertexBufferInfos.data());
        writes.push_back(writeVertexBuffers);

        vk::WriteDescriptorSet writeUboBuffers{};
        writeUboBuffers.setDstSet(rtDescriptorSet_.get());
        writeUboBuffers.setDstBinding(4);
        writeUboBuffers.setDstArrayElement(0);
        writeUboBuffers.setDescriptorCount(static_cast<uint32_t>(uboBufferInfos.size()));
        writeUboBuffers.setDescriptorType(vk::DescriptorType::eStorageBuffer);
        writeUboBuffers.setPBufferInfo(uboBufferInfos.data());
        writes.push_back(writeUboBuffers);


        // Связываем дескрипторы с ресурсами
        device_.getLogicalDevice()->updateDescriptorSets(writes.size(), writes.data(), 0, nullptr);

        // Дескрипторный набор готов к использованию
        rtDescriptorSetReady_ = true;
    }
}

/**
 * Деинициализация дескрипторного набора
 */
void VkRenderer::rtDeInitDescriptorSet()
{
    if(rtDescriptorSetReady_)
    {
        device_.getLogicalDevice()->freeDescriptorSets(descriptorPoolRayTracing_.get(),{rtDescriptorSet_.get()});
        rtDescriptorSet_.release();
        rtDescriptorSetReady_ = false;
    }
}

/**
 * Инициализировать набор дескрипторов для счетчика кадров
 */
void VkRenderer::initFrameCounter()
{
    // Выделить дескрипторный набор
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.descriptorPool = descriptorPoolFrameCounter_.get();
    descriptorSetAllocateInfo.pSetLayouts = &(descriptorSetLayoutFrameCounter_.get());
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    auto fcSets = this->device_.getLogicalDevice()->allocateDescriptorSets(descriptorSetAllocateInfo);
    this->frameCounterDescriptorSet_ = vk::UniqueDescriptorSet(fcSets[0]);

    // Создать буфер UBO для счетчика кадров
    frameCounterUbo_ = vk::tools::Buffer(
            &device_,
            sizeof(glm::uint32_t),
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

    // Разметить память и получить указатель
    pFrameCounterUboData_ = frameCounterUbo_.mapMemory(0, VK_WHOLE_SIZE);

    // Информация о буфере
    vk::DescriptorBufferInfo frameCounterBufferInfo = {
            frameCounterUbo_.
            getBuffer().get(),
            0,
            VK_WHOLE_SIZE};

    // Описываем связь дескриптора с буфером (описание "записи")
    vk::WriteDescriptorSet writeDescriptorSet(
            frameCounterDescriptorSet_.get(),
            0,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &frameCounterBufferInfo,
            nullptr
    );

    // Связываем дескрипторы с ресурсами
    device_.getLogicalDevice()->updateDescriptorSets(1,&writeDescriptorSet,0, nullptr);
}

/**
 * Обновление кадрового буфера
 */
void VkRenderer::updateFrameCounter()
{
    memcpy(pFrameCounterUboData_,&frameCounter_,sizeof(glm::uint32_t));

    /*
    std::vector<vk::MappedMemoryRange> ranges = {
            {frameCounterUbo_.getMemory().get(),0,sizeof(glm::uint32_t)}
    };
    device_.getLogicalDevice()->flushMappedMemoryRanges(ranges);
    */
}

/**
 * Деинициализация набора дескрипторов для счетчика кадров
 */
void VkRenderer::deInitFrameCounter()
{
    // Вернуть дескрипторный набор в пул
    device_.getLogicalDevice()->freeDescriptorSets(
            descriptorPoolFrameCounter_.get(),
            {frameCounterDescriptorSet_.get()});

    // Освободить объект
    frameCounterDescriptorSet_.release();

    // Уничтожить буфер
    frameCounterUbo_.unmapMemory();
    frameCounterUbo_.destroyVulkanResources();
}
