#pragma once

#include "VkTools.h"
#include "VkToolsImage.hpp"

namespace vk
{
    namespace tools
    {
        /**
         * Класс обертка для работы с кадровым буфером Vulkan
         */
        class FrameBuffer
        {
        private:
            /// Готов ли кадровый буфер
            bool isReady_;

            /// Указатель на устройство владеющее изображением (создающее его)
            const vk::tools::Device* pDevice_;

            /// Разрешение
            vk::Extent3D extent_;

            /// Объект кадрового буфера Vulkan (smart pointer)
            vk::UniqueFramebuffer frameBuffer_;

            /// Цветовое вложение
            struct Attachment{
                vk::UniqueImage image;
                vk::UniqueImageView imageView;
            } colorAttachment_;

            /// Вложение глубины трафарета
            vk::tools::Image depthStencilAttachment_;

        public:
            /**
             * Конструктор по умолчанию
             */
            FrameBuffer():isReady_(false),pDevice_(nullptr){}

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            FrameBuffer(const FrameBuffer& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            FrameBuffer& operator=(const FrameBuffer& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            FrameBuffer(FrameBuffer&& other) noexcept:FrameBuffer(){
                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(extent_, other.extent_);
                frameBuffer_.swap(other.frameBuffer_);
                colorAttachment_.image.swap(other.colorAttachment_.image);
                colorAttachment_.imageView.swap(other.colorAttachment_.imageView);
                depthStencilAttachment_ = std::move(other.depthStencilAttachment_);
            }

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            FrameBuffer& operator=(FrameBuffer&& other) noexcept {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                isReady_ = false;
                pDevice_ = nullptr;

                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(extent_,other.extent_);
                frameBuffer_.swap(other.frameBuffer_);
                colorAttachment_.image.swap(other.colorAttachment_.image);
                colorAttachment_.imageView.swap(other.colorAttachment_.imageView);
                depthStencilAttachment_ = std::move(other.depthStencilAttachment_);

                return *this;
            }

            /**
             * Основной конструктор
             * @param pDevice Указатель на устройство создающее кадровый буфер (и владеющее им)
             * @param image Полученное от swap-chain изображение цветового вложения
             * @param colorAttImageFormat Формат цветового вложения
             * @param depthStencilAttFormat Формат вложения глубины-трафарета
             * @param extent Расширение (разрешение) буфера
             * @param renderPass Проход рендеринга
             */
            explicit FrameBuffer(
                    const vk::tools::Device* pDevice,
                    const vk::Image& image,
                    const vk::Format& colorAttImageFormat,
                    const vk::Format& depthStencilAttFormat,
                    const vk::Extent3D& extent,
                    const vk::UniqueRenderPass& renderPass):
            pDevice_(pDevice),
            extent_(extent)
            {
                // Сохранить полученный объект изображения
                colorAttachment_.image = vk::UniqueImage(image);

                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Создать image-view для цветового вложения
                vk::ImageViewCreateInfo imageViewCreateInfo{};
                imageViewCreateInfo.image = image;
                imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
                imageViewCreateInfo.format = colorAttImageFormat;
                imageViewCreateInfo.components = {vk::ComponentSwizzle::eR,vk::ComponentSwizzle::eG,vk::ComponentSwizzle::eB,vk::ComponentSwizzle::eA};
                imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
                imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
                imageViewCreateInfo.subresourceRange.levelCount = 1;
                imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
                imageViewCreateInfo.subresourceRange.layerCount = 1;
                colorAttachment_.imageView = pDevice_->getLogicalDevice()->createImageViewUnique(imageViewCreateInfo);

                // Создать буфер глубины трафарета
                depthStencilAttachment_ = vk::tools::Image(
                        pDevice_,
                        vk::ImageType::e2D,
                        depthStencilAttFormat,
                        extent,
                        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferSrc,
                        vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil,
                        vk::MemoryPropertyFlagBits::eDeviceLocal,
                        pDevice_->isPresentAndGfxQueueFamilySame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent);

                // Массив вложений (1 цветовое, 1 глубины-трафарета)
                std::vector<vk::ImageView> attachments = {
                        colorAttachment_.imageView.get(),
                        depthStencilAttachment_.getImageView().get()
                };

                // Создать объект кадрового буфера Vulkan
                vk::FramebufferCreateInfo frameBufferCreateInfo{};
                frameBufferCreateInfo.renderPass = renderPass.get();
                frameBufferCreateInfo.attachmentCount = attachments.size();
                frameBufferCreateInfo.pAttachments = attachments.data();
                frameBufferCreateInfo.width = extent_.width;
                frameBufferCreateInfo.height = extent_.height;
                frameBufferCreateInfo.layers = 1;
                frameBuffer_ = pDevice_->getLogicalDevice()->createFramebufferUnique(frameBufferCreateInfo);

                // Объект инициализирован
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
                    // Удалить созданный image-view
                    pDevice_->getLogicalDevice()->destroyImageView(colorAttachment_.imageView.get());
                    colorAttachment_.imageView.release();

                    // Отвязать image объект об smart-pointer'а (дабы не вызвать авто-удаление объекта которым владеет swap-chain)
                    colorAttachment_.image.release();

                    // Уничтожить выделенные Vulkan ресурсы объекта изображения глубины-трафарета
                    depthStencilAttachment_.destroyVulkanResources();

                    // Удалить созданный объект кадрового буфера
                    pDevice_->getLogicalDevice()->destroyFramebuffer(frameBuffer_.get());
                    frameBuffer_.release();

                    isReady_ = false;
                }
            }

            /**
             * Деструктор
             */
            ~FrameBuffer()
            {
                destroyVulkanResources();
            }

            /**
             * Получить разрешение
             * @return объект структуры Extent3D
             */
            vk::Extent3D getExtent() const
            {
                return extent_;
            }

            /**
             * Получить кадровый буфер Vulkan
             * @return ссылка на unique smart pointer объекта буфера
             */
            const vk::UniqueFramebuffer& getVulkanFrameBuffer() const
            {
                return frameBuffer_;
            }
        };
    }
}


