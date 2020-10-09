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
    /// Максимальное число мешей
    uint32_t maxMeshes_;

    /// Экземпляр Vulkan (smart pointer)
    vk::UniqueInstance vulkanInstance_;
    /// Идентификатор объекта для работы с debug-callback'ом
    vk::UniqueDebugReportCallbackEXT debugReportCallbackExt_;
    /// Поверхность для рисования на окне (smart pointer)
    vk::UniqueSurfaceKHR surface_;
    /// Устройство
    vk::tools::Device device_;
    /// Основной проход рендеринга
    vk::UniqueRenderPass mainRenderPass_;
    /// Объект очереди показа (swap-chain)
    vk::UniqueSwapchainKHR swapChainKhr_;
    /// Кадровые буферы
    std::vector<vk::resources::FrameBuffer> frameBuffers_;
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
    /// Объект дескрипторного пула для трассировки лучей
    vk::UniqueDescriptorPool descriptorPoolRayTracing_;

    /// Макет размещения дескрипторного набора для камеры (матрицы)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutCamera_;
    /// Макет размещения дескрипторного набора для меша (матрицы, материалы, текстуры)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutMeshes_;
    /// Макет размещения дескрипторного набора для источников света (кол-во, массив источников)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutLightSources_;
    /// Макет размещения дескрипторного набора для трассировки лучей
    vk::UniqueDescriptorSetLayout descriptorSetLayoutRayTracing_;

    /// Макет размещения графического конвейера
    vk::UniquePipelineLayout pipelineLayout_;
    /// Макет размещения ray-tracing конвейера
    vk::UniquePipelineLayout rtPipelineLayout_;
    /// Графический конвейер
    vk::UniquePipeline pipeline_;
    /// Ray-tracing конвейер
    vk::UniquePipeline rtPipeline_;

    /// Шейдерные группы для трассировки лучей
    std::vector<vk::RayTracingShaderGroupCreateInfoKHR> rtShaderGroups_;
    /// Буфер для таблицы SBT
    vk::tools::Buffer rtSbtTableBuffer_;

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

    /// Структура ускорения верхнего уровня для трассировки лучей
    vk::UniqueAccelerationStructureKHR rtTopLevelAccelerationStructureKhr_;
    ///Память для структуры ускорения верхнего уровня
    vk::UniqueDeviceMemory rtTopLevelAccelerationStructureMemory_;
    /// Буфер содержащий информацию об instance'ах используемых структурой ускорения
    vk::tools::Buffer rtTopLevelAccelerationStructureInstanceBuffer_;
    /// Структура ускорения верхнего уровня построена
    bool rtTopLevelAccelerationStructureReady_;

    /// Итоговое изображение хранящее результат трассировки сцены
    vk::tools::Image rtOffscreenBufferImage_;
    /// Дескрипторный набор для трассировки сцены
    vk::UniqueDescriptorSet rtDescriptorSet_;
    /// Готов ли дескрипторный набор к использованию
    bool rtDescriptorSetReady_;


    /**
     * Инициализация проходов рендеринга
     * @param colorAttachmentFormat Формат цветовых вложений
     * @param depthStencilAttachmentFormat Формат вложений глубины
     * @details Цветовые вложения - по сути изображения в которые шейдеры пишут информацию. У них могут быть форматы.
     * При создании проходов необходимо указать с каким форматом и размещением вложений происходит работа на конкретных этапах
     */
    void initRenderPasses(const vk::Format& colorAttachmentFormat, const vk::Format& depthStencilAttachmentFormat);

    /**
     * Де-инициализация проходов рендеринга
     */
    void deInitRenderPasses() noexcept;


    /**
     * Инициализация swap-chain (цепочки показа)
     * Цепочка показа - набор сменяющихся изображений показываемых на поверхности отображения
     * @param surfaceFormat Формат поверхности
     * @param bufferCount Желаемое кол-во буферов изображений (0 для авто-определения)
     */
    void initSwapChain(const vk::SurfaceFormatKHR& surfaceFormat, size_t bufferCount = 0);

    /**
     * Де-инициализация swap-chain (цепочки показа)
     */
    void deInitSwapChain() noexcept;


    /**
     * Инициализация кадровых буферов
     * @param colorAttachmentFormat Формат цветовых вложений
     * @param depthStencilAttachmentFormat Формат вложений глубины
     *
     * @details Несмотря на то, что кадровые буферы чисто логически не обязательно должны быть связаны с изображениями swap-chain'а,
     * если мы используем показ, то так или иначе, осуществляем запись в изображения swap-chain'а для последующего отображения.
     * Поэтому предварительно необходимо инициализировать swap-chain, чтобы на основании изображений из него, подготовить кадр. буферы (цвет. вложения).
     * При этом отдельные вложения кадрового буфера могут быть и вовсе не связаны со swap-chain (как, например, глубина), но быть использованы проходом
     */
    void initFrameBuffers(const vk::Format& colorAttachmentFormat, const vk::Format& depthStencilAttachmentFormat);

    /**
     * Де-инициализация кадровых буферов
     */
    void deInitFrameBuffers() noexcept;

    /**
     * Инициализация изображения хранящего результат трассировки лучами сцены
     * @param colorAttachmentFormat Формат цветового вложения
     */
    void initRtOffscreenBuffer(const vk::Format& colorAttachmentFormat);

    /**
     * Де-инициализация кадрового буфера для трассировки лучей
     */
    void deInitRtOffscreenBuffer() noexcept;


    /**
     * Инициализация дескрипторных пулов и макетов размещения дескрипторов
     * @param maxMeshes Максимальное кол-во одновременно отображающихся мешей (влияет на максимальное кол-во наборов для материала меша и прочего)
     *
     * @details Дескрипторы описывают правила доступа из шейдера к различным ресурсам (таким как UBO буферы, изображения, и прочее). Они объединены в наборы
     * Наборы дескрипторов выделяются из дескрипторных пулов, а у пулов есть свой макет размещения, который описывает сколько наборов можно будет выделить
     * из пула и какие конкретно дескрипторы в этих наборах (и сколько их) будут доступны
     */
    void initDescriptorPoolsAndLayouts(size_t maxMeshes);

    /**
     * Де-инициализация дескрипторов
     */
    void deInitDescriptorPoolsAndLayouts() noexcept;

    /**
     * Инициализация графического конвейера
     * @param vertexShaderCodeBytes Код вершинного шейдера
     * @param geometryShaderCodeBytes Код геометрического шейдера
     * @param fragmentShaderCodeBytes Код фрагментного шейдера
     */
    void initPipeline(
            const std::vector<unsigned char>& vertexShaderCodeBytes,
            const std::vector<unsigned char>& geometryShaderCodeBytes,
            const std::vector<unsigned char>& fragmentShaderCodeBytes);

    /**
     * Де-инициализация графического конвейера
     */
    void deInitPipeline() noexcept;

    /**
     * Инициализация конвейера трассировки лучей
     * @param rayGenShaderCodeBytes Код шейдера генерации лучей
     * @param rayMissShaderCodeBytes Код шейдера промаха лучей
     * @param rayMissShadowShaderCodeBytes Код шейдера промаха лучей для теней
     * @param rayHitShaderCodeBytes Код шейдера попадания луча
     */
    void initRtPipeline(
            const std::vector<unsigned char>& rayGenShaderCodeBytes,
            const std::vector<unsigned char>& rayMissShaderCodeBytes,
            const std::vector<unsigned char>& rayMissShadowShaderCodeBytes,
            const std::vector<unsigned char>& rayHitShaderCodeBytes);

    /**
     * Де-инициализация конвейера ray tracing
     */
    void deInitRtPipeline() noexcept;

    /**
     * Инициализация таблицы связи шейдеров
     * @details Таблица связей шейдеров описывает какие шейдеры будут срабатывать при промахе/генерации/пересечении луча
     * с геометрией какой-то конкретной hit-группы. По сути это схема процесса трассировки
     */
    void initRtShaderBindingTable();

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

    /**
     * Уничтожение структуры ускорения верхнего уровня (для трассировки лучей)
     */
    void rtDeInitTopLevelAccelerationStructure() noexcept;

