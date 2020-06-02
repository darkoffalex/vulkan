#pragma once

#include "../VkTools/Tools.h"
#include "../VkTools/Image.hpp"

namespace vk
{
    namespace resources
    {
        /**
         * Инициализирующая структура описывающая вложение кадрового буфера
         */
        struct FrameBufferAttachmentInfo
        {
            // Если передан указать на объект изображения - оно не будет создаваться
            // Это полезно если у нас уже есть изображение (например из swap-chain)
            const vk::Image* pImage = nullptr;
            // Тип изображения (1D, 2D, 3D)
            vk::ImageType imageType = vk::ImageType::e2D;
            // Формат изображения
            vk::Format format;
            // Флаг использования (в качестве чего будет использовано изображение, используется если pImage равен nullptr)
            vk::ImageUsageFlags usageFlags;
            // Флаг доступа к под-ресурсам (слоям) изображения
            vk::ImageAspectFlags aspectFlags;
        };

        /**
         * Класс обертка для работы с кадровым буфером Vulkan
         */
        class FrameBuffer
        {
        private:
            /// Готов ли кадровый буфер
            bool isReady_;

            /// Указатель на устройство владеющее кадровым буфером (создающее его)
            const vk::tools::Device* pDevice_;

            /// Разрешение
            vk::Extent3D extent_;

            /// Объект кадрового буфера Vulkan (smart pointer)
            vk::UniqueFramebuffer frameBuffer_;

            /// Массив вложений (изображений) кадрового буфера
            std::vector<vk::tools::Image> attachments_;

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
                attachments_.swap(other.attachments_);
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
                attachments_.swap(other.attachments_);

                return *this;
            }

            /**
             * Основной конструктор кадрового буфера
             * @param pDevice Указатель на устройство создающее кадровый буфер (и владеющее им)
             * @param renderPass Целевой проход рендеринга, в котором будет использован данный кадровый буфер
             * @param extent Расширение (разрешение) буфера
             * @param attachmentsInfo Массив структур описывающих вложения
             */
            explicit FrameBuffer(
                const vk::tools::Device* pDevice,
                const vk::UniqueRenderPass& renderPass,
                const vk::Extent3D& extent,
                const std::vector<vk::resources::FrameBufferAttachmentInfo>& attachmentsInfo):
        	isReady_(false),
        	pDevice_(pDevice),
        	extent_(extent)
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Массив объектов imageView вложений для создания кадрового буфера
                std::vector<vk::ImageView> attachmentsImageViews;

                // Пройти по всем объектам инициализации вложений буфера
                for(const auto& info : attachmentsInfo)
                {
                    // Если объект изображения не был передан
                    if(info.pImage == nullptr){
                        // Создать вложение создавая изображение и выделяя память
                        attachments_.emplace_back(vk::tools::Image(
                                pDevice_,
                                info.imageType,
                                info.format,
                                extent,
                                info.usageFlags,
                                info.aspectFlags,
                                vk::MemoryPropertyFlagBits::eDeviceLocal,
                                pDevice_->isPresentAndGfxQueueFamilySame() ? vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent));
                    }
                    // Если объект изображения был передан
                    else{
                        attachments_.emplace_back(vk::tools::Image(
                                pDevice_,
                                *(info.pImage),
                                info.imageType,
                                info.format,
                                info.aspectFlags));
                    }

                    // Добавить image-view объект в массив
                    attachmentsImageViews.push_back(attachments_.back().getImageView().get());
                }

                // Создать объект кадрового буфера Vulkan
                vk::FramebufferCreateInfo frameBufferCreateInfo{};
                frameBufferCreateInfo.renderPass = renderPass.get();
                frameBufferCreateInfo.attachmentCount = attachmentsImageViews.size();
                frameBufferCreateInfo.pAttachments = attachmentsImageViews.data();
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
                    // Очистка массива вложений (деструкторы объектов vk::tools::Image очистят ресурсы Vulkan)
                    attachments_.clear();

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

            /**
             * Получить список объектов вложений
             * @return ссылка на массив изображений
             */
            const std::vector<vk::tools::Image>& getAttachmentImages() const
            {
                return attachments_;
            }

            /**
             * Получить указатель на владеющее устройство
             * @return Константный указатель
             */
            const vk::tools::Device* getOwnerDevice() const
            {
                return pDevice_;
            }
        };
    }
}


