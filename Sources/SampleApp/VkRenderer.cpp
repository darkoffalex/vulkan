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
    depthStAttDesc.stencilLoadOp = vk::AttachmentLoadOp::eClear;            // Трафарет. Начало под-прохода - очищать
    depthStAttDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;       // Трафарет. Конец под-прохода - не важно
    depthStAttDesc.initialLayout = vk::ImageLayout::eUndefined;             // Макет памяти изображения в начале - не важно
    depthStAttDesc.finalLayout = vk::ImageLayout::eDepthAttachmentOptimal;  // Макет памяти изображения в конце - буфер глубины-трафарета
    attachmentDescriptions.push_back(depthStAttDesc);

    // Ссылки на вложения
    // Они содержат индексы описаний вложений, они также совместимы с порядком вложений в кадровом буфере, который привязывается во время начала прохода
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

    // Конфигурация swap-chain
    vk::SwapchainCreateInfoKHR swapChainCreateInfo{};
    swapChainCreateInfo.surface = surface_.get();
    swapChainCreateInfo.minImageCount = bufferCount;
    swapChainCreateInfo.imageFormat = surfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
    swapChainCreateInfo.imageExtent = capabilities.currentExtent;
    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = device_.isPresentAndGfxQueueFamilySame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
    swapChainCreateInfo.queueFamilyIndexCount = swapChainCreateInfo.imageSharingMode == vk::SharingMode::eConcurrent ? device_.getQueueFamilyIndices().size() : 0;
    swapChainCreateInfo.pQueueFamilyIndices = swapChainCreateInfo.imageSharingMode == vk::SharingMode::eConcurrent ? device_.getQueueFamilyIndices().data() : nullptr;
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
 * Инициализация UBO буферов
 * @param maxMeshes Максимальное кол-во одновременно отображающихся мешей
 */
void VkRenderer::initUboBuffers(size_t maxMeshes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize UBO buffers. Device not ready");
    }

    // Создаем UBO буферы для матриц вида и проекции
    uboBufferViewProjection_ = vk::tools::Buffer(
            &device_,
            sizeof(glm::mat4)*2,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);
}

/**
 * Де-инициализация UBO буферов
 */
