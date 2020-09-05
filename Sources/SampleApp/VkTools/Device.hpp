#pragma once

#include "Tools.h"

#include <iostream>
#include <memory>
#include <utility>
#include <unordered_map>

namespace vk
{
    namespace tools
    {
        /**
         * Класс-обертка для устройства Vulkan
         */
        class Device
        {
        private:
            /// Готово ли устройство
            bool isReady_;
            /// Физическое устройство
            vk::PhysicalDevice physicalDevice_;
            /// Логическое устройство
            vk::UniqueDevice device_;
            /// Индекс семейства очередей граф. команд
            int queueFamilyGraphicsIndex_;
            /// Индекс семейства очередей команд представления
            int queueFamilyPresentIndex_;
            /// Индекс семейства очередей команд вычисления
            int queueFamilyComputeIndex_;
            /// Очередь графических команд
            vk::Queue queueGraphics_;
            /// Очередь команд представления
            vk::Queue queuePresent_;
            /// Очередь команд вычисления
            vk::Queue queueCompute_;
            /// Командный пул графического семейства (для выделения командных буферов)
            vk::UniqueCommandPool commandPoolGraphics_;
            /// Командный пул вычислительного семейства (для выделения командных буферов)
            vk::UniqueCommandPool commandPoolCompute_;

