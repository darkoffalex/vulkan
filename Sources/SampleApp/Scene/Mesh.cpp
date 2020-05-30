#include "Mesh.h"

#include <utility>
#include <glm/glm.hpp>

namespace vk
{
    namespace scene
    {
        /**
         * Конструктор по умолчанию
         */
        Mesh::Mesh():SceneElement(),
        isReady_(false),
        pDevice_(nullptr),
        pDescriptorPool_(nullptr),
        pUboModelMatrixData_(nullptr),
        pUboMaterialData_(nullptr){}

        /**
         * Конструктор перемещения
         * @param other R-value ссылка на другой объект
         * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        Mesh::Mesh(Mesh &&other) noexcept :Mesh()
        {
            std::swap(isReady_,other.isReady_);
            std::swap(pDevice_,other.pDevice_);
            std::swap(pDescriptorPool_,other.pDescriptorPool_);
            std::swap(pUboModelMatrixData_, other.pUboModelMatrixData_);
            std::swap(pUboMaterialData_, other.pUboMaterialData_);
            geometryBufferPtr_.swap(other.geometryBufferPtr_);
            descriptorSet_.swap(other.descriptorSet_);
            uboModelMatrix_ = std::move(other.uboModelMatrix_);
            uboMaterial_ = std::move(other.uboMaterial_);
        }

        /**
         * Перемещение через присваивание
         * @param other R-value ссылка на другой объект
         * @return Ссылка на текущий объект
         */
        Mesh &Mesh::operator=(Mesh &&other) noexcept
        {
            if (this == &other) return *this;

            this->destroyVulkanResources();
            isReady_ = false;
            pDevice_ = nullptr;
            pDescriptorPool_ = nullptr;
            pUboModelMatrixData_ = nullptr;
            pUboMaterialData_ = nullptr;

            std::swap(isReady_,other.isReady_);
            std::swap(pDevice_,other.pDevice_);
            std::swap(pDescriptorPool_,other.pDescriptorPool_);
            std::swap(pUboModelMatrixData_, other.pUboModelMatrixData_);
            std::swap(pUboMaterialData_, other.pUboMaterialData_);
            geometryBufferPtr_.swap(other.geometryBufferPtr_);
            descriptorSet_.swap(other.descriptorSet_);
            uboModelMatrix_ = std::move(other.uboModelMatrix_);
            uboMaterial_ = std::move(other.uboMaterial_);

            return *this;
        }

        /**
         * Деструктор
         */
        Mesh::~Mesh()
        {
            this->destroyVulkanResources();
        }

        /**
         * Основной конструктор
         * @param pDevice Указатель на объект устройства
         * @param descriptorPool Unique smart pointer объекта дескрипторного пула
         * @param descriptorSetLayout Макет размещения дескрипторного набора меша
         * @param geometryBufferPtr Smart-pointer на объект геом. буфера
         */
        Mesh::Mesh(const vk::tools::Device* pDevice,
                   const vk::UniqueDescriptorPool& descriptorPool,
                   const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                   vk::tools::GeometryBufferPtr geometryBufferPtr):SceneElement(),
        isReady_(false),
        pDevice_(pDevice),
        pDescriptorPool_(&(descriptorPool.get())),
        geometryBufferPtr_(std::move(geometryBufferPtr))
        {
            // Проверить устройство
            if(pDevice_ == nullptr || !pDevice_->isReady()){
                throw vk::DeviceLostError("Device is not available");
            }

            // Выделить буфер для матрицы модели
            uboModelMatrix_ = vk::tools::Buffer(pDevice_,
                    sizeof(glm::mat4),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для параметров материала
            uboMaterial_ = vk::tools::Buffer(pDevice_,
                    sizeof(vk::scene::MeshMaterial),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Разметить память буферов, получив указатели
            pUboModelMatrixData_ = uboModelMatrix_.mapMemory();
            pUboMaterialData_ = uboMaterial_.mapMemory();

            // Выделить дескрипторный набор
            vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{};
            descriptorSetAllocateInfo.descriptorPool = descriptorPool.get();
            descriptorSetAllocateInfo.pSetLayouts = &(descriptorSetLayout.get());
            descriptorSetAllocateInfo.descriptorSetCount = 1;
            auto allocatedSets = pDevice_->getLogicalDevice()->allocateDescriptorSets(descriptorSetAllocateInfo);

            // Сохраняем набор
            descriptorSet_ = vk::UniqueDescriptorSet(allocatedSets[0]);

            // Информация о буферах
            std::vector<vk::DescriptorBufferInfo> bufferInfos = {
                    {uboModelMatrix_.getBuffer().get(),0,VK_WHOLE_SIZE},
                    {uboMaterial_.getBuffer().get(),0,VK_WHOLE_SIZE}
            };

            // Описываем связи дескрипторов с буферами (описание "записей")
            std::vector<vk::WriteDescriptorSet> writes = {
                    {
                            descriptorSet_.get(),
                            0,
                            0,
                            1,
                            vk::DescriptorType::eUniformBuffer,
                            nullptr,
                            bufferInfos.data() + 0,
                            nullptr
                    },
                    {
                            descriptorSet_.get(),
                            1,
                            0,
                            1,
                            vk::DescriptorType::eUniformBuffer,
                            nullptr,
                            bufferInfos.data() + 1,
                            nullptr
                    },
            };

            // Связываем дескрипторы с ресурсами (буферами)
            pDevice_->getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

            // Обновить UBO буферы
            this->updateMatrixBuffers();

            // Инициализация завершена
            isReady_ = true;
        }

        /**
         * Де-инициализация ресурсов Vulkan
         */
        void Mesh::destroyVulkanResources()
        {
            if(isReady_ && pDevice_!= nullptr && pDevice_->isReady())
            {
                // Вернуть в пул набор дескрипторов и освободить smart-pointer
                pDevice_->getLogicalDevice()->freeDescriptorSets(*pDescriptorPool_,{descriptorSet_.get()});
                descriptorSet_.release();

                // Очистить UBO буферы
                uboModelMatrix_.unmapMemory();
                uboModelMatrix_.destroyVulkanResources();
                uboMaterial_.unmapMemory();
                uboMaterial_.destroyVulkanResources();

                // Обнулить указатели
                pDevice_ = nullptr;
                pDescriptorPool_ = nullptr;
                pUboModelMatrixData_ = nullptr;
                pUboMaterialData_ = nullptr;

                // Объект де-инициализирован
                isReady_ = false;
            }
        }

        /**
         * Был ли объект инициализирован
         * @return Да или нет
         */
        bool Mesh::isReady() const {
            return isReady_;
        }

        /**
         * Получить буфер геометрии
         * @return Константная ссылка на smart-pointer объекта буфера
         */
        const vk::tools::GeometryBufferPtr &Mesh::getGeometryBuffer() const {
            return geometryBufferPtr_;
        }

        /**
         * Получить дескрипторный набор
         * @return Константная ссылка на объект дескрипторного набора
         */
        const vk::DescriptorSet &Mesh::getDescriptorSet() const {
            return descriptorSet_.get();
        }

        /**
         * Обновление буферов UBO
         */
        void Mesh::onMatricesUpdated()
        {
            this->updateMatrixBuffers();
        }

        /**
         * Обновление UBO буферов матриц модели
         */
        void Mesh::updateMatrixBuffers()
        {
            if(uboModelMatrix_.isReady()){
                auto modelMatrix = this->getModelMatrix();
                memcpy(pUboModelMatrixData_,&modelMatrix, sizeof(glm::mat4));
            }
        }
    }
}

