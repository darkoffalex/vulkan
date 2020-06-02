#pragma once

#include "VkTools.h"
#include "VkToolsImage.hpp"

namespace vk
{
    namespace tools
    {
        enum TextureBufferType
        {
            e2D,
            e2DArray,
            eCubeMap
        };

        class TextureBuffer
        {
        private:
            /// Готово ли изображение
            bool isReady_;
            /// Указатель на устройство владеющее буфером
            const vk::tools::Device* pDevice_;
            /// Указатель на текстурный семплер
            const vk::UniqueSampler* pSampler_;
            /// Тип текстуры
            TextureBufferType type_;
            /// Размер по ширине
            size_t width_;
            /// Размер по высоте
            size_t height_;
            /// Байт на пиксель
            size_t bpp_;
            /// Буфер изображения
            vk::tools::Image image_;

            /**
             * Получить формат текстуры
             * @param bpp Байт на пиксель
             * @param sRGB Использовать sRGB пространство
             * @return Формат изображения
             */
            static vk::Format getImageFormat(size_t bpp, bool sRGB = false)
            {
                switch (bpp)
                {
                    case 1:
                        return vk::Format::eR8Unorm;
                    case 2:
                        return vk::Format::eR8G8Unorm;
                    case 3:
                    default:
                        return sRGB ? vk::Format::eR8G8B8Srgb : vk::Format::eR8G8B8Unorm;
                    case 4:
                        return sRGB ? vk::Format::eR8G8B8A8Srgb : vk::Format::eR8G8B8A8Unorm;
                }
            }

            /**
             * Копировать изображение из временного в основное
             * @param srcImage Исходное изображение (временное)
             * @param dstImage Целевое изображение (основное)
             * @param extent Разрешение изображения
             */
            void copyTmpToDst(const vk::Image& srcImage, const vk::Image& dstImage, const vk::Extent3D& extent)
            {
                // Выделить командный буфер для исполнения команды копирования
                vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
                commandBufferAllocateInfo.commandBufferCount = 1;
                commandBufferAllocateInfo.commandPool = pDevice_->getCommandGfxPool().get();
                commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
                auto cmdBuffers = pDevice_->getLogicalDevice()->allocateCommandBuffers(commandBufferAllocateInfo);

                // П О Д Г О Т О В К А  Р А З М Е Щ Е Н И Я  П А М Я Т И

                // Барьер памяти для смены размещения исходного (временного) изображения
                vk::ImageMemoryBarrier imageMemoryBarrierSrc{};
                imageMemoryBarrierSrc.image = srcImage;
                imageMemoryBarrierSrc.subresourceRange = {vk::ImageAspectFlagBits::eColor,0,0};
                imageMemoryBarrierSrc.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrierSrc.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrierSrc.oldLayout = vk::ImageLayout::ePreinitialized;
                imageMemoryBarrierSrc.newLayout = vk::ImageLayout::eTransferSrcOptimal;
                imageMemoryBarrierSrc.srcAccessMask = vk::AccessFlagBits::eHostWrite;
                imageMemoryBarrierSrc.dstAccessMask = vk::AccessFlagBits::eTransferRead;

                // Барьер памяти для смены размещения целевого изображения
                vk::ImageMemoryBarrier imageMemoryBarrierDst{};
                imageMemoryBarrierDst.image = dstImage;
                imageMemoryBarrierDst.subresourceRange = {vk::ImageAspectFlagBits::eColor,0,0};
                imageMemoryBarrierDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrierDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                imageMemoryBarrierDst.oldLayout = vk::ImageLayout::ePreinitialized;
                imageMemoryBarrierDst.newLayout = vk::ImageLayout::eTransferDstOptimal;
                imageMemoryBarrierDst.srcAccessMask = vk::AccessFlagBits::eHostWrite;
                imageMemoryBarrierDst.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

                // Запись команд в буфер
                cmdBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
                cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,vk::PipelineStageFlagBits::eTopOfPipe,{},{},{},imageMemoryBarrierSrc);
                cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe,vk::PipelineStageFlagBits::eTopOfPipe,{},{},{},imageMemoryBarrierDst);
                cmdBuffers[0].end();

                // Отправить команду в очередь и подождать выполнения, затем очистить буфер
                pDevice_->getGraphicsQueue().submit({vk::SubmitInfo(0, nullptr, nullptr,cmdBuffers.size(),cmdBuffers.data())},{});
                pDevice_->getGraphicsQueue().waitIdle();
                pDevice_->getLogicalDevice()->freeCommandBuffers(pDevice_->getCommandGfxPool().get(),cmdBuffers);