        public:
            /**
             * Конструктор по умолчанию
             */
            Device():
                    isReady_(false),
                    queueFamilyGraphicsIndex_(0),
                    queueFamilyPresentIndex_(0),
                    queueFamilyComputeIndex_(0)
            {};

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            Device(const Device& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            Device& operator=(const Device& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            Device(Device&& other) noexcept:Device(){
                std::swap(isReady_,other.isReady_);
                std::swap(queueFamilyPresentIndex_,other.queueFamilyPresentIndex_);
                std::swap(queueFamilyGraphicsIndex_,other.queueFamilyGraphicsIndex_);
                std::swap(queueGraphics_,other.queueGraphics_);
                std::swap(queuePresent_,other.queuePresent_);
                std::swap(queueCompute_, other.queueCompute_);
                std::swap(physicalDevice_ ,other.physicalDevice_);
                device_.swap(other.device_);
                commandPoolGraphics_.swap(other.commandPoolGraphics_);
                commandPoolCompute_.swap(other.commandPoolCompute_);
            }


            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            Device& operator=(Device&& other) noexcept {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                queueFamilyGraphicsIndex_ = 0;
                queueFamilyPresentIndex_ = 0;
                queueFamilyComputeIndex_ = 0;

                std::swap(isReady_,other.isReady_);
                std::swap(queueFamilyPresentIndex_,other.queueFamilyPresentIndex_);
                std::swap(queueFamilyGraphicsIndex_,other.queueFamilyGraphicsIndex_);
                std::swap(queueGraphics_,other.queueGraphics_);
                std::swap(queuePresent_,other.queuePresent_);
                std::swap(queueCompute_, other.queueCompute_);
                std::swap(physicalDevice_ ,other.physicalDevice_);
                device_.swap(other.device_);
                commandPoolGraphics_.swap(other.commandPoolGraphics_);
                commandPoolCompute_.swap(other.commandPoolCompute_);

                return *this;
            }

            /**
             * Конструктор
             * @param instance Экземпляр Vulkan
             * @param surfaceKhr Поверхность отображения на окне (для проверки доступности представления)
             * @param requireExtensions Запрашивать расширения (названия расширений)
             * @param requireValidationLayers Запрашивать слои (названия слоев)
             * @param allowIntegrated Позволять использование встроенных устройств
             */
            explicit Device(const vk::UniqueInstance& instance,
                            const vk::UniqueSurfaceKHR& surfaceKhr,
                            const std::vector<const char*>& requireExtensions = {},
                            const std::vector<const char*>& requireValidationLayers = {},
                            bool allowIntegrated = false):
        	isReady_(false)
            {
                // Найти подходящее физ. устройство. Семейства очередей устройства должны поддерживать необходимые типы команд
                auto physicalDevices = instance->enumeratePhysicalDevices();
                if(!physicalDevices.empty())
                {
                    bool physicalDeviceSelected = false;

                    for(const auto& physicalDevice : physicalDevices)
                    {
                        auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
                        int queueFamilyPresentIndex = -1;
                        int queueFamilyGraphicsIndex = -1;
                        int queueFamilyComputeIndex = -1;

                        // Найти семейства поддерживающие графику и представление
                        for(size_t i = 0; i < queueFamilyProperties.size(); i++)
                        {
                            // Поддерживает ли семейство графические команды
                            if(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics){
                                queueFamilyGraphicsIndex = i;
                            }

                            // Поддерживает ли семейство команды вычисления
                            if(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute){
                                queueFamilyComputeIndex = i;
                            }

                            // Поддерживает ли семейство представление
                            vk::Bool32 presentSupported = false;
                            physicalDevice.getSurfaceSupportKHR(i,surfaceKhr.get(),&presentSupported);
                            if(presentSupported) queueFamilyPresentIndex = i;
                        }

                        // Перейти к следующему устройству если необходимые возможности не поддерживаются
                        if(queueFamilyPresentIndex == -1 || queueFamilyGraphicsIndex == -1 || queueFamilyComputeIndex == -1){
                            continue;
                        }

                        // Если запросили какие-то расширения устройства и не все из них доступны - к следующему устройству
                        if(!requireExtensions.empty() && !CheckDeviceExtensionsSupported(physicalDevice,requireExtensions)){
                            continue;
                        }

                        // Если запросили какие-то слои устройства и не все из них доступны - к следующему устройству
                        if(!requireValidationLayers.empty() && !CheckDeviceLayersSupported(physicalDevice,requireValidationLayers)){
                            continue;
                        }

                        // Если запрещено использовать встроенные устройства, то в случае если устройство не дискретное - к следующему устройству
                        if(!allowIntegrated && physicalDevice.getProperties().deviceType != vk::PhysicalDeviceType::eDiscreteGpu){
                            continue;
                        }

                        // Если для работы с поверхностью у устройства нет форматов и режимов представления - к следующему
                        auto formats = physicalDevice.getSurfaceFormatsKHR(surfaceKhr.get());
                        auto presentModes = physicalDevice.getSurfacePresentModesKHR(surfaceKhr.get());
                        if(formats.empty() || presentModes.empty()){
                            continue;
                        }

                        // Если все проверки пройдены - сохраняем найденное физ. устройство и индексы
                        physicalDevice_ = physicalDevice;
                        queueFamilyGraphicsIndex_ = queueFamilyGraphicsIndex;
                        queueFamilyPresentIndex_ = queueFamilyPresentIndex;
                        queueFamilyComputeIndex_ = queueFamilyComputeIndex;

                        // Физическое устройство выбрано
                        physicalDeviceSelected = true;

                        // Выходим из цикла
                        break;
                    }

                    // Если физическое устройство выбрано
                    if(physicalDeviceSelected)
                    {
                        // Массив с описанием создаваемых очередей
                        std::vector<vk::DeviceQueueCreateInfo> queueCreateInfoEntries;

                        // Индексы семейств очередей
                        // Если одно и то же семейство поддерживает все необходимые типы команд, индексы могут совпадать
                        std::vector<uint32_t> queueFamilies = {
                                static_cast<uint32_t>(queueFamilyGraphicsIndex_),
                                static_cast<uint32_t>(queueFamilyPresentIndex_),
                                static_cast<uint32_t>(queueFamilyComputeIndex_)
                        };

                        // Ассоциативный массив - количества очередей выделяемых на каждое семейство
                        // Выделяется одна очередь на семейство, но если индексы семейств семейств совпадают то выделяется более одной
                        std::unordered_map<uint32_t,uint32_t> queueCounts;
                        // Ассоциативный массив - приоритеты очередей в пределах одного семейства
                        std::unordered_map<uint32_t,std::vector<glm::float32_t>> queuePriorities;

                        // Массив индексов семейств очередей исключающий дубликаты
                        std::vector<uint32_t> queueFamilyIndicesUnique;

                        // Заполнить ассоциативный массив количеств очередей
                        for(uint32_t i = 0; i < queueFamilies.size(); i++)
                        {
                            auto queueFamilyIndex = queueFamilies[i];
                            if(queueCounts.find(queueFamilyIndex) != queueCounts.end()){
                                queueCounts[queueFamilyIndex]++;
                                queuePriorities[queueFamilyIndex].push_back(1.0f);
                            }else{
                                queueCounts[queueFamilyIndex] = 1;
                                queuePriorities[queueFamilyIndex] = {1.0f};
                                queueFamilyIndicesUnique.push_back(queueFamilyIndex);
                            }
                        }

                        // Заполняем массив описаний создаваемых очередей
                        // Если считать что совпадений нет, на каждое семейство создается одна структура vk::DeviceQueueCreateInfo
                        // Если индексы семейств совпадают, структур может быть меньше чем явных семейств, но одна структура может выделять несколько очередей
                        for(auto queueCount : queueCounts){
                            vk::DeviceQueueCreateInfo queueCreateInfo = {};
                            queueCreateInfo.setQueueFamilyIndex(queueCount.first);
                            queueCreateInfo.setQueueCount(queueCount.second);
                            queueCreateInfo.setPQueuePriorities(queuePriorities[queueCount.first].data());
                            queueCreateInfoEntries.push_back(queueCreateInfo);
                        }

                        // Особенности устройства
                        vk::PhysicalDeviceFeatures physicalDeviceFeatures{};
                        physicalDeviceFeatures.setGeometryShader(VK_TRUE);
                        physicalDeviceFeatures.setSamplerAnisotropy(VK_TRUE);

                        vk::PhysicalDeviceFeatures2 deviceFeatures2{};
                        deviceFeatures2.setFeatures(physicalDeviceFeatures);

                        vk::PhysicalDeviceRayTracingFeaturesKHR rayTracingFeaturesKhr{};
                        rayTracingFeaturesKhr.setRayTracing(VK_TRUE);
                        rayTracingFeaturesKhr.setRayQuery(VK_FALSE);
                        rayTracingFeaturesKhr.setPNext(&deviceFeatures2);

                        vk::PhysicalDeviceVulkan12Features vulkan12Features{};
                        vulkan12Features.setDescriptorBindingVariableDescriptorCount(VK_TRUE);
                        vulkan12Features.setRuntimeDescriptorArray(VK_TRUE);
                        vulkan12Features.setDescriptorIndexing(VK_TRUE);
                        vulkan12Features.setBufferDeviceAddress(VK_TRUE);
                        vulkan12Features.setPNext(&rayTracingFeaturesKhr);

                        // Информация о создаваемом устройстве
                        vk::DeviceCreateInfo deviceCreateInfo{};
                        deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfoEntries.size());
                        deviceCreateInfo.setPQueueCreateInfos(queueCreateInfoEntries.data());
                        deviceCreateInfo.setPpEnabledExtensionNames(!requireExtensions.empty() ? requireExtensions.data() : nullptr);
                        deviceCreateInfo.setEnabledExtensionCount(requireExtensions.size());
                        deviceCreateInfo.setPpEnabledLayerNames(!requireValidationLayers.empty() ? requireValidationLayers.data() : nullptr);
                        deviceCreateInfo.setEnabledLayerCount(requireValidationLayers.size());
//                        deviceCreateInfo.setPEnabledFeatures(&physicalDeviceFeatures);
                        deviceCreateInfo.setPNext(&vulkan12Features);

                        // Создание устройства
                        this->device_ = physicalDevice_.createDeviceUnique(deviceCreateInfo);

                        // Получение очередей
                        std::vector<vk::Queue> queues;
                        for(auto queueFamilyIndex : queueFamilyIndicesUnique){
                            auto queueCount = queueCounts[queueFamilyIndex];
                            for(uint32_t i = 0; i < queueCount; i++){
                                queues.push_back(device_->getQueue(queueFamilyIndex, i));
                            }
                        }

                        // Известно что кол-во полученных очередей не меньше чем изначальное кол-во семейств (3)
                        queueGraphics_ = queues[0];
                        queuePresent_ = queues[1];
                        queueCompute_ = queues[2];

                        // Создание командного пула для графического семейства
                        commandPoolGraphics_ = device_->createCommandPoolUnique({
                                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                static_cast<uint32_t>(queueFamilyGraphicsIndex_)
                        });

                        // Создание командного пула для вычислительного семейства
                        commandPoolCompute_ = device_->createCommandPoolUnique({
                                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                static_cast<uint32_t>(queueFamilyComputeIndex_)
                        });

                        // Инициализация успешно произведена
                        isReady_ = true;
                    }
                }

                // Если в итоге устройство не инициализировано
                if(!isReady_){
                    throw vk::InitializationFailedError("Can't initialize device");
                }
            }

            /**
             * Деструктор
             */
            ~Device()
            {
                destroyVulkanResources();
            }

            /**
             * Уничтожение устройства Vulkan
             */
            void destroyVulkanResources()
            {
                if(isReady_)
                {
                    // Уничтожить командные пулы
                    this->device_->destroyCommandPool(commandPoolGraphics_.get());
                    this->commandPoolGraphics_.release();
                    this->device_->destroyCommandPool(commandPoolCompute_.get());
                    this->commandPoolCompute_.release();

                    // Уничтожить логическое устройство
                    this->device_->destroy();
                    this->device_.release();

                    isReady_ = false;
                }
            }

            /**
             * Получить физическое устройство
             * @return Константная ссылка на физ. устройство
             */
            const vk::PhysicalDevice& getPhysicalDevice() const
            {
                return physicalDevice_;
            }

            /**
             * Получить логическое устройство
             * @return Константная ссылка на smart pointer логического устройства
             */
            const vk::UniqueDevice& getLogicalDevice() const
            {
                return device_;
            }

            /**
             * Готов ли объект к использованию
             * @return Да или нет
             */
            bool isReady() const
            {
                return isReady_;
            }

            /**
             * Получить очередь графических команд
             * @return Очередь
             */
            vk::Queue getGraphicsQueue() const
            {
                return queueGraphics_;
            }

            /**
             * Получить очередь команд представления
             * @return Очередь
             */
            vk::Queue getPresentQueue() const
            {
                return queuePresent_;
            }

            /**
             * Получить очередь команд вычисления
             * @return
             */
            vk::Queue getComputeQueue() const
            {
                return queueCompute_;
            }

            /**
             * Получить командный пул графических команд
             * @return Константная ссылка на smart pointer объекта командного пула
             */
            const vk::UniqueCommandPool& getCommandGfxPool() const
            {
                return commandPoolGraphics_;
            }

            /**
             * Получить командный пул вычислительных команд
             * @return Константная ссылка на smart pointer объекта командного пула
             */
            const vk::UniqueCommandPool& getCommandComputePool() const
            {
                return commandPoolCompute_;
            }

            /**
             * Получить размер динамически выравненного блока в UBO с учетом лимитов устройства
             * @tparam T тип элемента передаваемого в UBO
             * @return Размер блока
             */
            template <typename T>
            vk::DeviceSize getDynamicallyAlignedUboBlockSize() const {
                if(!isReady_) return 0;
                const auto minUboAlignment = physicalDevice_.getProperties().limits.minUniformBufferOffsetAlignment;
                auto dynamicAlignment = static_cast<vk::DeviceSize>(sizeof(T));

                if (minUboAlignment > 0) {
                    dynamicAlignment = (dynamicAlignment + minUboAlignment - 1) & ~(minUboAlignment - 1);
                }

                return dynamicAlignment;
            }

            /**
             * Поддерживается ли формат поверхности (цветовое пространство и формат совпадает)
             * @param surfaceFormatKhr Формат поверхности (формат и цветовое пространство)
             * @param surfaceKhr Поверхность
             * @return Да или нет
             */
            bool isSurfaceFormatSupported(const vk::SurfaceFormatKHR& surfaceFormatKhr, const vk::UniqueSurfaceKHR& surfaceKhr) const
            {
                if(!isReady_) return false;

                auto surfaceFormats = this->physicalDevice_.getSurfaceFormatsKHR(surfaceKhr.get());
                if(surfaceFormats.empty()) return false;

                if(surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined){
                    return true;
                }

                for(const auto& surfaceFormatEntry : surfaceFormats)
                {
                    if(surfaceFormatEntry.colorSpace == surfaceFormatKhr.colorSpace &&
                       surfaceFormatEntry.format == surfaceFormatKhr.format)
                    {
                        return true;
                    }
                }

                return false;
            }

            /**
             * Поддерживается ли формат цветовых вложений устройством для конкретной поверхности
             * @param format Формат (отвечает за то, сколько байт приходится на цвет и в какой последовательности)
             * @param surfaceKhr Поверхность
             * @return Да или нет
             */
            bool isFormatSupported(const vk::Format& format, const vk::UniqueSurfaceKHR& surfaceKhr) const
            {
                if(!isReady_) return false;

                auto surfaceFormats = this->physicalDevice_.getSurfaceFormatsKHR(surfaceKhr.get());
                if(surfaceFormats.empty()) return false;

                if(surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined){
                    return true;
                }

                for(const auto& surfaceFormatEntry : surfaceFormats)
                {
                    if(surfaceFormatEntry.format == format){
                        return true;
                    }
                }

                return false;
            }

            /**
             * Поддерживается ли устройством возможность использования глубины-трафарета для конкретного формата
             * @param format Формат (отвечает за то, сколько байт приходится на цвет и в какой последовательности)
             * @return Да или нет
             */
            bool isDepthStencilSupportedForFormat(const vk::Format& format) const
            {
                if(!isReady_) return false;
                const auto formatProperties = this->physicalDevice_.getFormatProperties(format);
                return !!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment);
            }

