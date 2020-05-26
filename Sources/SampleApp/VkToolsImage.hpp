#pragma once

#include "VkTools.h"
#include "VkToolsDevice.hpp"

namespace vk
{
    namespace tools
    {
        /**
         * Класс обертка для работы с изображением (картинкой/текстурой) Vulkan
         */
        class Image
        {
        private:
            /// Готово ли изображение
            bool isReady_;
            /// Владеет ли объект объектом изображения Vulkan (если да, его нужно уничтожить в деструкторе)
            bool ownsImage_;
            /// Указатель на устройство владеющее изображением (создающее его)
            const vk::tools::Device* pDevice_;
            /// Изображение
            vk::UniqueImage image_;
            /// Объект для доступа к изображению
            vk::UniqueImageView imageView_;
            /// Память изображения
            vk::UniqueDeviceMemory deviceMemory_;

        public:
            /**
             * Получить тип объекта ImageView в зависимости от типа объекта Image
             * @param imageType Тип объекта Image
             * @param isArray Является ли изображение массивом текстур
             * @return Тип объекта ImageView
             */
            static vk::ImageViewType imageTypeToViewType(const vk::ImageType& imageType, bool isArray = false)
            {
                switch(imageType){
                    case vk::ImageType::e1D:
                    {
                        return isArray ? vk::ImageViewType::e1DArray : vk::ImageViewType::e1D;
                    }
                    case vk::ImageType::e2D:
                    default:
                    {
                        return isArray ? vk::ImageViewType::e2DArray : vk::ImageViewType::e2D;
                    }
                    case vk::ImageType::e3D:
                    {
                        return vk::ImageViewType::e3D;
                    }
                }
            }

