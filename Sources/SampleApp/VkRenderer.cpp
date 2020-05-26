#include "VkRenderer.h"
#include "ext_loader/vulkan_ext.h"

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
    if(surface_.get() == nullptr){
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
void VkRenderer::deInitRenderPasses()
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't de-init render pass. Device not ready");
    }

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
    if(surface_.get() == nullptr){
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
    auto oldSwapChain = swapChainKhr_.get() != nullptr ? swapChainKhr_.get() : nullptr;

    // Создать swap-chain
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
    swapChainKhr_ = device_.getLogicalDevice()->createSwapchainKHRUnique(swapChainCreateInfo);

    // Уничтожить старый swap-chain если он был
    if(oldSwapChain != nullptr){
        device_.getLogicalDevice()->destroySwapchainKHR(oldSwapChain);
    }
}

/**
 * Де-инициализация swap-chain (цепочки показа)
 */
void VkRenderer::deInitSwapChain()
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't de-init swap-chain. Device not ready");
    }

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
    if(surface_.get() == nullptr){
        throw vk::InitializationFailedError("Can't initialize frame buffers. Surface not ready");
    }
    // Проверяем готовность swap-chain'а
    if(swapChainKhr_.get() == nullptr){
        throw vk::InitializationFailedError("Can't initialize frame-buffers. Swap-chain not ready");
    }
    // Проверяем готовность прохода для которого создаются буферы
    if(mainRenderPass_.get() == nullptr){
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
        std::vector<vk::tools::FrameBufferAttachmentInfo> attachmentsInfo = {
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
        frameBuffers_.emplace_back(vk::tools::FrameBuffer(
                &device_,
                mainRenderPass_,
                {capabilities.currentExtent.width,capabilities.currentExtent.height,1},
                attachmentsInfo));
    }
}

/**
 * Де-инициализация кадровых буферов
 */
void VkRenderer::deInitFrameBuffers()
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

    // Создаем UBO буферы для матриц модели (на каждый меш своя матрица)
    // В связи с тем что тип дескриптора предполагается как "динамический буфер", используем динамическое выравнивание для выяснения размеров буфера
    uboBufferModel_ = vk::tools::Buffer(
            &device_,
            device_.getDynamicallyAlignedUboBlockSize<glm::mat4>() * maxMeshes,
            vk::BufferUsageFlagBits::eUniformBuffer,
            vk::MemoryPropertyFlagBits::eHostVisible);
}

/**
 * Де-инициализация UBO буферов
 */
void VkRenderer::deInitUboBuffers()
{
    // Уничтожаем ресурсы Vulkan
    uboBufferModel_.destroyVulkanResources();
    uboBufferViewProjection_.destroyVulkanResources();
}

/**
 * Инициализация дескрипторов (наборов дескрипторов)
 * @param maxMeshes Максимальное кол-во одновременно отображающихся мешей (влияет на максимальное кол-во наборов для материала меша и прочего)
 */
