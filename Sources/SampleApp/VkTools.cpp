#include "VkTools.h"
#include <iostream>

namespace vk
{
    namespace tools
    {
        /**
         * Проверить доступны ли указанные расширения
         * @param extensionNames Список наименований расширений
         * @return Да или нет
         */
        bool CheckInstanceExtensionsSupported(const std::vector<const char*>& extensionNames)
        {
            auto extensionProperties = vk::enumerateInstanceExtensionProperties();
            if(extensionProperties.empty()){
                return false;
            }else{
                for(const char * extensionName : extensionNames){
                    bool found = false;
                    for(const auto& extensionInfo : extensionProperties){
                        if(strcmp(extensionName, extensionInfo.extensionName) == 0){
                            found = true;
                            break;
                        }
                    }
                    if(!found) return false;
                }
            }
            return true;
        }

        /**
         * Проверить поддерживаются ли указанные расширения физ. устройством
         * @param device Физ устройство
         * @param extensionNames Список наименований расширений
         * @return Да или нет
         */
        bool CheckDeviceExtensionsSupported(const vk::PhysicalDevice &device, const std::vector<const char *> &extensionNames)
        {
            auto extensionProperties = device.enumerateDeviceExtensionProperties();
            if(extensionProperties.empty()){
                return false;
            }else{
                for(const char * extensionName : extensionNames){
                    bool found = false;
                    for(const auto& extensionInfo : extensionProperties){
                        if(strcmp(extensionName, extensionInfo.extensionName) == 0){
                            found = true;
                            break;
                        }
                    }
                    if(!found) return false;
                }
            }
            return true;
        }

        /**
         * Проверить доступны ли указанные слои валидации
         * @param layerNames Список наименований слоев
         * @return Да или нет
         */
        bool CheckInstanceLayersSupported(const std::vector<const char *> &layerNames)
        {
            auto validationLayerProperties = vk::enumerateInstanceLayerProperties();
            if(validationLayerProperties.empty()){
                return false;
            }else{
                for(const char * layerName : layerNames){
                    bool found = false;
                    for(const auto& layerInfo : validationLayerProperties){
                        if(strcmp(layerName, layerInfo.layerName) == 0){
                            found = true;
                            break;
                        }
                    }
                    if(!found) return false;
                }
            }
            return true;
        }

        /**
         * Проверить поддерживаются ли указанные слои валидации
         * @param device Физ устройство
         * @param layerNames
         * @return Да или нет
         */
        bool CheckDeviceLayersSupported(const vk::PhysicalDevice &device, const std::vector<const char *> &layerNames)
        {
            auto validationLayerProperties = device.enumerateDeviceLayerProperties();
            if(validationLayerProperties.empty()){
                return false;
            }else{
                for(const char * layerName : layerNames){
                    bool found = false;
                    for(const auto& layerInfo : validationLayerProperties){
                        if(strcmp(layerName, layerInfo.layerName) == 0){
                            found = true;
                            break;
                        }
                    }
                    if(!found) return false;
                }
            }
            return true;
        }

        /**
         * Создать экземпляр Vulkan
         * @param appName Наименование приложения
         * @param engineName Наименование движка
         * @param appVersion Версия приложения
         * @param engineVersion Версия движка
         * @param requireExtensions Запрашивать следующие расширения
         * @param requireLayers Запрашивать следующие слои
         * @return Unique smart pointer объект экземпляра
         */
        vk::UniqueInstance CreateVulkanInstance(
                const std::string& appName,
                const std::string& engineName,
                uint32_t appVersion,
                uint32_t engineVersion,
                const std::vector<const char*>& requireExtensions,
                const std::vector<const char*>& requireLayers)
        {
            // Структуры с информацией о приложении
            vk::ApplicationInfo applicationInfo(
                    appName.c_str(),
                    appVersion,
                    engineName.c_str(),
                    engineVersion,
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
            if(!requireLayers.empty()){
                if(vk::tools::CheckInstanceLayersSupported(requireLayers)){
                    instanceCreateInfo.setPpEnabledLayerNames(requireLayers.data());
                    instanceCreateInfo.setEnabledLayerCount(requireLayers.size());
                }else{
                    throw vk::LayerNotPresentError("Some of required layers is unavailable");
                }
            }

            return vk::createInstanceUnique(instanceCreateInfo);
        }

        /**
         * Метод обратного вызова для обработки сообщений об ошибках и предупреждениях в ходе работы Vulkan
         * @param flags Битовая маска набора причин вызвавших метод обратного вызова (VkDebugReportFlagBitsEXT)
         * @param objType Тип объекта при работе с которым был вызван метод (VkDebugReportObjectTypeEXT)
         * @param obj Дескриптор объекта при работе с которым был вызван метод
         * @param location Компонента (слой,драйвер,загрузчик) причина вызова
         * @param messageCode Код сообщения
         * @param pLayerPrefix Префикс слоя (определен только на этапе вызова)
         * @param msg Сообщение (определено только на этапе вызова)
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
                void* userData)
        {
            std::cout << "Vulkan: validation layer - " << msg << std::endl;
            return VK_FALSE;
        }

        /**
         * Создать семплер для изображения
         * @param device Объект логического устройства
         * @param filtering Тип фильтрации (интерполяции) текселей
         * @param addressMode Адресный режим для координат выходящих за пределы
         * @param anisotropyLevel Уровень анизотропной фильтрации
         * @return Объект текстурного семплера (smart-pointer)
         */
        vk::UniqueSampler CreateImageSampler(const vk::Device& device,
                                             const vk::Filter& filtering,
                                             const vk::SamplerAddressMode& addressMode,
                                             float anisotropyLevel)
        {
            vk::SamplerCreateInfo samplerCreateInfo{};
            // Тип фильтрации в случае если тексель отображаемой текстуры больше фрагмента (пикселя экрана)
            samplerCreateInfo.magFilter = filtering;
            // Тип фильтрации в случае если тексель отображаемой текстуры меньше фрагмента (пикселя экрана)
            samplerCreateInfo.minFilter = filtering;
            // Тип фильтрации для mip уровней
            samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
            // Как обрабатывать координаты оказавшиеся за пределами изображения
            samplerCreateInfo.addressModeU = addressMode;
            samplerCreateInfo.addressModeV = addressMode;
            samplerCreateInfo.addressModeW = addressMode;
            // Анизотропная фильтрация
            samplerCreateInfo.anisotropyEnable = anisotropyLevel > 0;
            samplerCreateInfo.maxAnisotropy = anisotropyLevel;
            // Цвет границ (если, например, используется адресный режим vk::SamplerAddressMode::eClampToBorder
            samplerCreateInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
            // Не используем не нормированные координаты (выборка в шейдере от 0 ло 1)
            samplerCreateInfo.unnormalizedCoordinates = false;
            // Не используем функцию сравнения глубины
            // Если изображение с которым работает семплер содержит вложение глубины, то можно осуществлять проверку (больше ли глубина чем глубина текущего фрагмента)
            samplerCreateInfo.compareEnable = false;
            samplerCreateInfo.compareOp = vk::CompareOp::eAlways;

            return device.createSamplerUnique(samplerCreateInfo);
        }
    }
}