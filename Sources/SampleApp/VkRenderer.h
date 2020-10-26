#pragma once

#include "VkTools/Tools.h"
#include "VkTools/Device.hpp"
#include "VkTools/Buffer.hpp"

#include "VkResources/FrameBuffer.hpp"
#include "VkResources/GeometryBuffer.hpp"

#include "VkScene/Mesh.h"
#include "VkScene/Camera.h"
#include "VkScene/LightSourceSet.hpp"

class VkRenderer
{
private:
    /// Запущен ли рендеринг
    bool isEnabled_;
    /// Готовы ли командные буферы
    bool isCommandsReady_;
    /// Данные на вход (о вершинах) подаются в стиле OpenGL (считая что начало view-port'а в нижнем левом углу)
    bool inputDataInOpenGlStyle_;
    /// Использовать validation-слои и report callback
    bool useValidation_;

    /// Экземпляр Vulkan (smart pointer)
    vk::UniqueInstance vulkanInstance_;
    /// Идентификатор объекта для работы с debug-callback'ом
    vk::UniqueDebugReportCallbackEXT debugReportCallbackExt_;
    /// Поверхность для рисования на окне (smart pointer)
    vk::UniqueSurfaceKHR surface_;
    /// Устройство
    vk::tools::Device device_;
    /// Проход рендеринга - первичный
    vk::UniqueRenderPass renderPassPrimary_;
    /// Проход рендеринга - пост-процессинг
    vk::UniqueRenderPass renderPassPostProcess_;
    /// Объект очереди показа (swap-chain)
    vk::UniqueSwapchainKHR swapChainKhr_;

    /// Кадровые буферы - основные
    std::vector<vk::resources::FrameBuffer> frameBuffersPrimary_;
    /// Кадровые буферы - пост-обработка
    std::vector<vk::resources::FrameBuffer> frameBuffersPostProcess_;
    /// Кадровые буферы - дескрипторные наборы изображений (для передачи в шейдер другого этапа)
    std::vector<vk::DescriptorSet> frameBuffersPrimaryDescriptorSets_;

    /// Командные буферы
    std::vector<vk::CommandBuffer> commandBuffers_;

    /// Текстурный семплер по умолчанию, используемый для всех создаваемых текстур
    vk::UniqueSampler textureSamplerDefault_;

    /// Объект дескрипторного пула для выделения набора для камеры (матрицы)
    vk::UniqueDescriptorPool descriptorPoolCamera_;
    /// Объект дескрипторного пула для выделения наборов мешей (матрицы, материалы, текстуры)
    vk::UniqueDescriptorPool descriptorPoolMeshes_;
    /// Объект дескрипторного пула для выделения набора для источников света
    vk::UniqueDescriptorPool descriptorPoolLightSources_;
    /// Объект дескрипторного пула для выделения набора использумого при пост-процессинге
    vk::UniqueDescriptorPool descriptorPoolImagesToPostProcess_;

    /// Макет размещения дескрипторного набора для камеры (матрицы)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutCamera_;
    /// Макет размещения дескрипторного набора для меша (матрицы, материалы, текстуры)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutMeshes_;
    /// Макет размещения дескрипторного набора для источников света (кол-во, массив источников)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutLightSources_;
    /// Макет размещения дескрипторного набора использумого при пост-процессинге
    vk::UniqueDescriptorSetLayout descriptorSetLayoutImagesToPostProcess_;

    /// Макет размещения графического конвейера - основной
    vk::UniquePipelineLayout pipelineLayoutPrimary_;
    /// Макет размещения графического конвейера - пост-процессинг
    vk::UniquePipelineLayout pipelineLayoutPostProcess_;
    /// Графический конвейер - основной
    vk::UniquePipeline pipelinePrimary_;
    /// Графический конвейер - пост-процессинг
    vk::UniquePipeline pipelinePostProcess_;

