#pragma once

#include "VkTools.h"
#include "VkToolsDevice.hpp"
#include "VkToolsFrameBuffer.hpp"
#include "VkToolsBuffer.hpp"
#include "VkToolsGeometryBuffer.hpp"

#include "Scene/Mesh.h"
#include "Scene/Camera.h"

class VkRenderer
{
private:
    /// Запущен ли рендеринг
    bool isEnabled_;
    /// Готовы ли командные буферы
    bool isCommandsReady_;
    /// Данные на вход (о вершинах) подаются в стиле OpenGL (считая что начало view-port'а в нижнем левом углу)
    bool inputDataInOpenGlStyle_;

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
    std::vector<vk::tools::FrameBuffer> frameBuffers_;
    /// Командные буферы
    std::vector<vk::CommandBuffer> commandBuffers_;

    /// UBO буфер для матриц вида и проекции
    vk::tools::Buffer uboBufferViewProjection_;
    /// Текстурный семплер по умолчанию, используемый для всех создаваемых текстур
    vk::UniqueSampler textureSamplerDefault_;

    /// Объект дескрипторного пула для выделения набора для камеры (матрицы)
    vk::UniqueDescriptorPool descriptorPoolCamera_;
    /// Объект дескрипторного пула для выделения наборов мешей (матрицы, материалы)
    vk::UniqueDescriptorPool descriptorPoolMeshes_;

    /// Макет размещения дескрипторного набора для камеры (матрицы)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutCamera_;
    /// Макет размещения дескрипторного набора для меша (матрицы, материалы)
    vk::UniqueDescriptorSetLayout descriptorSetLayoutMeshes_;

    /// Макет размещения графического конвейера
    vk::UniquePipelineLayout pipelineLayout_;
    /// Графический конвейер
    vk::UniquePipeline pipeline_;

    /// Примитивы синхронизации - семафор сигнализирующий о готовности к рендерингу
    vk::UniqueSemaphore semaphoreReadyToRender_;
    /// Примитивы синхронизации - семафор сигнализирующий о готовности к показу отрендереной картинки
    vk::UniqueSemaphore semaphoreReadyToPresent_;

    /// Массив указателей на выделенные геометрические буферы
    std::vector<vk::tools::GeometryBufferPtr> geometryBuffers_;
    /// Массив указателей мешей сцены
    std::vector<vk::scene::MeshPtr> sceneMeshes_;
    /// Камера (матрицы, UBO)
    vk::scene::Camera camera_;

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
     * Инициализация UBO буферов
     * @param maxMeshes Максимальное кол-во одновременно отображающихся мешей (влияет на размер буфера матриц моделей)
     *
     * @details UBO буферы используются для передачи информации в шейдер во время рендеринга.
     * Например, матрицы (проекции, вида, модели), источники света и другое.
     */
    void initUboBuffers(size_t maxMeshes);

    /**
     * Де-инициализация UBO буферов
     */
    void deInitUboBuffers() noexcept;


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
     * @param fragmentShaderCodeBytes Код фрагментного шейдера
     */
    void initPipeline(
            const std::vector<unsigned char>& vertexShaderCodeBytes,
            const std::vector<unsigned char>& fragmentShaderCodeBytes);

    /**
     * Де-инициализация графического конвейера
     */
    void deInitPipeline() noexcept;

    /**
     * Освобождение геометрических буферов
     *
     * @details Перед де-инициализацией объекта рендерера, перед уничтожением устройства, будет правильным очистить все
     * геометрические буферы которые когда либо выделялись.
     */
    void freeGeometryBuffers();

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
     * @param fragmentShaderCodeBytes Rод фрагментного шейдера (байты)
     * @param maxMeshes Максимальное кол-во мешей
     */
    VkRenderer(HINSTANCE hInstance,
            HWND hWnd,
            const std::vector<unsigned char>& vertexShaderCodeBytes,
            const std::vector<unsigned char>& fragmentShaderCodeBytes,
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
    vk::tools::GeometryBufferPtr createGeometryBuffer(const std::vector<vk::tools::Vertex>& vertices, const std::vector<size_t>& indices);

    /**
     * Добавление меша на сцену
     * @param geometryBuffer Геометрический буфер
     * @param materialSettings Параметры материала меша
     * @return Shared smart pointer на объект меша
     */
    vk::scene::MeshPtr addMeshToScene(const vk::tools::GeometryBufferPtr& geometryBuffer,
            const vk::scene::MeshMaterialSettings& materialSettings = {{1.0f,1.0f,1.0f},0.0f,1.0f});

    /**
     * Удалить меш со сцены
     * @param meshPtr Shared smart pointer на объект меша
     */
    void removeMeshFromScene(const vk::scene::MeshPtr& meshPtr);

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
