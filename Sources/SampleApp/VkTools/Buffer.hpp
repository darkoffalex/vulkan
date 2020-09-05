#pragma once

#include "Tools.h"
#include "Device.hpp"

namespace vk
{
    namespace tools
    {
        class Buffer
        {
        private:
            /// Готово ли изображение
            bool isReady_;
            /// Указатель на устройство владеющее буфером (создающее его)
            const vk::tools::Device* pDevice_;
            /// Буфер Vulkan (smart-pointer)
            vk::UniqueBuffer buffer_;
            /// Память буфера
            vk::UniqueDeviceMemory memory_;
            /// Размер буфера
            vk::DeviceSize size_;

        public:
            /**
             * Конструктор по умолчанию
             */
            Buffer():isReady_(false),pDevice_(nullptr),size_(0){};

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            Buffer(const Buffer& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            Buffer& operator=(const Buffer& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            Buffer(Buffer&& other) noexcept:Buffer(){
                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(size_,other.size_);
                buffer_.swap(other.buffer_);
                memory_.swap(other.memory_);
            }

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            Buffer& operator=(Buffer&& other) noexcept {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                isReady_ = false;
                pDevice_ = nullptr;
                size_ = 0;

                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(size_,other.size_);
                buffer_.swap(other.buffer_);
                memory_.swap(other.memory_);

                return *this;
            }

            /**
             * Основной конструктор
             * @param pDevice Указатель на устройство
             * @param size Размер буфера
             * @param usageFlags Флаги использования (назначения) буфера
             * @param memoryPropertyFlags Тип и доступ к памяти (память устройства, хоста, видима ли хостом и тд.)
             * @param memoryRequirements Требования к памяти. Если передан указатель на структуры будут использованы они
             */
            Buffer(const vk::tools::Device* pDevice,
                   const vk::DeviceSize& size,
                   const vk::BufferUsageFlags& usageFlags,
                   const vk::MemoryPropertyFlags& memoryPropertyFlags,
                   vk::MemoryRequirements* memoryRequirements = nullptr):
                    isReady_(false),
                    pDevice_(pDevice),
                    size_(size)
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Создать объект буфера Vulkan
                vk::BufferCreateInfo bufferCreateInfo{};
                bufferCreateInfo.size = size_;
                bufferCreateInfo.usage = usageFlags;
                bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;    //TODO: добавить параметр
                bufferCreateInfo.queueFamilyIndexCount = 0;                    //TODO: добавить параметр
                bufferCreateInfo.pQueueFamilyIndices = nullptr;                //TODO: добавить параметр
                bufferCreateInfo.flags = {};
                buffer_ = pDevice->getLogicalDevice()->createBufferUnique(bufferCreateInfo);

                // Получить требования к памяти для данного буфера
                const auto memRequirements = (memoryRequirements != nullptr ? *memoryRequirements : pDevice->getLogicalDevice()->getBufferMemoryRequirements(buffer_.get()));

                // Получить индекс типа памяти
                const auto memoryTypeIndex = pDevice->getMemoryTypeIndex(memRequirements.memoryTypeBits,memoryPropertyFlags);
                if(memoryTypeIndex == -1) throw vk::FeatureNotPresentError("Can't use required memory type");

                // Выделить память для буфера
                vk::MemoryAllocateInfo memoryAllocateInfo;
                memoryAllocateInfo.allocationSize = memRequirements.size;
                memoryAllocateInfo.memoryTypeIndex = static_cast<uint32_t>(memoryTypeIndex);

                // Флаги выделения памяти - eDeviceAddress, используются если буфер создается с флагом eShaderDeviceAddress
                vk::MemoryAllocateFlagsInfoKHR memoryAllocateFlagsInfoKhr{};
                memoryAllocateFlagsInfoKhr.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;

                if(usageFlags & vk::BufferUsageFlagBits::eShaderDeviceAddress){
                    memoryAllocateInfo.pNext = &memoryAllocateFlagsInfoKhr;
                }

                memory_ = pDevice->getLogicalDevice()->allocateMemoryUnique(memoryAllocateInfo);

                // Связать объект буфера и память
                pDevice->getLogicalDevice()->bindBufferMemory(buffer_.get(),memory_.get(),0);

                // Буфер инициализирован
                isReady_ = true;
            };

            /**
             * Де-инициализация ресурсов Vulkan
             */
            void destroyVulkanResources()
            {
                // Если объект инициализирован и устройство доступно
                if(isReady_ && pDevice_!= nullptr && pDevice_->isReady())
                {
                    // Уничтожить буфер
                    pDevice_->getLogicalDevice()->destroyBuffer(buffer_.get());
                    buffer_.release();

                    // Освободить память
                    pDevice_->getLogicalDevice()->freeMemory(memory_.get());
                    memory_.release();

                    isReady_ = false;
                }
            }

            /**
             * Деструктор
             */
            ~Buffer(){
                destroyVulkanResources();
            }

            /**
             * Был ли объект инициализирован
             * @return Да или нет
             */
            bool isReady() const
            {
                return isReady_;
            }

            /**
             * Разметить память и получить указатель на размеченную область
             * @param offset Сдвиг в байтах
             * @param size Размер в байтах
             * @return Указатель на область
             */
            void* mapMemory(VkDeviceSize offset = 0, vk::DeviceSize size = VK_WHOLE_SIZE)
            {
                if(pDevice_ == nullptr || !pDevice_->isReady()) return nullptr;
                return pDevice_->getLogicalDevice()->mapMemory(memory_.get(),offset,size);
            }

            /**
             * Отменить разметку памяти
             */
            void unmapMemory()
            {
                pDevice_->getLogicalDevice()->unmapMemory(memory_.get());
            }

            /**
             * Получить ресурс буфера Vulkan
             * @return ссылка на smart-pointer
             */
            const vk::UniqueBuffer& getBuffer() const
            {
                return buffer_;
            }

            /**
             * Получить ресурс памяти Vulkan
             * @return ссылка на smart-pointer
             */
            const vk::UniqueDeviceMemory& getMemory() const
            {
                return memory_;
            }

            /**
             * Получить размер буфера
             * @return значение
             */
            vk::DeviceSize getSize() const
            {
                return size_;
            }
        };
    }
}