void VkRenderer::initDescriptors(size_t maxMeshes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize descriptors. Device not ready");
    }
    // Проверить готовность буферов UBO
    if(!uboBufferModel_.isReady() || !uboBufferViewProjection_.isReady()){
        throw vk::InitializationFailedError("Can't initialize descriptors. UBO buffers not ready");
    }

    // Д Е С К Р И П Т О Р Н Ы Е  П У Л Ы
    // Наборы дескрипторов выделяются из пулов. При создании пула, необходимо знать какие дескрипторы и сколько их
    // будут в наборах, которые будут выделяться из пула, а так же сколько таких наборов всего будет

    // Создать пул для наборов типа "UBO"
    {
        // Размеры пула для наборов типа "UBO"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Один дескриптор в наборе отвечает за обычный UBO буфер (привязывается единожды за кадр)
                {vk::DescriptorType::eUniformBuffer, 1},
                // Один дескриптор в наборе отвечает за динамический UBO буфер (может привязываться несколько раз за кадр со смещением в буфере)
                {vk::DescriptorType::eUniformBufferDynamic, 1},
        };


        // У набора UBO есть динамический дескриптор для матриц модели, что позволяет многократно привязывать один набор
        // с динамическим смещением. Это позволяет использовать только один набор для всех матриц мешей
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        descriptorPoolUBO_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Создать для наборов типа "материал меша"
    {
        // Размеры пула для наборов типа "материал меша"
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = {
                // Один дескриптор в наборе отвечает за текстуру совмещенную с текстурным семплером
                {vk::DescriptorType::eCombinedImageSampler, 1}
        };

        // У данного набора нет динамических дескрипторов, и для данного набора нельзя использовать динамическое смещение,
        // соответственно у каждого меша должен быть свой набор дескрипторов отвечающий за метериал (текстуры и прочее)
        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = maxMeshes;
        descriptorPoolMeshMaterial_ = device_.getLogicalDevice()->createDescriptorPoolUnique(descriptorPoolCreateInfo);
    }

    // Р А З М Е Щ Е Н И Я  Н А Б О Р О В  Д Е С К Р И П Т О Р О В
    // Макет размещения набора подробно описывает какие конкретно типы дескрипторов будут в наборе, сколько их, на каком
    // этапе конвейера они доступы, а так же какой у них индекс привязки (binding) в шейдере

    // Макет размещения набора UBO
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
                // Динамический UBO буфер (для матриц модели)
                // Привязывая набор с этим буфером можно указать динамическое смещение
                {
                        1,
                        vk::DescriptorType::eUniformBufferDynamic,
                        1,
                        vk::ShaderStageFlagBits::eVertex,
                        nullptr,
                },
        };

        // Создать макет размещения дескрипторного набора
        vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        descriptorSetLayoutUBO_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // Макет размещения материала меша
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
        descriptorSetLayoutMeshMaterial_ = device_.getLogicalDevice()->createDescriptorSetLayoutUnique(descriptorSetLayoutCreateInfo);
    }

    // U B O  Н А Б О Р
    // В отличии от набора материала меша, который будет выделяться отдельно для каждого меше, набор UBO можно
    // выделить уже сейчас, ибо это единственный набор.

    // Выделить из пула набор дескрипторов
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
    descriptorSetAllocateInfo.descriptorPool = descriptorPoolUBO_.get(),
    descriptorSetAllocateInfo.pSetLayouts = &(descriptorSetLayoutUBO_.get());
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    auto allocatedSets = device_.getLogicalDevice()->allocateDescriptorSets(descriptorSetAllocateInfo);

    // Информация о буферах
    std::vector<vk::DescriptorBufferInfo> bufferInfos = {
            {uboBufferViewProjection_.getBuffer().get(),0,VK_WHOLE_SIZE},
            {uboBufferModel_.getBuffer().get(),0,VK_WHOLE_SIZE}
    };

    // Описываем связи дескрипторов с буферами (описание "записей")
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

    // Связываем дескрипторы с ресурсами (буферами)
    device_.getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

    // Сохранить созданный набор
    descriptorSetUBO_ = vk::UniqueDescriptorSet(allocatedSets[0]);
}

/**
 * Де-инициализация дескрипторов
 */
void VkRenderer::deInitDescriptors()
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't de-init descriptors. Device not ready");
    }

    // Уничтожить все выделенные наборы дескрипторов
    device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolUBO_.get());
    device_.getLogicalDevice()->resetDescriptorPool(descriptorPoolMeshMaterial_.get());
    descriptorSetUBO_.release();

    // Уничтожить размещения дескрипторов
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutUBO_.get());
    device_.getLogicalDevice()->destroyDescriptorSetLayout(descriptorSetLayoutMeshMaterial_.get());
    descriptorSetLayoutUBO_.release();
    descriptorSetLayoutMeshMaterial_.release();

    // Уничтожить пулы дескрипторов
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolUBO_.get());
    device_.getLogicalDevice()->destroyDescriptorPool(descriptorPoolMeshMaterial_.get());
    descriptorPoolUBO_.release();
    descriptorPoolMeshMaterial_.release();
}

/**
 * Инициализация графического конвейера
 * @param vertexShaderCodeBytes Код вершинного шейдера
 * @param fragmentShaderCodeBytes Код фрагментного шейдера
 */