        public:
            /**
             * Конструктор по умолчанию
             */
            Image():
            pDevice_(nullptr),
            isReady_(false),
            ownsImage_(false){};

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            Image(const Image& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            Image& operator=(const Image& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            Image(Image&& other) noexcept:Image(){
                std::swap(isReady_,other.isReady_);
                std::swap(ownsImage_, other.ownsImage_);
                std::swap(pDevice_,other.pDevice_);
                image_.swap(other.image_);
                imageView_.swap(other.imageView_);
                deviceMemory_.swap(other.deviceMemory_);
            }

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            Image& operator=(Image&& other) noexcept {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                isReady_ = false;
                ownsImage_ = false;
                pDevice_ = nullptr;

                std::swap(isReady_,other.isReady_);
                std::swap(ownsImage_, other.ownsImage_);
                std::swap(pDevice_,other.pDevice_);
                image_.swap(other.image_);
                imageView_.swap(other.imageView_);
                deviceMemory_.swap(other.deviceMemory_);

                return *this;
            }

            /**
             * Подробная инициализация (с выделением памяти)
             * @param pDevice Указатель на устройство
             * @param type Тип изображения (1D, 2D, 3D)
             * @param format Формат пикселей (цвета, их порядок, размер)
             * @param extent Разрешение (размеры) изображения
             * @param usage Флаг использования (в качестве чего будет использовано изображение)
             * @param subResourceRangeAspect Опция доступа к под-ресурсам (слоям) изображения
             * @param memoryProperties Настройка доступа (видимости) памяти (для устройства или хоста)
             * @param sharingMode Память используется одним семейством (exclusive) или несколькими (concurrent)
             * @param layout Изначальный макет размещения данных в памяти
             * @param imageTiling Упаковка данных текселей изображения в памяти
             */
            explicit Image(const vk::tools::Device* pDevice,
                           const vk::ImageType& type,
                           const vk::Format& format,
                           const vk::Extent3D& extent,
                           const vk::ImageUsageFlags& usage,
                           const vk::ImageAspectFlags& subResourceRangeAspect,
                           const vk::MemoryPropertyFlags& memoryProperties,
                           const vk::SharingMode& sharingMode = vk::SharingMode::eExclusive,
                           const vk::ImageLayout& layout = vk::ImageLayout::eUndefined,
                           const vk::ImageTiling& imageTiling = vk::ImageTiling::eOptimal):
                    isReady_(false),
                    ownsImage_(false),
                    pDevice_(pDevice)
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Создать изображение
                vk::ImageCreateInfo imageCreateInfo{};
                imageCreateInfo.imageType = type;
                imageCreateInfo.extent = extent;
                imageCreateInfo.format = format;
                imageCreateInfo.mipLevels = 1; // TODO: добавить параметр
                imageCreateInfo.arrayLayers = 1;
                imageCreateInfo.samples = vk::SampleCountFlagBits::e1; // TODO: добавить параметр
                imageCreateInfo.tiling = imageTiling;
                imageCreateInfo.sharingMode = sharingMode;
                imageCreateInfo.usage = usage;
                imageCreateInfo.initialLayout = layout;
                image_ = pDevice_->getLogicalDevice()->createImageUnique(imageCreateInfo);

                // Этот объект теперь владеет изображением (оно было создано при создании объекта)
                // Это означат его нужно будет уничтожить при уничтожении данного объекта
                ownsImage_ = true;

                // Получить требования к памяти, учитывая характеристики созданного объекта изображения
                auto memRequirements = pDevice_->getLogicalDevice()->getImageMemoryRequirements(image_.get());

                // Проверить доступность типа памяти
                auto memTypeIndex = pDevice_->getMemoryTypeIndex(memRequirements.memoryTypeBits,memoryProperties);
                if(memTypeIndex == -1){
                    throw vk::FeatureNotPresentError("Can't use required memory type");
                }

                // Выделить память для изображения (память на устройстве)
                vk::MemoryAllocateInfo memoryAllocateInfo{};
                memoryAllocateInfo.allocationSize = memRequirements.size;
                memoryAllocateInfo.memoryTypeIndex = static_cast<uint32_t>(memTypeIndex);
                deviceMemory_ = pDevice_->getLogicalDevice()->allocateMemoryUnique(memoryAllocateInfo);

                // Связать память и объект изображения
                pDevice_->getLogicalDevice()->bindImageMemory(image_.get(),deviceMemory_.get(),0);

                // Создать image-view объект (связь с конкретным слоем/мип-уровнем) изображения
                vk::ImageViewCreateInfo imageViewCreateInfo{};
                imageViewCreateInfo.viewType = imageTypeToViewType(imageCreateInfo.imageType, false);
                imageViewCreateInfo.format = format;
                imageViewCreateInfo.subresourceRange.levelCount = 1;
                imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
                imageViewCreateInfo.subresourceRange.layerCount = 1;
                imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
                imageViewCreateInfo.subresourceRange.aspectMask = subResourceRangeAspect;
                imageViewCreateInfo.image = image_.get();
                imageView_ = pDevice_->getLogicalDevice()->createImageViewUnique(imageViewCreateInfo);

                // Изображение инициализировано
                isReady_ = true;
            }

            /**
             * Альтернативная инициализация (с использованием готового объекта Image)
             * @param pDevice Указатель на устройство
             * @param image Готовый объект vk::Image с ассоциированной памятью
             * @param type Тип изображения (1D, 2D, 3D)
             * @param format Формат пикселей (цвета, их порядок, размер)
             * @param subResourceRangeAspect Опция доступа к под-ресурсам (слоям) изображения
             */
            explicit Image(const vk::tools::Device* pDevice,
                           const vk::Image& image,
                           const vk::ImageType& type,
                           const vk::Format& format,
                           const vk::ImageAspectFlags& subResourceRangeAspect):
                    isReady_(false),
                    ownsImage_(false),
                    pDevice_(pDevice)
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Добавить ранее созданное изображение
                image_ = vk::UniqueImage(image);

                // Поскольку используем готовое изображение, предполагается что оно ассоциировано с памятью и ее не создаем
                ownsImage_ = false;

                // Создать image-view объект (связь с конкретным слоем/мип-уровнем) изображения
                vk::ImageViewCreateInfo imageViewCreateInfo{};
                imageViewCreateInfo.viewType = imageTypeToViewType(type, false);
                imageViewCreateInfo.format = format;
                imageViewCreateInfo.subresourceRange.levelCount = 1;
                imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
                imageViewCreateInfo.subresourceRange.layerCount = 1;
                imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
                imageViewCreateInfo.subresourceRange.aspectMask = subResourceRangeAspect;
                imageViewCreateInfo.image = image_.get();
                imageView_ = pDevice_->getLogicalDevice()->createImageViewUnique(imageViewCreateInfo);

                // Изображение инициализировано
                isReady_ = true;
            }

            /**
             * Де-инициализация ресурсов Vulkan
             */
            void destroyVulkanResources()
            {
                // Если объект инициализирован и устройство доступно
                if(isReady_ && pDevice_!= nullptr && pDevice_->isReady())
                {
                    // Уничтожить image-view объект
                    pDevice_->getLogicalDevice()->destroyImageView(imageView_.get());
                    imageView_.release();

                    // Если объект владеет изображением - уничтожить его
                    // Иначе достаточно только освободить unique pointer объекта изображения
                    if(ownsImage_){
                        pDevice_->getLogicalDevice()->destroyImage(image_.get());
                    }
                    image_.release();

                    // Освободить память изображения
                    // Если объект владеет изображением, то память тоже выделялась при создании
                    if(ownsImage_){
                        pDevice_->getLogicalDevice()->freeMemory(deviceMemory_.get());
                        deviceMemory_.release();
                    }

                    isReady_ = false;
                }
            }

            /**
             * Деструктор
             */
            ~Image(){
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
             * Получить объект изображения Vulkan
             * @return ссылка на smart-pointer
             */
            vk::UniqueImage& getImage()
            {
                return image_;
            }

            /**
             * Получить объект image-view
             * @return ссылка на smart-pointer
             */
            vk::UniqueImageView& getImageView()
            {
                return imageView_;
            }

            /**
             * Получить объект памяти устройства
             * @return ссылка на smart-pointer
             */
            vk::UniqueDeviceMemory& getDeviceMemory()
            {
                return deviceMemory_;
            }
        };
    }
}

