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
                void* userData)
        {
            std::cout << "Vulkan: validation layer - " << msg << std::endl;
            return VK_FALSE;
        }
    }
}