                // К О П И Р О В А Н И Е

                // Описание параметров под-ресурса
                vk::ImageSubresourceLayers subResourceLayers{};
                subResourceLayers.aspectMask = vk::ImageAspectFlagBits::eColor;
                subResourceLayers.baseArrayLayer = 0;
                subResourceLayers.mipLevel = 0;
                subResourceLayers.layerCount = 1;

                // Параметры копирования
                vk::ImageCopy imageCopy{};
                imageCopy.srcSubresource = subResourceLayers;
                imageCopy.dstSubresource = subResourceLayers;
                imageCopy.srcOffset = vk::Offset3D(0,0,0);
                imageCopy.dstOffset = vk::Offset3D(0,0,0);
                imageCopy.extent = extent;

                // Запись команд в буфер
                cmdBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
                cmdBuffers[0].copyImage(srcImage,vk::ImageLayout::eTransferSrcOptimal,dstImage,vk::ImageLayout::eTransferDstOptimal,imageCopy);
                cmdBuffers[0].end();

                // Отправить команду в очередь и подождать выполнения
                pDevice_->getGraphicsQueue().submit({vk::SubmitInfo(0, nullptr, nullptr,cmdBuffers.size(),cmdBuffers.data())},{});
                pDevice_->getGraphicsQueue().waitIdle();

                // Очищаем буфер команд
                pDevice_->getLogicalDevice()->freeCommandBuffers(pDevice_->getCommandGfxPool().get(),cmdBuffers);
            }

            /**
             * Копирование байт во временное изображение
             * @param pStagingImageData Указатель на размеченную область памяти во временном изображении
             * @param imageBytes Байты изображения
             * @param size Размер копируемого фрагмента в байтах
             * @param imageSubResourceLayout Макет размещение под-ресурса изображения
             */
            void copyBytesToTmp(void* pStagingImageData, const unsigned char* imageBytes, size_t size, const vk::SubresourceLayout& imageSubResourceLayout)
            {
                // Если размер ряда байт в под-ресурсе равен размеру ряда байт в изображении (ширина на bpp),
                // значит можно использовать обычный memcpy, ибо байты упакованы оптимально. Иначе, например если
                // размер изображения не кратен степени двойки, нужно копировать ряд за рядом по отдельности
                if(imageSubResourceLayout.rowPitch == width_ * bpp_){
                    memcpy(pStagingImageData,imageBytes,size);
                }
                else{
                    for(size_t i = 0; i < height_; i++)
                    {
                        memcpy(
                                reinterpret_cast<unsigned char*>(pStagingImageData) + (i * imageSubResourceLayout.rowPitch),
                                imageBytes + (i * width_ * bpp_),
                                width_ * bpp_);
                    }
                }
            }

