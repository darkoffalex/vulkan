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

            /// Структура ускорения нижнего уровня (BLAS) для трассировки геометрии лучами (Ray Tracing)
            vk::UniqueAccelerationStructureKHR accelerationStructureKhr_;
            /// Память для структуры ускорения нижнего уровня
            vk::UniqueDeviceMemory accelerationStructureMemory_;

            /**
             * Построение структуры ускорения нижнего уровня (BLAS) для трассировки геометрии лучами (Ray Tracing)
             * @param buildFlags Флаги построения
             *
             * @details Структура ускорения нижнего уровня связана с самой геометрией. Структуру можно считать некой надстройкой над
             * обычным геометрическим буфером, которая позволяет ускорить перебор треугольников в нем за счет деления пространства и
             * группировки треугольников. Построение структуры происходит на устройстве
             */
            void buildBottomLevelAccelerationStructure(const vk::BuildAccelerationStructureFlagsKHR& buildFlags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
            {
                // Описание типа геометрии которая используется для построения BLAS
                std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR> geometryTypeInfos;
                vk::AccelerationStructureCreateGeometryTypeInfoKHR asCreateGeometryTypeInfo;
                asCreateGeometryTypeInfo.setGeometryType(vk::GeometryTypeKHR::eTriangles);
                asCreateGeometryTypeInfo.setIndexType(vk::IndexType::eUint32);
                asCreateGeometryTypeInfo.setVertexFormat(vk::Format::eR32G32B32Sfloat);
                asCreateGeometryTypeInfo.setMaxPrimitiveCount(static_cast<uint32_t>(indexCount_ / 3));
                asCreateGeometryTypeInfo.setMaxVertexCount(static_cast<uint32_t>(vertexCount_));
                asCreateGeometryTypeInfo.setAllowsTransforms(VK_FALSE);
                geometryTypeInfos.push_back(asCreateGeometryTypeInfo);

                // Адреса вершинного и индексного буфера
                vk::DeviceAddress vertexBufferAddress = pDevice_->getLogicalDevice()->getBufferAddress({vertexBuffer_.getBuffer().get()});
                vk::DeviceAddress indexBufferAddress  = pDevice_->getLogicalDevice()->getBufferAddress({indexBuffer_.getBuffer().get()});

                // Описание треугольников BLAS, связь с реальными буферами геометрии
                vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
                triangles.setVertexFormat(asCreateGeometryTypeInfo.vertexFormat);
                triangles.setVertexData(vertexBufferAddress);
                triangles.setVertexStride(sizeof(tools::Vertex));
                triangles.setIndexType(asCreateGeometryTypeInfo.indexType);
                triangles.setIndexData(indexBufferAddress);
                triangles.setTransformData({});

                // Описание геометрии
                std::vector<vk::AccelerationStructureGeometryKHR> geometries;
                vk::AccelerationStructureGeometryKHR asGeom;
                asGeom.setGeometryType(asCreateGeometryTypeInfo.geometryType);
                asGeom.setFlags(vk::GeometryFlagBitsKHR::eOpaque);
                asGeom.geometry.setTriangles(triangles);
                geometries.push_back(asGeom);

                // Смещение (по сути описание одного объекта)
                std::vector<vk::AccelerationStructureBuildOffsetInfoKHR> offsets;
                vk::AccelerationStructureBuildOffsetInfoKHR offset;
                offset.setFirstVertex(0);
                offset.setPrimitiveCount(asCreateGeometryTypeInfo.maxPrimitiveCount);
                offset.setPrimitiveOffset(0);
                offset.setTransformOffset(0);
                offsets.push_back(offset);

                // Создание структуры ускорения
                {
                    // 1 - Создать сам идентификатор структуры
                    vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
                    asCreateInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
                    asCreateInfo.setFlags(buildFlags);
                    asCreateInfo.setMaxGeometryCount(static_cast<uint32_t>(geometryTypeInfos.size()));
                    asCreateInfo.setPGeometryInfos(geometryTypeInfos.data());
                    accelerationStructureKhr_ = pDevice_->getLogicalDevice()->createAccelerationStructureKHRUnique(asCreateInfo);

                    // 2 - Получить требования к памяти
                    vk::AccelerationStructureMemoryRequirementsInfoKHR memReqInfo{};
                    memReqInfo.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice);
                    memReqInfo.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject);
                    memReqInfo.setAccelerationStructure(accelerationStructureKhr_.get());
                    auto memReq = pDevice_->getLogicalDevice()->getAccelerationStructureMemoryRequirementsKHR(memReqInfo);

                    // 3 - Выделение памяти
                    vk::MemoryAllocateFlagsInfoKHR memoryAllocateFlagsInfoKhr{};
                    memoryAllocateFlagsInfoKhr.setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress);

                    vk::MemoryAllocateInfo memoryAllocateInfo{};
                    memoryAllocateInfo.setAllocationSize(memReq.memoryRequirements.size);
                    memoryAllocateInfo.setMemoryTypeIndex(static_cast<uint32_t>(pDevice_->getMemoryTypeIndex(memReq.memoryRequirements.memoryTypeBits,vk::MemoryPropertyFlagBits::eDeviceLocal)));
                    //memoryAllocateInfo.setPNext(&memoryAllocateFlagsInfoKhr);
                    accelerationStructureMemory_ = pDevice_->getLogicalDevice()->allocateMemoryUnique(memoryAllocateInfo);

                    // 4 - связать память и BLAS
                    vk::BindAccelerationStructureMemoryInfoKHR bindInfo{};
                    bindInfo.setAccelerationStructure(accelerationStructureKhr_.get());
                    bindInfo.setMemory(accelerationStructureMemory_.get());
                    bindInfo.setMemoryOffset(0);
                    pDevice_->getLogicalDevice()->bindAccelerationStructureMemoryKHR({bindInfo});
                }

                // Получить требования к памяти рабочего буфера (используемого для построения BLAS)
                vk::AccelerationStructureMemoryRequirementsInfoKHR memReqInfoScratch{};
                memReqInfoScratch.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice);
                memReqInfoScratch.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch);
                memReqInfoScratch.setAccelerationStructure(accelerationStructureKhr_.get());
                auto memReqScratch = pDevice_->getLogicalDevice()->getAccelerationStructureMemoryRequirementsKHR(memReqInfoScratch);

                // Создать рабочий буфер
                vk::tools::Buffer scratchBuffer(pDevice_,
                        memReqScratch.memoryRequirements.size,
                        vk::BufferUsageFlagBits::eRayTracingKHR|vk::BufferUsageFlagBits::eShaderDeviceAddress,
                        vk::MemoryPropertyFlagBits::eDeviceLocal);

                // Адрес буфера
                vk::DeviceAddress scratchBufferAddress = pDevice_->getLogicalDevice()->getBufferAddress({scratchBuffer.getBuffer().get()});

                // Выделить командный буфер для исполнения команды построения
                vk::CommandBufferAllocateInfo commandBufferAllocateInfo{};
                commandBufferAllocateInfo.commandBufferCount = 1;
                commandBufferAllocateInfo.commandPool = pDevice_->getCommandComputePool().get();
                commandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
                auto cmdBuffers = pDevice_->getLogicalDevice()->allocateCommandBuffers(commandBufferAllocateInfo);

                // Начало записи команд в буфер
                cmdBuffers[0].begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});

                // Информация для команды построения
                const vk::AccelerationStructureGeometryKHR* pGeometry = geometries.data();
                vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeometryInfo{};
                asBuildGeometryInfo.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);
                asBuildGeometryInfo.setFlags(buildFlags);
                asBuildGeometryInfo.setUpdate(VK_FALSE);
                asBuildGeometryInfo.setSrcAccelerationStructure(nullptr);
                asBuildGeometryInfo.setDstAccelerationStructure(accelerationStructureKhr_.get());
                asBuildGeometryInfo.setGeometryArrayOfPointers(VK_FALSE);
                asBuildGeometryInfo.setGeometryCount(static_cast<uint32_t>(geometries.size()));
                asBuildGeometryInfo.setPpGeometries(&pGeometry);
                asBuildGeometryInfo.scratchData.setDeviceAddress(scratchBufferAddress);

                // Массив указателей на смещения
                std::vector<const vk::AccelerationStructureBuildOffsetInfoKHR*> pBuildOffset(offsets.size());
                for(size_t i = 0; i < offsets.size(); i++) pBuildOffset[i] = &(offsets[i]);

                // Барьер памяти
                vk::MemoryBarrier memoryBarrier{};
                memoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);
                memoryBarrier.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);

                // Запись команды построения BLAS, барьер и завершение
                cmdBuffers[0].buildAccelerationStructureKHR({asBuildGeometryInfo},pBuildOffset);
                cmdBuffers[0].pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,{},{memoryBarrier},{},{});
                cmdBuffers[0].end();

                // Отправить команду в очередь и подождать выполнения
                vk::SubmitInfo submitInfo{};
                submitInfo.commandBufferCount = cmdBuffers.size();
                submitInfo.pCommandBuffers = cmdBuffers.data();
                pDevice_->getComputeQueue().submit({submitInfo},{});
                pDevice_->getComputeQueue().waitIdle();

                // Очистить рабочий буфер
                scratchBuffer.destroyVulkanResources();
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

                accelerationStructureKhr_.swap(other.accelerationStructureKhr_);
                accelerationStructureMemory_.swap(other.accelerationStructureMemory_);
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

                accelerationStructureKhr_.swap(other.accelerationStructureKhr_);
                accelerationStructureMemory_.swap(other.accelerationStructureMemory_);

                return *this;
            }

            /**
             * Основной конструктор геометрического буфера
             * @param pDevice Указатель на устройство
             * @param vertices Массив вершин
             * @param indices Массив индексов
             */
            GeometryBuffer(const vk::tools::Device* pDevice, const std::vector<vk::tools::Vertex>& vertices, const std::vector<uint32_t>& indices):
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
                    // Данный буфер может быть также использован при трассировке лучей, как часть BLAS (флаг eRayTracingKHR)
                    vertexBuffer_ = vk::tools::Buffer(pDevice_,
                            sizeof(tools::Vertex) * vertexCount_,
                            vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eVertexBuffer|vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eShaderDeviceAddressKHR,
                            vk::MemoryPropertyFlagBits::eDeviceLocal);

                    // Заполнить временный буфер вершин
                    auto pStagingVertexBufferData = stagingVertexBuffer.mapMemory(0,sizeof(tools::Vertex) * vertexCount_);
                    memcpy(pStagingVertexBufferData,vertices.data(),sizeof(tools::Vertex) * vertexCount_);
                    stagingVertexBuffer.unmapMemory();

                    // Копировать из временного буфера в основной
                    this->pDevice_->copyBuffer(
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
                    // Данный буфер может быть также использован при трассировке лучей, как часть BLAS (флаг eRayTracingKHR)
                    indexBuffer_ = vk::tools::Buffer(pDevice_,
                            sizeof(size_t) * indexCount_,
                            vk::BufferUsageFlagBits::eTransferDst|vk::BufferUsageFlagBits::eIndexBuffer|vk::BufferUsageFlagBits::eStorageBuffer|vk::BufferUsageFlagBits::eShaderDeviceAddress,
                            vk::MemoryPropertyFlagBits::eDeviceLocal);

                    // Заполнить временный буфер индексов
                    auto pStagingIndexBufferData = stagingIndexBuffer.mapMemory();
                    memcpy(pStagingIndexBufferData,indices.data(),sizeof(size_t) * indexCount_);
                    stagingIndexBuffer.unmapMemory();

                    // Копировать из временного буфера в основной
                    this->pDevice_->copyBuffer(
                            stagingIndexBuffer.getBuffer().get(),
                            indexBuffer_.getBuffer().get(),
                            stagingIndexBuffer.getSize());

                    // Очищаем временный буфер (не обязательно, все равно очистится, но можно для ясности)
                    stagingIndexBuffer.destroyVulkanResources();
                }

                // Построить структуру ускорения нижнего уровня (BLAS) для трассировки лучей
                this->buildBottomLevelAccelerationStructure();

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

                    pDevice_->getLogicalDevice()->destroyAccelerationStructureKHR(accelerationStructureKhr_.get());
                    accelerationStructureKhr_.release();

                    pDevice_->getLogicalDevice()->freeMemory(accelerationStructureMemory_.get());
                    accelerationStructureMemory_.release();

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

            /**
             * Получить структуру ускорения
             * @return Константная ссылка на unique-smart-pointer
             */
            const vk::UniqueAccelerationStructureKHR& getAccelerationStructure() const
            {
                return accelerationStructureKhr_;
            }
        };

        /**
         * Smart-pointer объекта геометрического буфера
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при создании геом. буферов
         */
        typedef std::shared_ptr<GeometryBuffer> GeometryBufferPtr;
    }
}