void VkRenderer::deInitUboBuffers() noexcept
{
    // Уничтожаем ресурсы Vulkan
    uboBufferViewProjection_.destroyVulkanResources();
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
    // Проверить готовность буферов UBO
    if(!uboBufferViewProjection_.isReady()){
        throw vk::InitializationFailedError("Can't initialize descriptors. UBO buffers not ready");
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
        descriptorPoolCamera_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Создать пул для наборов мешей
    {
        // Размеры пула для наборов типа "материал меша"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Дескрипторы для буферов UBO (матрица модели)
                {vk::DescriptorType::eUniformBuffer,1},
                // Дескрипторы для буферов UBO (параметры материала)
                {vk::DescriptorType::eUniformBuffer,1},
                // Дескрипторы для текстур (пока-что одна albedo/diffuse текстура)
                {vk::DescriptorType::eCombinedImageSampler, 1}
        };

        // Поскольку у каждого меша есть свой набор, то кол-во таких наборов ограничено кол-вом мешей
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = maxMeshes;
        descriptorPoolMeshes_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Р А З М Е Щ Е Н И Я  Н А Б О Р О В  Д Е С К Р И П Т О Р О В
    // Макет размещения набора подробно описывает какие конкретно типы дескрипторов будут в наборе, сколько их, на каком
    // этапе конвейера они доступы, а так же какой у них индекс привязки (binding) в шейдере

    // Макет размещения набора камеры
    {
        // Описание привязок
        std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                // Обычный UBO буфер (для матриц вида и проекции)
                {
                        0,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eVertex,
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
                // UBO буфер для параметров материала
                {
                        1,
                        vk::DescriptorType::eUniformBuffer,
                        1,
                        vk::ShaderStageFlagBits::eFragment,
                        nullptr,
                },
                // Текстурный семплер
                {
                        2,
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
        descriptorSetLayoutMeshes_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
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
    }
	catch(std::exception&){}

    // Уничтожить размещения дескрипторов
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutCamera_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutMeshes_.get());
    descriptorSetLayoutCamera_.release();
    descriptorSetLayoutMeshes_.release();

    // Уничтожить пулы дескрипторов
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolCamera_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolMeshes_.get());
    descriptorPoolCamera_.release();
    descriptorPoolMeshes_.release();
}

/**
 * Инициализация графического конвейера
 * @param vertexShaderCodeBytes Код вершинного шейдера
 * @param fragmentShaderCodeBytes Код фрагментного шейдера
 */
void VkRenderer::initPipeline(
        const std::vector<unsigned char> &vertexShaderCodeBytes,
        const std::vector<unsigned char> &fragmentShaderCodeBytes)
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
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
            descriptorSetLayoutCamera_.get(),
            descriptorSetLayoutMeshes_.get()
    };

    // Создать макет размещения конвейера
    pipelineLayout_ = device_.getLogicalDevice()->createPipelineLayoutUnique({
        {},
        descriptorSetLayouts.size(),
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
//    vk::Rect2D scissors{};
//    scissors.offset.x = 0;
//    scissors.offset.y = 0;
//    scissors.extent.width = viewPortExtent.width;
//    scissors.extent.height = viewPortExtent.height;

    // Описываем кол-во областей вида и обрезки
    vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo{};
    pipelineViewportStateCreateInfo.viewportCount = 1;
    pipelineViewportStateCreateInfo.pViewports = &viewport;
//    pipelineViewportStateCreateInfo.scissorCount = 1;
//    pipelineViewportStateCreateInfo.pScissors = &scissors;
    pipelineViewportStateCreateInfo.scissorCount = 0;
    pipelineViewportStateCreateInfo.pScissors = nullptr;

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

    // Вернуть unique smart pointer
    pipeline_ = vk::UniquePipeline(pipeline);
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
 * @param maxMeshes Максимальное кол-во мешей
 */
VkRenderer::VkRenderer(HINSTANCE hInstance,
        HWND hWnd,
        const std::vector<unsigned char>& vertexShaderCodeBytes,
        const std::vector<unsigned char>& fragmentShaderCodeBytes,
        size_t maxMeshes):
isEnabled_(true),
isCommandsReady_(false),
inputDataInOpenGlStyle_(true)
{
    // Инициализация экземпляра Vulkan
    this->vulkanInstance_ = vk::tools::CreateVulkanInstance("My Application",
            "My engine",
            1,1,
            { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME},
            { "VK_LAYER_LUNARG_standard_validation"});
    std::cout << "Vulkan instance created." << std::endl;

    // Загрузка функций расширений (получение адресов)
    vkExtInitInstance(static_cast<VkInstance>(vulkanInstance_.get()));

    // Создание debug report callback'а (создается только в том случае, если расширение VK_EXT_DEBUG_REPORT_EXTENSION_NAME использовано)
    debugReportCallbackExt_ = vulkanInstance_->createDebugReportCallbackEXTUnique(vk::DebugReportCallbackCreateInfoEXT({vk::DebugReportFlagBitsEXT::eError|vk::DebugReportFlagBitsEXT::eWarning},vk::tools::DebugVulkanCallback));
    std::cout << "Report callback object created." << std::endl;

    // Создание поверхности отображения на окне
    this->surface_ = vulkanInstance_->createWin32SurfaceKHRUnique(vk::Win32SurfaceCreateInfoKHR({},hInstance,hWnd));
    std::cout << "Surface created." << std::endl;

    // Создание устройства
    device_ = vk::tools::Device(vulkanInstance_,surface_,{VK_KHR_SWAPCHAIN_EXTENSION_NAME},{"VK_LAYER_LUNARG_standard_validation"},false);
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

    // Выделение командных буферов
    // Командный буфер может быть и один, но в таком случае придется ожидать его выполнения перед тем, как начинать запись
    // в очередное изображение swap-chain'а (что не есть оптимально). Поэтому лучше использовать отдельные буферы, для каждого
    // изображения swap-chain'а (по сути это копии одного и того же буфера)
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(),vk::CommandBufferLevel::ePrimary,frameBuffers_.size());
    commandBuffers_ = device_.getLogicalDevice()->allocateCommandBuffers(allocInfo);
    std::cout << "Command-buffers allocated (" << commandBuffers_.size() << ")." << std::endl;

    // Выделение UBO буферов
    this->initUboBuffers(maxMeshes);
    std::cout << "UBO-buffer allocated (" << uboBufferViewProjection_.getSize() << ")." << std::endl;

    // Создать текстурный семплер по умолчанию
    textureSamplerDefault_ = vk::tools::CreateImageSampler(device_.getLogicalDevice().get(), vk::Filter::eLinear,vk::SamplerAddressMode::eRepeat, 2);
    std::cout << "Default texture sampler created." << std::endl;

    // Инициализация дескрипторных пулов и наборов
    this->initDescriptorPoolsAndLayouts(maxMeshes);
    std::cout << "Descriptor pool and layouts initialized." << std::endl;

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

    // Создать проход рендеринга
    this->initPipeline(vertexShaderCodeBytes,fragmentShaderCodeBytes);
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
    this->setRenderingStatus(false);

    // Удалить примитивы синхронизации
    device_.getLogicalDevice()->destroySemaphore(semaphoreReadyToRender_.get());
    device_.getLogicalDevice()->destroySemaphore(semaphoreReadyToPresent_.get());
    semaphoreReadyToRender_.release();
    semaphoreReadyToPresent_.release();
    std::cout << "Synchronization semaphores destroyed." << std::endl;

    // Уничтожение прохода рендеринга
    this->deInitPipeline();
    std::cout << "Graphics pipeline destroyed." << std::endl;

    // Очистить все ресурсы мешей
    this->freeMeshes();
    std::cout << "All allocated meshes data freed." << std::endl;

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

    // Освобождение UBO буферов
    this->deInitUboBuffers();
    std::cout << "UBO-buffers freed." << std::endl;

    // Освобождение командных буферов
    device_.getLogicalDevice()->freeCommandBuffers(device_.getCommandGfxPool().get(),commandBuffers_);
    commandBuffers_.clear();
    std::cout << "Command-buffers freed." << std::endl;

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

    // Уничтожение устройства
    device_.destroyVulkanResources();
    std::cout << "Device destroyed." << std::endl;

    // Уничтожение поверхности
    vulkanInstance_->destroySurfaceKHR(surface_.get());
    surface_.release();
    std::cout << "Surface destroyed." << std::endl;

    // Уничтожение debug report callback'а
    vulkanInstance_->destroyDebugReportCallbackEXT(debugReportCallbackExt_.get());
    debugReportCallbackExt_.release();
    std::cout << "Report callback object destroyed." << std::endl;

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

    // Де-инициализация кадровых буферов
    this->deInitFrameBuffers();
    std::cout << "Frame-buffers destroyed." << std::endl;

    // Ре-инициализация swap-chain (старый swap-chain уничтожается)
    this->initSwapChain({vk::Format::eB8G8R8A8Unorm,vk::ColorSpaceKHR::eSrgbNonlinear});
    std::cout << "Swap-chain re-created." << std::endl;

    // Инициализация кадровых буферов
    this->initFrameBuffers(vk::Format::eB8G8R8A8Unorm,vk::Format::eD32SfloatS8Uint);
    std::cout << "Frame-buffers initialized (" << frameBuffers_.size() << ") [" << frameBuffers_[0].getExtent().width << " x " << frameBuffers_[0].getExtent().height << "]" << std::endl;

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
vk::resources::GeometryBufferPtr VkRenderer::createGeometryBuffer(const std::vector<vk::tools::Vertex> &vertices, const std::vector<size_t> &indices)
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
 * @param sRgb Использовать цветовое пространство sRGB (гамма-коррекция)
 * @return Shared smart pointer на объект буфера
 */
vk::resources::TextureBufferPtr VkRenderer::createTextureBuffer(const unsigned char *imageBytes, size_t width, size_t height, size_t bpp, bool sRgb)
{
    auto buffer = std::make_shared<vk::resources::TextureBuffer>(&device_,&textureSamplerDefault_,imageBytes,width,height,bpp,sRgb);
    textureBuffers_.push_back(buffer);
    return buffer;
}

/**
 * Добавление меша на сцену
 * @param geometryBuffer Геометрический буфер
 * @param materialSettings Параметры материала меша
 * @return Shared smart pointer на объект меша
 */
vk::scene::MeshPtr VkRenderer::addMeshToScene(
        const vk::resources::GeometryBufferPtr& geometryBuffer,
        const vk::resources::TextureBufferPtr& textureBuffer,
        const vk::scene::MeshMaterialSettings& materialSettings)
{
    // Создание меша
    auto mesh = std::make_shared<vk::scene::Mesh>(&device_,descriptorPoolMeshes_,descriptorSetLayoutMeshes_,geometryBuffer, textureBuffer, materialSettings);

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

    // Удалям меш из списка
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
            commandBuffers_[i].setViewport(0,1,&viewport);

            // Привязать набор дескрипторов камеры (матрицы вида и проекции)
            commandBuffers_[i].bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    pipelineLayout_.get(),
                    0,
                    {getCameraPtr()->getDescriptorSet()},{});

            for(const auto& meshPtr : sceneMeshes_)
            {
                if(meshPtr->isReady() && meshPtr->getGeometryBuffer()->isReady())
                {
                    // Привязать наборы дескрипторов меша (матрица модели, свойства материала, текстуры и прочее)
                    commandBuffers_[i].bindDescriptorSets(
                            vk::PipelineBindPoint::eGraphics,
                            pipelineLayout_.get(),
                            1,
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
}