    /// Примитивы синхронизации - семафор сигнализирующий о готовности к рендерингу
    vk::UniqueSemaphore semaphoreReadyToRender_;
    /// Примитивы синхронизации - семафор сигнализирующий о готовности к показу отрендереной картинки
    vk::UniqueSemaphore semaphoreReadyToPresent_;

    /// Массив указателей на выделенные геометрические буферы
    std::vector<vk::resources::GeometryBufferPtr> geometryBuffers_;
    /// Массив указателей на выделенные текстурные буферы
    std::vector<vk::resources::TextureBufferPtr> textureBuffers_;
    /// Массив указателей мешей сцены
    std::vector<vk::scene::MeshPtr> sceneMeshes_;
    /// Камера (матрицы, UBO)
    vk::scene::Camera camera_;
    /// Источники освещения сцены
    vk::scene::LightSourceSet lightSourceSet_;

    /// Ресурсы по умолчанию - текстуры
    vk::resources::TextureBufferPtr blackPixelTexture_;


    /**
     * Инициализация основного прохода рендеринга
     * @param colorAttachmentFormat Формат цветовых вложений
     * @param depthStencilAttachmentFormat Формат вложений глубины
     * @details Цветовые вложения - по сути изображения в которые шейдеры пишут информацию. У них могут быть форматы.
     * При создании проходов необходимо указать с каким форматом и размещением вложений происходит работа на конкретных этапах
     */
    void initRenderPassPrimary(const vk::Format& colorAttachmentFormat, const vk::Format& depthStencilAttachmentFormat);

    /**
     * Де-инициализация основного прохода рендеринга
     */
    void deInitRenderPassPrimary() noexcept;

    /**
     * Инициализация прохода для пост-обработки
     * @param colorAttachmentFormat Формат цветовых вложений
     * @details В отличии от основного прохода, проход пост-обработки пишет только в цветовое вложение
     */
    void initRenderPassPostProcess(const vk::Format& colorAttachmentFormat);

    /**
     * Де-инициализация прохода пост-обработки
     */
    void deInitRenderPassPostProcess() noexcept;

    /**
     * Инициализация swap-chain (цепочки показа) - набор сменяющихся изображений показываемых на поверхности отображения
     * @param surfaceFormat Формат поверхности
     * @param bufferCount Кол-ва запрашиваемых буферов (уровень желаемой буферизации, 0 - определить автоматически)
     * @return Фактическое количество буферов (сколько будет использовано)
     */
    void initSwapChain(const vk::SurfaceFormatKHR& surfaceFormat, uint32_t bufferCount = 0);

    /**
     * Де-инициализация swap-chain (цепочки показа)
     */
    void deInitSwapChain() noexcept;


    /**
     * Инициализация основноых кадровых буферов
     * @param colorAttachmentFormat Формат цветовых вложений
     * @param depthStencilAttachmentFormat Формат вложений глубины
     *
     * @details Несмотря на то, что кадровые буферы чисто логически не обязательно должны быть связаны с изображениями swap-chain'а,
     * если мы используем показ, то так или иначе, осуществляем запись в изображения swap-chain'а для последующего отображения.
     * Поэтому предварительно необходимо инициализировать swap-chain, чтобы на основании изображений из него, подготовить кадр. буферы (цвет. вложения).
     * При этом отдельные вложения кадрового буфера могут быть и вовсе не связаны со swap-chain (как, например, глубина), но быть использованы проходом
     */
    void initFrameBuffersPrimary(const vk::Format& colorAttachmentFormat, const vk::Format& depthStencilAttachmentFormat);

    /**
     * Де-инициализация основных кадровых буферов
     */
    void deInitFrameBuffersPrimary() noexcept;

    /**
     * Инициализация кадровых буферов для пост-процессинга
     * @param colorAttachmentFormat Формат цветовых вложений
     *
     * @details Это итоговые кадровые буферы, которые используются для показа.
     */
    void initFrameBuffersPostProcess(const vk::Format& colorAttachmentFormat);

