#pragma once

#include "../VkTools/Tools.h"
#include "../VkTools/Buffer.hpp"

namespace vk
{
    namespace resources
    {
        class GeometryBuffer
        {
        private:
            /// Готово ли изображение
            bool isReady_;
            /// Индексированная геометрия
            bool isIndexed_;
            /// Указатель на устройство владеющее буфером геометрии
            const vk::tools::Device* pDevice_;
            /// Буфер вершин
            vk::tools::Buffer vertexBuffer_;
            /// Буфер индексов
            vk::tools::Buffer indexBuffer_;
            /// Кол-во вершин
            size_t vertexCount_;
            /// Кол-во индексов
            size_t indexCount_;

            /**
             * Копирование содержимого из временного буфера в основной
             * @param srcBuffer Исходный буфер
             * @param dstBuffer Целевой буфер
             * @param size Размер
             *
             * @details Функция использует команду копирования буфера. Для ее выполнения необходимо выделить командный буфер
             * и отправить его в очередь. Очередь должна поддерживать команды копирования данных, благо семейство команд рисования
             * по умолчанию поддерживает копирование данных, и нет нужны в отдельном семействе
             */
            void copyTmpToDst(const vk::Buffer& srcBuffer, const vk::Buffer& dstBuffer, vk::DeviceSize size)
            {
                // Выделить командный буфер для исполнения команды копирования
                vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
                commandBufferAllocateInfo.commandBufferCount = 1;
                commandBufferAllocateInfo.commandPool = pDevice_->getCommandGfxPool().get();
                commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
                auto cmdBuffers = pDevice_->getLogicalDevice()->allocateCommandBuffers(commandBufferAllocateInfo);

                // Записать команду в буфер
                cmdBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
                cmdBuffers[0].copyBuffer(srcBuffer,dstBuffer,vk::BufferCopy(0,0,size));
                cmdBuffers[0].end();

                // Отправить команду в очередь и подождать выполнения
                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = cmdBuffers.size();
                submitInfo.pCommandBuffers = cmdBuffers.data();
                pDevice_->getGraphicsQueue().submit({submitInfo},{});
                pDevice_->getGraphicsQueue().waitIdle();

                // Очищаем буфер команд
                pDevice_->getLogicalDevice()->free(pDevice_->getCommandGfxPool().get(),cmdBuffers.size(),cmdBuffers.data());
            }

