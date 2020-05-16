#pragma once

#include "VkTools.h"
#include "VkToolsDevice.hpp"
#include "VkToolsFrameBuffer.hpp"
#include "VkToolsBuffer.hpp"

class VkRenderer
{
private:
    /// Экземпляр Vulkan (smart pointer)
    vk::UniqueInstance vulkanInstance_;
    /// Идентификатор объекта для работы с debug-callback'ом
    VkDebugReportCallbackEXT debugReportCallback_;
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
    /// UBO буфер для матриц модели (для каждого меша свой регион буфера)
    vk::tools::Buffer uboBufferModel_;


    /**
     * Инициализация экземпляра
     * @param appName Наименования приложения
     * @param engineName Наименование движка
     * @param requireExtensions Запрашивать расширения (названия расширений)
     * @param requireValidationLayers Запрашивать слои (названия слоев)
     */
    static vk::UniqueInstance initInstance(const std::string& appName,
            const std::string& engineName,
            const std::vector<const char*>& requireExtensions = {},
            const std::vector<const char*>& requireValidationLayers = {});

    /**
     * Создание объекта для работы с debug-callback'ом
     * @param vulkanInstance Экземпляр Vulkan
     * @return Идентификатор объекта для работы с debug-callback'ом
     */
    static VkDebugReportCallbackEXT createDebugReportCallback(const vk::UniqueInstance& vulkanInstance);

    /**
     * Создание поверхности отображения на окне
     * @param vulkanInstance Экземпляр Vulkan
     * @param hInstance экземпляр WinApi приложения
     * @param hWnd дескриптор окна WinApi
     * @return Поверхность для рисования на окне (smart pointer)
     */
    static vk::UniqueSurfaceKHR createSurface(const vk::UniqueInstance& vulkanInstance, HINSTANCE hInstance, HWND hWnd);

    /**
     * Инициализация устройства
     * @param instance Экземпляр Vulkan
     * @param surfaceKhr Поверхность отображения
     * @param requireExtensions Запрашивать расширения (названия расширений)
     * @param requireValidationLayers Запрашивать слои (названия слоев)
     * @param allowIntegratedDevices Позволять использование встроенной графики
     * @return Указатель на созданный объект-обертку над устройством Vulkan
     */
    static vk::tools::Device initDevice(const vk::UniqueInstance& instance,
            const vk::UniqueSurfaceKHR& surfaceKhr,
            const std::vector<const char*>& requireExtensions = {},
            const std::vector<const char*>& requireValidationLayers = {},
            bool allowIntegratedDevices = false);

    /**
     * Создание прохода рендерера
     * @param device Объект-обертка устройства
     * @param surfaceKhr Поверхность отображения (для проверки поддержки форматов поверхностью)
     * @param colorAttachmentFormat Формат цветовых вложений
     * @param depthStencilAttachmentFormat Формат вложений глубины трафарета
     * @details Цветовые вложения - по сути изображения в которые шейдеры пишут информацию. У них могут быть форматы.
     * Для vulkan важно настроить доступность и формат вложений на этапе инициализации прохода
     * @return Проход рендеринга (smart pointer)
     */
    static vk::UniqueRenderPass createRenderPass(const vk::tools::Device& device,
            const vk::UniqueSurfaceKHR& surfaceKhr,
            const vk::Format& colorAttachmentFormat,
            const vk::Format& depthStencilAttachmentFormat);

    /**
     * Создать объект цепочки показа
     * @param device Объект-обертка устройства
     * @param surfaceFormat Формат поверхности
     * @param surfaceKhr Поверхность отображения (для проверки поддержки форматов поверхностью)
     * @param bufferCount Кол-во буферов (0 - авто-определение)
     * @return Объект swap-chain (smart pointer)
     */
    static vk::UniqueSwapchainKHR createSwapChain(
            const vk::tools::Device& device,
            const vk::SurfaceFormatKHR& surfaceFormat,
            const vk::UniqueSurfaceKHR& surfaceKhr,
            size_t bufferCount = 0);

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
    static void initFrameBuffers(std::vector<vk::tools::FrameBuffer>* frameBuffers,
            const vk::tools::Device& device,
            const vk::UniqueSurfaceKHR& surfaceKhr,
            const vk::UniqueSwapchainKHR& swapChain,
            const vk::UniqueRenderPass& renderPass,
            const vk::Format& colorAttachmentFormat,
            const vk::Format& depthStencilAttachmentFormat);

    /**
     * Инициализация UBO буферов
     * @param device Объект-обертка устройства
     * @param uboViewProjection Указатель на объект UBO буфера матриц вида-проекции
     * @param uboModel Указатель на объект UBO буфера матриц модели (на каждый меш своя матрица)
     * @param maxMeshes Максимальное кол-во возможных мешей
     */
    static void initUboBuffers(const vk::tools::Device& device, vk::tools::Buffer* uboViewProjection, vk::tools::Buffer* uboModel, size_t maxMeshes);

public:
    /**
     * Конструктор
     * @param hInstance Экземпляр WinApi приложения
     * @param hWnd Дескриптор окна WinApi
     * @param maxMeshes Максимальное кол-во мешей
     */
    VkRenderer(HINSTANCE hInstance, HWND hWnd, size_t maxMeshes = 1000);

    /**
     * Деструктор
     */
    ~VkRenderer();
};