    /**
     * Де-инициализация кадровых буферов для пост-процессинга
     */
    void deInitFrameBuffersPostProcess() noexcept;


    /**
     * Инициализация дескрипторных пулов и макетов размещения дескрипторов
     * @param maxMeshes Максимальное кол-во одновременно отображающихся мешей (влияет на максимальное кол-во наборов для материала меша и прочего)
     * @param frameBufferCount Кол-во кадровых буферов (от него зависит кол-во дескрипторных наборов передаваемых на этап пост-обработки)
     *
     * @details Дескрипторы описывают правила доступа из шейдера к различным ресурсам (таким как UBO буферы, изображения, и прочее). Они объединены в наборы
     * Наборы дескрипторов выделяются из дескрипторных пулов, а у пулов есть свой макет размещения, который описывает сколько наборов можно будет выделить
     * из пула и какие конкретно дескрипторы в этих наборах (и сколько их) будут доступны
     */
    void initDescriptorPoolsAndLayouts(size_t maxMeshes, size_t frameBufferCount);

    /**
     * Де-инициализация дескрипторов
     */
    void deInitDescriptorPoolsAndLayouts() noexcept;


    /**
     * Выделение дескрипторного набора для передачи изображения предыдущего кадра в проход пост-обработки
     * @param descriptorPool Unique smart pointer объекта дескрипторного пула
     * @param descriptorSetLayout Unique smart pointer макета размещения дескрипторного набора меша
     *
     * @details Для того чтобы осуществить пост-обработку нужно передавть во фрагментный шейдер прохода пост-обработки
     * изображение, полученное в результате работы предыдущего прхода. Чтобы это сделать нужен дескрипторный набор
     */
    void allocateFrameBuffersPrimaryDescriptorSets(
            const vk::UniqueDescriptorPool& descriptorPool,
            const vk::UniqueDescriptorSetLayout& descriptorSetLayout);

    /**
     * Обновление дескрипторных наборов основных кадровых буферов (связаывание с конрктеными изобрадениями)
     * @details Данный метод должен срабатывать всякий раз, когда пересоздаются изображения
     */
    void updateFrameBuffersPrimaryDescriptorSets();


    /**
     * Инициализация основного графического конвейера
     * @param vertexShaderCodeBytes Код вершинного шейдера
     * @param geometryShaderCodeBytes Код геометрического шейдера
     * @param fragmentShaderCodeBytes Код фрагментного шейдера
     */
    void initPipelinePrimary(
            const std::vector<unsigned char>& vertexShaderCodeBytes,
            const std::vector<unsigned char>& geometryShaderCodeBytes,
            const std::vector<unsigned char>& fragmentShaderCodeBytes);

    /**
     * Де-инициализация графического конвейера
     */
    void deInitPipelinePrimary() noexcept;

    /**
     * Инициализация конвейера пост-обработки
     * @param vertexShaderCodeBytes
     * @param fragmentShaderCodeBytes
     */
    void initPipelinePostProcess(
            const std::vector<unsigned char>& vertexShaderCodeBytes,
            const std::vector<unsigned char>& fragmentShaderCodeBytes);

    /**
     * Де-инициализация конвейера пост-обработки
     */
    void deInitPipelinePostProcess() noexcept;


    /**
     * Освобождение геометрических буферов
     *
     * @details Перед де-инициализацией объекта рендерера, перед уничтожением устройства, будет правильным очистить все
     * геометрические буферы которые когда либо выделялись.
     */
    void freeGeometryBuffers();

    /**
     * Освобождение текстурных буферов
     *
     * @details Перед де-инициализацией объекта рендерера, перед уничтожением устройства, будет правильным очистить все
     * текстурные буферы которые когда либо выделялись.
     */
    void freeTextureBuffers();

