#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define VULKAN_HPP_TYPESAFE_CONVERSION

#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <vector>

namespace vk
{
    namespace tools
    {
        /**
         * Вершина
         */
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 uv;
            glm::vec3 normal;
        };

        /**
         * Тип дескрипторного набора
         */
        enum DescriptorSetType {
            eUBO,
            eMeshMaterial
        };

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

        /**
         * Проверить доступны ли указанные расширения
         * @param extensionNames Список наименований расширений
         * @return Да или нет
         */
        bool CheckInstanceExtensionsSupported(const std::vector<const char*>& extensionNames);

        /**
         * Проверить поддерживаются ли указанные расширения физ. устройством
         * @param device Физ устройство
         * @param extensionNames Список наименований расширений
         * @return Да или нет
         */
        bool CheckDeviceExtensionsSupported(const vk::PhysicalDevice& device, const std::vector<const char*>& extensionNames);
        /**
         * Проверить доступны ли указанные слои валидации
         * @param layerNames Список наименований слоев
         * @return Да или нет
         */
        bool CheckInstanceLayersSupported(const std::vector<const char *> &layerNames);

        /**
         * Проверить поддерживаются ли указанные слои валидации
         * @param device Физ устройство
         * @param layerNames
         * @return Да или нет
         */
        bool CheckDeviceLayersSupported(const vk::PhysicalDevice& device, const std::vector<const char *> &layerNames);

        /**
         * Метод обратного вызова для обработки сообщений об ошибках и предупреждениях в ходе работы Vulkan
         * @param flags Битовая маска набора причин вызвавших метод обратного вызова (VkDebugReportFlagBitsEXT)
         * @param objType Тип объекта при работе с которым был вызван метод (VkDebugReportObjectTypeEXT)
         * @param obj Дескриптор объекта при работе с которым был вызван метод
         * @param location Компонента (слой,драйвер,загрузчик) причина вызова
         * @param messageCode Код сообщения
         * @param pLayerPrefix Префикс слоя (определен только на этапе вызова)
         * @param pMessage Сообщение (определено только на этапе вызова)
         * @param userData Пользовательские данные переданные во время создания объекта VkDebugReportCallbackEXT
         * @return Возвращает всегда VK_FALSE (особенности стандарта)
         */
        VKAPI_ATTR VkBool32 VKAPI_CALL DebugVulkanCallback(
                VkDebugReportFlagsEXT flags,
                VkDebugReportObjectTypeEXT objType,
                uint64_t obj,
                size_t location,
                int32_t messageCode,
                const char* pLayerPrefix,
                const char* msg,
                void* userData);
        /**
         * Создать семплер для изображения
         * @param device Объект логического устройства
         * @param filtering Тип фильтрации (интерполяции) текселей
         * @param addressMode Адресный режим для координат выходящих за пределы
         * @param anisotropyLevel Уровень анизотропной фильтрации
         * @return Объект текстурного семплера (smart-pointer)
         */
        vk::UniqueSampler createImageSampler(const vk::Device& device,
                const vk::Filter& filtering,
                const vk::SamplerAddressMode& addressMode,
                float anisotropyLevel = 0);
    }
}