        public:
            /**
             * Конструктор по умолчанию
             */
            GeometryBuffer():
                    isReady_(false),
                    isIndexed_(false),
                    pDevice_(nullptr),
                    vertexCount_(0),
                    indexCount_(0){};

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            GeometryBuffer(const GeometryBuffer& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            GeometryBuffer& operator=(const GeometryBuffer& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            GeometryBuffer(GeometryBuffer&& other) noexcept:GeometryBuffer()
            {
                std::swap(isReady_,other.isReady_);
                std::swap(isIndexed_,other.isIndexed_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(vertexCount_,other.vertexCount_);
                std::swap(indexCount_,other.indexCount_);

                vertexBuffer_ = std::move(other.vertexBuffer_);
                indexBuffer_ = std::move(other.indexBuffer_);
            }

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            GeometryBuffer& operator=(GeometryBuffer&& other) noexcept
            {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                isReady_ = false;
                isIndexed_ = false;
                pDevice_ = nullptr;
                vertexCount_ = 0;
                indexCount_ = 0;

                std::swap(isReady_,other.isReady_);
                std::swap(isIndexed_,other.isIndexed_);
                std::swap(pDevice_,other.pDevice_);
                std::swap(vertexCount_,other.vertexCount_);
                std::swap(indexCount_,other.indexCount_);

                vertexBuffer_ = std::move(other.vertexBuffer_);
                indexBuffer_ = std::move(other.indexBuffer_);

                return *this;
            }

            /**
             * Основной конструктор геометрического буфера
             * @param pDevice Указатель на устройство
             * @param vertices Массив вершин
             * @param indices Массив индексов
             */
            GeometryBuffer(const vk::tools::Device* pDevice, const std::vector<vk::tools::Vertex>& vertices, const std::vector<size_t>& indices):
                    isReady_(false),
                    isIndexed_(!indices.empty()),
                    pDevice_(pDevice),
                    vertexCount_(vertices.size()),
                    indexCount_(indices.size())
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }
                // Проверить вершины
                if(vertices.empty()){
                    throw vk::InitializationFailedError("No vertices provided");
                }

                // Данные о вершинах и индексах желательно располагать в памяти устройства, а не хоста,
                // но мы не можем напрямую помещать данные в память устройства. Однако, можем копировать из
                // буфера хоста (временного буфера) в буфер устройства.

                // Загрузка вершинного буфера в память устройства
                {
                    // Создать временный буфер вершин (память хоста)
                    vk::tools::Buffer stagingVertexBuffer = vk::tools::Buffer(pDevice_,
                            sizeof(tools::Vertex) * vertexCount_,
                            vk::BufferUsageFlagBits::eTransferSrc,
                            vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

                    // Создать основной буфер вершин (память устройства)
                    vertexBuffer_ = vk::tools::Buffer(pDevice_,
                            sizeof(tools::Vertex) * vertexCount_,
                            vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eVertexBuffer,
                            vk::MemoryPropertyFlagBits::eDeviceLocal);

                    // Заполнить временный буфер вершин
                    auto pStagingVertexBufferData = stagingVertexBuffer.mapMemory(0,sizeof(tools::Vertex) * vertexCount_);
                    memcpy(pStagingVertexBufferData,vertices.data(),sizeof(tools::Vertex) * vertexCount_);
                    stagingVertexBuffer.unmapMemory();

                    // Копировать из временного буфера в основной
                    this->copyTmpToDst(
                            stagingVertexBuffer.getBuffer().get(),
                            vertexBuffer_.getBuffer().get(),
                            stagingVertexBuffer.getSize());

                    // Очищаем временный буфер (не обязательно, все равно очистится, но можно для ясности)
                    stagingVertexBuffer.destroyVulkanResources();
                }


                // Загрузка буфера индексов в память (если индексы были переданы)
                if(isIndexed_)
                {
                    // Создать временный буфер индексов (память хоста)
                    vk::tools::Buffer stagingIndexBuffer = vk::tools::Buffer(pDevice_,
                            sizeof(size_t) * indexCount_,
                            vk::BufferUsageFlagBits::eTransferSrc,
                            vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

                    // Создать основной буфер вершин (память устройства)
                    indexBuffer_ = vk::tools::Buffer(pDevice_,
                            sizeof(size_t) * indexCount_,
                            vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eIndexBuffer,
                            vk::MemoryPropertyFlagBits::eDeviceLocal);

                    // Заполнить временный буфер индексов
                    auto pStagingIndexBufferData = stagingIndexBuffer.mapMemory();
                    memcpy(pStagingIndexBufferData,indices.data(),sizeof(size_t) * indexCount_);
                    stagingIndexBuffer.unmapMemory();

                    // Копировать из временного буфера в основной
                    this->copyTmpToDst(
                            stagingIndexBuffer.getBuffer().get(),
                            indexBuffer_.getBuffer().get(),
                            stagingIndexBuffer.getSize());

                    // Очищаем временный буфер (не обязательно, все равно очистится, но можно для ясности)
                    stagingIndexBuffer.destroyVulkanResources();
                }

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
                    vertexBuffer_.destroyVulkanResources();
                    indexBuffer_.destroyVulkanResources();
                    isReady_ = false;
                }
            }

            /**
             * Деструктор
             */
            ~GeometryBuffer(){
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
             * Индексированная ли это геометрия
             * @return Да или нет
             */
            bool isIndexed() const
            {
                return isIndexed_;
            }

            /**
             * Получить буфер вершин
             * @return Константная ссылка на объект-обертку буфера
             */
            const vk::tools::Buffer& getVertexBuffer() const
            {
                return vertexBuffer_;
            }

            /**
             * Получить буфер индексов
             * @return Константная ссылка на объект обертку буфера
             */
            const vk::tools::Buffer& getIndexBuffer() const
            {
                return indexBuffer_;
            }

            /**
             * Получить кол-во вершин
             * @return Целое положительное число
             */
            size_t getVertexCount() const
            {
                return vertexCount_;
            }

            /**
             * Получить кол-во индексов
             * @return Целое положительное число
             */
            size_t getIndexCount() const
            {
                return indexCount_;
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

        /**
         * Smart-pointer объекта геометрического буфера
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при создании геом. буферов
         */
        typedef std::shared_ptr<GeometryBuffer> GeometryBufferPtr;
    }
}