void VkRenderer::initPipeline(const std::vector<unsigned char> &vertexShaderCodeBytes, const std::vector<unsigned char> &fragmentShaderCodeBytes)
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't initialize pipeline. Device not ready");
    }
    // Проверяем готовность основного прохода рендеринга
    if(mainRenderPass_.get() == nullptr){
        throw vk::InitializationFailedError("Can't initialize pipeline. Render pass not ready");
    }

    // М А К Е Т  Р А З М Е Щ Е Н И Я  К О Н В Е Й Е Р А

    // Массив макетов размещения дескрипторов
    std::vector<vk::DescriptorSetLayout> descriptorSetLayouts = {
            descriptorSetLayoutUBO_.get(),
            descriptorSetLayoutMeshMaterial_.get()
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
    vk::ShaderModule shaderModuleVS = device_.getLogicalDevice()->createShaderModule({
        {},
        vertexShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(vertexShaderCodeBytes.data())});

    // Фрагментный шейдер
    vk::ShaderModule shaderModuleFS = device_.getLogicalDevice()->createShaderModule({
        {},
        fragmentShaderCodeBytes.size(),
        reinterpret_cast<const uint32_t*>(fragmentShaderCodeBytes.data())});

    // Описываем стадии
    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eVertex,shaderModuleVS,"main"),
            vk::PipelineShaderStageCreateInfo({},vk::ShaderStageFlagBits::eFragment,shaderModuleFS,"main")
    };

    // V I E W  P O R T  &  S C I S S O R S

    // Получить разрешение view-port'а
    auto viewPortExtent = frameBuffers_[0].getExtent();

    // Настройки области отображения
    vk::Viewport viewport{};
    viewport.setX(0.0f);
    viewport.setY(0.0f);
    viewport.setWidth(viewPortExtent.width);
    viewport.setHeight(viewPortExtent.height);
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
    graphicsPipelineCreateInfo.layout = pipelineLayout_.get();
    graphicsPipelineCreateInfo.renderPass = mainRenderPass_.get();
    graphicsPipelineCreateInfo.subpass = 0;
    auto pipeline = device_.getLogicalDevice()->createGraphicsPipeline(nullptr,graphicsPipelineCreateInfo);

    // Уничтожить шейдерные модули (конвейер создан, они не нужны)
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleVS);
    device_.getLogicalDevice()->destroyShaderModule(shaderModuleFS);

    // Вернуть unique smart pointer
    pipeline_ = vk::UniquePipeline(pipeline);
}

/**
 * Де-инициализация графического конвейера
 */
void VkRenderer::deInitPipeline()
{
    // Проверяем готовность устройства
    if(!device_.isReady()){
        throw vk::InitializationFailedError("Can't de-init pipeline. Device not ready");
    }

    // Уничтожить конвейер
    device_.getLogicalDevice()->destroyPipeline(pipeline_.get());
    pipeline_.release();

    // Уничтожить размещение конвейера
    device_.getLogicalDevice()->destroyPipelineLayout(pipelineLayout_.get());
    pipelineLayout_.release();
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
isCommandsReady_(false)
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
    std::cout << "Frame-buffers initialized (" << frameBuffers_.size() << ")." << std::endl;

    // Выделение командных буферов
    // Командный буфер может быть и один, но в таком случае придется ожидать его выполнения перед тем, как начинать запись
    // в очередное изображение swap-chain'а (что не есть оптимально). Поэтому лучше использовать отдельные буферы, для каждого
    // изображения swap-chain'а (по сути это копии одного и того же буфера)
    auto allocInfo = vk::CommandBufferAllocateInfo(device_.getCommandGfxPool().get(),vk::CommandBufferLevel::ePrimary,frameBuffers_.size());
    commandBuffers_ = device_.getLogicalDevice()->allocateCommandBuffers(allocInfo);
    std::cout << "Command-buffers allocated (" << commandBuffers_.size() << ")." << std::endl;

    // Выделение UBO буферов
    this->initUboBuffers(maxMeshes);
    std::cout << "UBO-buffers allocated (" << uboBufferViewProjection_.getSize() << " and " << uboBufferModel_.getSize() << ")." << std::endl;

    // Создать текстурный семплер по умолчанию
    textureSamplerDefault_ = vk::tools::CreateImageSampler(device_.getLogicalDevice().get(), vk::Filter::eLinear,vk::SamplerAddressMode::eRepeat, 4);
    std::cout << "Default texture sampler created." << std::endl;

    // Инициализация дескрипторных пулов и наборов
    this->initDescriptors(maxMeshes);
    std::cout << "Descriptors initialized." << std::endl;

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

    // Де-инициализация дескрипторов
    this->deInitDescriptors();
    std::cout << "Descriptors de-initialized" << std::endl;

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
void VkRenderer::setRenderingStatus(bool isEnabled)
{
    // Если мы выключаем рендеринг
    if(!isEnabled && isEnabled_){
        // Ожидаем завершения всех команд
        device_.getLogicalDevice()->waitIdle();
    }

    // Сменить статус
    isEnabled_ = isEnabled;
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