            /**
             * Получить индекс типа памяти у которого есть все необходимые свойства
             * @param typeBits Тип памяти
             * @param memoryPropertyFlag Свойства (напр, доступность и видимость памяти хостом, устройством)
             * @return Индекс (если не найдено, то -1)
             */
            int getMemoryTypeIndex(uint32_t typeBits, const vk::MemoryPropertyFlags& memoryPropertyFlag) const
            {
                if(!isReady_) return -1;

                auto memoryProperties = this->physicalDevice_.getMemoryProperties();

//                for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
//                    if ((typeBits & 1) == 1){
//                        if ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlag) == memoryPropertyFlag){
//                            return i;
//                        }
//                    }
//                    typeBits >>= 1;
//                }

                for(uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
                {
                    if(((typeBits & (1 << i)) > 0) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlag) == memoryPropertyFlag)
                    {
                        return i;
                    }
                }

                return -1;
            }

            /**
             * Используется ли для графических команд и для показа одно и то же семейство
             * @return Да или нет
             */
            bool isPresentAndGfxQueueFamilySame() const
            {
                return queueFamilyPresentIndex_ == queueFamilyGraphicsIndex_;
            }

            /**
             * Получить массив индексов семейств очередей
             * @return Массив uint32_t значений
             */
            std::vector<uint32_t> getQueueFamilyIndices() const
            {
                return {
                    static_cast<uint32_t>(queueFamilyGraphicsIndex_),
                    static_cast<uint32_t>(queueFamilyPresentIndex_)
                };
            }
        };
    }
}
