#pragma once

#include "VkTools.h"
#include <iostream>
#include <memory>
#include <utility>

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
            /// Очередь графических команд
            vk::Queue queueGraphics_;
            /// Очередь команд представления
            vk::Queue queuePresent_;
            /// Командный пул графического семейства (для выделения командных буферов)
            vk::UniqueCommandPool commandPoolGraphics_;

        public:
            /**
             * Конструктор по умолчанию
             */
            Device():
                    isReady_(false),
                    queueFamilyGraphicsIndex_(0),
                    queueFamilyPresentIndex_(0)
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
                std::swap(physicalDevice_ ,other.physicalDevice_);
                device_.swap(other.device_);
                commandPoolGraphics_.swap(other.commandPoolGraphics_);
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

                std::swap(isReady_,other.isReady_);
                std::swap(queueFamilyPresentIndex_,other.queueFamilyPresentIndex_);
                std::swap(queueFamilyGraphicsIndex_,other.queueFamilyGraphicsIndex_);
                std::swap(queueGraphics_,other.queueGraphics_);
                std::swap(queuePresent_,other.queuePresent_);
                std::swap(physicalDevice_ ,other.physicalDevice_);
                device_.swap(other.device_);
                commandPoolGraphics_.swap(other.commandPoolGraphics_);

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
                            bool allowIntegrated = false)
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

                        // Найти семейства поддерживающие графику и представление
                        for(int i = 0; i < queueFamilyProperties.size(); i++)
                        {
                            // Поддерживает ли семейство графические команды
                            if(queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics){
                                queueFamilyGraphicsIndex = i;
                            }

                            // Поддерживает ли семейство представление
                            vk::Bool32 presentSupported = false;
                            physicalDevice.getSurfaceSupportKHR(i,surfaceKhr.get(),&presentSupported);
                            if(presentSupported) queueFamilyPresentIndex = i;
                        }

                        // Перейти к следующему устройству если необходимые возможности не поддерживаются
                        if(queueFamilyPresentIndex == -1 || queueFamilyGraphicsIndex == -1){
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
                        uint32_t queueFamilies[2] = {
                                static_cast<uint32_t>(queueFamilyGraphicsIndex_),
                                static_cast<uint32_t>(queueFamilyPresentIndex_)
                        };

                        // Используется ли для команд показа и представления одно и то же семейство
                        bool sameFamily = queueFamilyGraphicsIndex_ == queueFamilyPresentIndex_;

                        // Заполняем массив описаний создаваемых очередей
                        // Если семейство одно, то при помощи ОДНОЙ структуры DeviceQueueCreateInfo выделяем ДВЕ разные очереди
                        // В противном случае нужны ДВЕ структуры DeviceQueueCreateInfo выделяющие ОДНУ очередь для каждого семейства
                        for (int i = 0; i < (sameFamily ? 1 : 2); i++) {
                            vk::DeviceQueueCreateInfo queueCreateInfo = {};
                            queueCreateInfo.setQueueFamilyIndex(queueFamilies[i]);
                            queueCreateInfo.setQueueCount(queueFamilies[0] == queueFamilies[1] ? 2 : 1);
                            queueCreateInfo.setPQueuePriorities(nullptr);
                            queueCreateInfoEntries.push_back(queueCreateInfo);
                        }

                        // Особенности устройства (пока-что пустая структура)
                        vk::PhysicalDeviceFeatures deviceFeatures = {};

                        // Информация о создаваемом устройстве
                        vk::DeviceCreateInfo deviceCreateInfo{};
                        deviceCreateInfo.setQueueCreateInfoCount(queueCreateInfoEntries.size());
                        deviceCreateInfo.setPQueueCreateInfos(queueCreateInfoEntries.data());
                        deviceCreateInfo.setPpEnabledExtensionNames(!requireExtensions.empty() ? requireExtensions.data() : nullptr);
                        deviceCreateInfo.setEnabledExtensionCount(requireExtensions.size());
                        deviceCreateInfo.setPpEnabledLayerNames(!requireValidationLayers.empty() ? requireValidationLayers.data() : nullptr);
                        deviceCreateInfo.setEnabledLayerCount(requireValidationLayers.size());
                        deviceCreateInfo.setPEnabledFeatures(&deviceFeatures);

                        // Создание устройства
                        this->device_ = physicalDevice_.createDeviceUnique(deviceCreateInfo);

                        // Получение очередей
                        queueGraphics_ = device_->getQueue(this->queueFamilyGraphicsIndex_, 0);
                        queuePresent_ = device_->getQueue(this->queueFamilyPresentIndex_, sameFamily ? 1 : 0);

                        // Создание командного пула для графического семейства
                        commandPoolGraphics_ = device_->createCommandPoolUnique({
                                vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                                static_cast<uint32_t>(queueFamilyGraphicsIndex_)
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
                    // Уничтожить созданный командный пул
                    this->device_->destroyCommandPool(commandPoolGraphics_.get());
                    this->commandPoolGraphics_.release();

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
             * Получить командный пул графических команд
             * @return Константная ссылка на smart pointer объекта командного пула
             */
            const vk::UniqueCommandPool& getCommandGfxPool() const
            {
                return commandPoolGraphics_;
            }

            /**
             * Получить размер динамически выравненного блока в UBO с учетом лимитов устройства
             * @tparam T тип элемента передаваемого в UBO
             * @return Размер блока
             */
            template <typename T>
            vk::DeviceSize getDynamicallyAlignedUboBlockSize() const {
                if(!isReady_) return 0;
                auto minUboAlignment = physicalDevice_.getProperties().limits.minUniformBufferOffsetAlignment;
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

                auto formatProperties = this->physicalDevice_.getFormatProperties(format);
                if(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment){
                    return true;
                }

                return false;
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

                for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++){
                    if ((typeBits & 1) == 1){
                        if ((memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlag) == memoryPropertyFlag){
                            return i;
                        }
                    }
                    typeBits >>= 1;
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