    /**
     * Очистка ресурсов мешей
     *
     * @details Поскольку каждый объект меша содержит набор дескрипторов и свои UBO буферы, перед уничтожением устройства
     * будет корректным очистить их ресурсы
     */
    void freeMeshes();

public:
    /**
     * Конструктор
     * @param hInstance Экземпляр WinApi приложения
     * @param hWnd Дескриптор окна WinApi
     * @param vertexShaderCodeBytes Код вершинного шейдера (байты)
     * @param geometryShaderCodeBytes Код геометрического шейдера (байты)
     * @param fragmentShaderCodeBytes Rод фрагментного шейдера (байты)
     * @param maxMeshes Максимальное кол-во мешей
     */
    VkRenderer(HINSTANCE hInstance,
            HWND hWnd,
            const std::vector<unsigned char>& vertexShaderCodeBytes,
            const std::vector<unsigned char>& geometryShaderCodeBytes,
            const std::vector<unsigned char>& fragmentShaderCodeBytes,
            const std::vector<unsigned char>& vertexShaderCodeBytesPp,
            const std::vector<unsigned char>& fragmentShaderCodeBytesPp,
            size_t maxMeshes = 1000);

    /**
     * Деструктор
     */
    ~VkRenderer();

    /**
     * Сменить статус рендеринга
     * @param isEnabled Выполняется ли рендеринг
     */
    void setRenderingStatus(bool isEnabled) noexcept;

    /**
     * Вызывается когда поверхность отображения изменилась
     *
     * @details Например, если сменился размер поверхности отображения, нужно заново пересоздать swap-chain.
     * Также необходимо пересоздать кадровые буферы и прочие компоненты которые зависят от swap-chain
     */
    void onSurfaceChanged();

    /**
     * Создание геометрического буфера
     * @param vertices Массив вершин
     * @param indices Массив индексов
     * @return Shared smart pointer на объект буфера
     */
    vk::resources::GeometryBufferPtr createGeometryBuffer(const std::vector<vk::tools::Vertex>& vertices, const std::vector<uint32_t>& indices);

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
    vk::resources::TextureBufferPtr createTextureBuffer(const unsigned char* imageBytes, uint32_t width, uint32_t height, uint32_t bpp, bool generateMip = false, bool sRgb = false);

    /**
     * Добавление меша на сцену
     * @param geometryBuffer Геометрический буфер
     * @param textureSet Текстурный набор
     * @param materialSettings Параметры материала меша
     * @param textureMapping Параметры отображения текстуры
     * @return Shared smart pointer на объект меша
     */
    vk::scene::MeshPtr addMeshToScene(const vk::resources::GeometryBufferPtr& geometryBuffer,
            const vk::scene::MeshTextureSet& textureSet = {},
            const vk::scene::MeshMaterialSettings& materialSettings = {{1.0f, 1.0f, 1.0f},1.0f,0.0f},
            const vk::scene::MeshTextureMapping& textureMapping = {{0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f});

    /**
     * Удалить меш со сцены
     * @param meshPtr Shared smart pointer на объект меша
     */
    void removeMeshFromScene(const vk::scene::MeshPtr& meshPtr);

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
    vk::scene::LightSourcePtr addLightToScene(const vk::scene::LightSourceType& type,
            const glm::vec3& position = {0.0f,0.0f,0.0f},
            const glm::vec3& color = {1.0f,1.0f,1.0f},
            glm::float32 attenuationLinear = 0.20f,
            glm::float32 attenuationQuadratic = 0.22f,
            glm::float32 cutOffAngle = 40.0f,
            glm::float32 cutOffOuterAngle = 45.0f);

    /**
     * Удалить источник света со сцены
     * @param lightSourcePtr
     */
    void removeLightFromScene(const vk::scene::LightSourcePtr& lightSourcePtr);

    /**
     * Доступ к камере
     * @return Константный указатель на объект камеры
     */
    vk::scene::Camera* getCameraPtr();

    /**
     * Рендеринг кадра
     */
    void draw();
};