        public:
            /**
             * Конструктор по умолчанию
             */
            TextureBuffer():
                    isReady_(false),
                    pDevice_(nullptr),
                    pSampler_(nullptr),
                    type_(TextureBufferType::e2D),
                    width_(0),height_(0),bpp_(0){};

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            TextureBuffer(const TextureBuffer& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            TextureBuffer& operator=(const TextureBuffer& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            TextureBuffer(TextureBuffer&& other) noexcept:TextureBuffer()
            {
                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(pSampler_, other.pSampler_);
                std::swap(type_, other.type_);
                std::swap(width_,other.width_);
                std::swap(height_,other.height_);
                std::swap(bpp_, other.bpp_);
                image_ = std::move(other.image_);
            }

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            TextureBuffer& operator=(TextureBuffer&& other) noexcept
            {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                isReady_ = false;
                pDevice_ = nullptr;
                pSampler_ = nullptr;
                type_ = TextureBufferType::e2D;
                width_ = 0;
                height_ = 0;
                bpp_ = 0;

                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(pSampler_, other.pSampler_);
                std::swap(type_, other.type_);
                std::swap(width_,other.width_);
                std::swap(height_,other.height_);
                std::swap(bpp_, other.bpp_);
                image_ = std::move(other.image_);

                return *this;
            }

            /**
             * Основной конструктор геометрического буфера
             * @param pDevice Указатель на устройство
             * @param pSampler Указатель на семплер
             * @param imageBytes Массив байт изображения
             * @param width Ширина изображения
             * @param height Высота изображения
             * @param bpp Байт на пиксель
             * @param sRGB Цветовое пространство sRGB
             */
            TextureBuffer(
                    const vk::tools::Device* pDevice,
                    const vk::UniqueSampler* pSampler,
                    const unsigned char* imageBytes,
                    size_t width,
                    size_t height,
                    size_t bpp,
                    bool sRGB = false):
                    pDevice_(pDevice),
                    pSampler_(pSampler),
                    isReady_(false),
                    type_(TextureBufferType::e2D),
                    width_(width),
                    height_(height),
                    bpp_(bpp)
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Проверить семплер (не нужен для инициализации, но нужен для использования в дальнейшем)
                if(pSampler_ == nullptr){
                    throw vk::InitializationFailedError("Sampler is not available");
                }

                // Создать промежуточное изображение (память хоста)
                vk::tools::Image stagingImage(
                        pDevice_,
                        vk::ImageType::e2D,                       //TODO: добавить зависимость от типа
                        this->getImageFormat(bpp_,sRGB),
                        {width_,height_,1},
                        vk::ImageUsageFlagBits::eTransferSrc,
                        vk::ImageAspectFlagBits::eColor,
                        vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent,
                        vk::SharingMode::eExclusive,
                        vk::ImageLayout::ePreinitialized,
                        vk::ImageTiling::eLinear);

                // Создать итоговое изображение (память устройства)
                image_ = vk::tools::Image(pDevice_,
                        vk::ImageType::e2D,                       //TODO: добавить зависимость от типа
                        this->getImageFormat(bpp_,sRGB),
                        {width_,height_,1},
                        vk::ImageUsageFlagBits::eTransferDst|vk::ImageUsageFlagBits::eSampled,
                        vk::ImageAspectFlagBits::eColor,
                        vk::MemoryPropertyFlagBits::eDeviceLocal,
                        vk::SharingMode::eExclusive,
                        vk::ImageLayout::ePreinitialized,
                        vk::ImageTiling::eOptimal);

                // Как и в случае с геометрическим буфером, данные текстур более оптимально хранить в памяти устройства,
                // но доступ к ней на прямую закрыт. Нужен промежуточный буфер (stagingImage)

                // Получить макет размещение под-ресурса
                auto imageSubResourceLayout = pDevice_->getLogicalDevice()->getImageSubresourceLayout(
                        stagingImage.getVulkanImage().get(),
                        {vk::ImageAspectFlagBits::eColor,0,0});

                // Разметить память
                size_t size = width_ * height_ * bpp_;
                auto pStagingImageData = stagingImage.mapMemory(0,size);
                // Копировать во временное изображение
                this->copyBytesToTmp(pStagingImageData,imageBytes,size,imageSubResourceLayout);
                // Убрать разметку
                stagingImage.unmapMemory();

                // Копировать из временного изображения в целевое
                this->copyTmpToDst(stagingImage.getVulkanImage().get(), image_.getVulkanImage().get(), {width, height, 1});

                // Очищаем временное изображение
                stagingImage.destroyVulkanResources();

                // Объект готов
                isReady_ = true;
            }

            /**
             * Де-инициализация ресурсов Vulkan
             */
            void destroyVulkanResources()
            {
                // Если объект инициализирован и устройство доступно
                if(isReady_)
                {
                    image_.destroyVulkanResources();
                    isReady_ = false;
                }
            }

            /**
             * Деструктор
             */
            ~TextureBuffer(){
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
             * Получить ширину
             * @return Целое положительное число
             */
            size_t geWidth() const
            {
                return width_;
            }

            /**
             * Получить высоту
             * @return Целое положительное число
             */
            size_t getHeight() const
            {
                return height_;
            }

            /**
             * Получить указатель на владеющее устройство
             * @return Константный указатель
             */
            const vk::tools::Device* getOwnerDevice() const
            {
                return pDevice_;
            }

            /**
             * Получить указатель на текстурный семплер
             * @return Константный указатель
             */
            const vk::UniqueSampler* getSampler() const
            {
                return pSampler_;
            }

            /**
             * Получить объект изображения
             * @return Константная ссылка на объект обертку изображения
             */
            const vk::tools::Image& getImage() const
            {
                return image_;
            }
        };

        /**
         * Smart-pointer объекта текстурного буфера
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при создании текстурного буферов
         */
        typedef std::shared_ptr<TextureBuffer> TextureBufferPtr;
    }
}