public:
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
    VkRenderer(HINSTANCE hInstance,
               HWND hWnd,
               const std::vector<unsigned char>& vertexShaderCodeBytes,
               const std::vector<unsigned char>& geometryShaderCodeBytes,
               const std::vector<unsigned char>& fragmentShaderCodeBytes,

               const std::vector<unsigned char>& rayGenShaderCodeBytes,
               const std::vector<unsigned char>& rayMissShaderCodeBytes,
               const std::vector<unsigned char>& rayMissShadowShaderCodeBytes,
               const std::vector<unsigned char>& rayHitShaderCodeBytes,
               uint32_t maxMeshes = 100);

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
            const vk::scene::MeshMaterialSettings& materialSettings = {{0.05f, 0.05f, 0.05f},{0.8f, 0.8f, 0.8f},{0.6f, 0.6f, 0.6f},16.0f},
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
     * Рендеринг кадра (растеризация)
     */
    void draw();

    /**
     * Рендеринг кадра (трассировка)
     */
    void raytrace();

    /**
     * Построение структуры ускорения верхнего уровня (для трассировки лучей)
     * @param buildFlags Флаги построения структуры
     *
     * @details Подразумевается что данный метод будет вызываться после добавления мешей на сцену, поскольку для формирования
     * структуры ускорения верхнего уровня нужны структуры нижнего уровня (геометрия) и матрицы описывающие положения в пространстве.
     *
     * В перспективе данные метод станет приватным, а структура верхнего уровня будет обновляться всякий раз, когда происходит добавление
     * или удаление мешей на сцену.
     */
    void rtBuildTopLevelAccelerationStructure(const vk::BuildAccelerationStructureFlagsKHR& buildFlags =
    vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace |
    vk::BuildAccelerationStructureFlagBitsKHR::eAllowUpdate);

    /**
     * Подготовка дескрипторного набора для трассировки лучей
     *
     * @details Поскольку при инициализации дескрипторного набора, в качестве одного из дескрипторов используется структура ускорения
     * верхнего уровня, которая создается методом rtBuildTopLevelAccelerationStructure (который вызывается после наполнения сцены),
     * данный метод также нужно вызывать явно.
     *
     * В перспективе он также будет приватным, и набор дескрипторов будет всякий раз обновляться при обновлении объекта
     * структуры ускорения (при добавлении или удалении мешей)
     */
    void rtPrepareDescriptorSet();

    /**
     * Деинициализация дескрипторного набора
     */
    void rtDeInitDescriptorSet();
};
