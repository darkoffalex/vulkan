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
        pUboMaterialData_(nullptr),
        pUboTextureMappingData_(nullptr){}

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
            std::swap(pUboTextureMappingData_, other.pUboTextureMappingData_);
            std::swap(materialSettings_,other.materialSettings_);
            std::swap(textureMapping_,other.textureMapping_);
            geometryBufferPtr_.swap(other.geometryBufferPtr_);
            textureBufferPtr_.swap(other.textureBufferPtr_);
            descriptorSet_.swap(other.descriptorSet_);
            uboModelMatrix_ = std::move(other.uboModelMatrix_);
            uboMaterial_ = std::move(other.uboMaterial_);
            uboTextureMapping_ = std::move(other.uboTextureMapping_);
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
            pUboTextureMappingData_ = nullptr;
            materialSettings_ = {};
            textureMapping_ = {};

            std::swap(isReady_,other.isReady_);
            std::swap(pDevice_,other.pDevice_);
            std::swap(pDescriptorPool_,other.pDescriptorPool_);
            std::swap(pUboModelMatrixData_, other.pUboModelMatrixData_);
            std::swap(pUboMaterialData_, other.pUboMaterialData_);
            std::swap(pUboTextureMappingData_, other.pUboTextureMappingData_);
            std::swap(materialSettings_,other.materialSettings_);
            std::swap(textureMapping_,other.textureMapping_);
            geometryBufferPtr_.swap(other.geometryBufferPtr_);
            textureBufferPtr_.swap(other.textureBufferPtr_);
            descriptorSet_.swap(other.descriptorSet_);
            uboModelMatrix_ = std::move(other.uboModelMatrix_);
            uboMaterial_ = std::move(other.uboMaterial_);
            uboTextureMapping_ = std::move(other.uboTextureMapping_);

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
         * @param materialSettings Параметры материала
         * @param textureMappingSettings Параметры отображения текстуры
         */
        Mesh::Mesh(const vk::tools::Device* pDevice,
                   const vk::UniqueDescriptorPool& descriptorPool,
                   const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                   vk::resources::GeometryBufferPtr geometryBufferPtr,
                   vk::resources::TextureBufferPtr textureBufferPtr,
                   const vk::scene::MeshMaterialSettings& materialSettings,
                   const vk::scene::MeshTextureMapping& textureMappingSettings): SceneElement(),
        isReady_(false),
        pDevice_(pDevice),
        pDescriptorPool_(&(descriptorPool.get())),
        geometryBufferPtr_(std::move(geometryBufferPtr)),
        textureBufferPtr_(std::move(textureBufferPtr)),
        materialSettings_(materialSettings)
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
                    MATERIAL_UBO_SIZE,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для параметров отображения текстуры
            uboTextureMapping_ = vk::tools::Buffer(pDevice_,
                    sizeof(vk::scene::MeshTextureMapping),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Разметить память буферов, получив указатели
            pUboModelMatrixData_ = uboModelMatrix_.mapMemory();
            pUboMaterialData_ = uboMaterial_.mapMemory();
            pUboTextureMappingData_ = uboTextureMapping_.mapMemory();

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
                    {uboTextureMapping_.getBuffer().get(),0,VK_WHOLE_SIZE},
                    {uboMaterial_.getBuffer().get(),0,VK_WHOLE_SIZE}
            };

            // Информация об изображении (изначально пуста)
            vk::DescriptorImageInfo descriptorImageInfo{};

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
                    {
                            descriptorSet_.get(),
                            2,
                            0,
                            1,
                            vk::DescriptorType::eUniformBuffer,
                            nullptr,
                            bufferInfos.data() + 2,
                            nullptr
                    },
            };

            // Если есть текстура
            if(textureBufferPtr_.get() != nullptr)
            {
                // Заполнить информацию об изображении
                descriptorImageInfo.sampler = textureBufferPtr_->getSampler()->get();
                descriptorImageInfo.imageView = textureBufferPtr_->getImage().getImageView().get();
                descriptorImageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

                // Связь дескриптора с изображением
                writes.emplace_back(vk::WriteDescriptorSet(
                        descriptorSet_.get(),
                        3,
                        0,
                        1,
                        vk::DescriptorType::eCombinedImageSampler,
                        &descriptorImageInfo,
                        nullptr,
                        nullptr
                        ));
            }

            // Связываем дескрипторы с ресурсами (буферами и изображениями)
            pDevice_->getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

            // Обновить UBO буферы
            this->updateMatrixBuffers();
            this->updateMaterialSettingsBuffers();
            this->updateTextureMappingBuffers();

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

                uboTextureMapping_.unmapMemory();
                uboTextureMapping_.destroyVulkanResources();

                // Обнулить указатели
                pDevice_ = nullptr;
                pDescriptorPool_ = nullptr;
                pUboModelMatrixData_ = nullptr;
                pUboMaterialData_ = nullptr;
                pUboTextureMappingData_ = nullptr;

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
        const vk::resources::GeometryBufferPtr &Mesh::getGeometryBuffer() const {
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
         * Событие смены положения
         * @param updateMatrices Запрос обновить матрицы
         */
        void Mesh::onPlacementUpdated(bool updateMatrices)
        {
            if(updateMatrices){
                this->updateModelMatrix();
                this->updateMatrixBuffers();
            }
        }

        /**
         * Обновление UBO буферов матриц модели
         */
        void Mesh::updateMatrixBuffers()
        {
            if(uboModelMatrix_.isReady()){
                memcpy(pUboModelMatrixData_,&(this->getModelMatrix()), sizeof(glm::mat4));
            }
        }

        /**
         * Обновление UBO буферов параметров материала меша
         */
        void Mesh::updateMaterialSettingsBuffers()
        {
            if(uboMaterial_.isReady()){
                // Преобразование указателя (для возможности указывать побайтовое смещение)
                auto pData = reinterpret_cast<unsigned char*>(pUboMaterialData_);

                // Копирование в буфер (с учетом выравнивания std140)
                memcpy(pData + 0,&materialSettings_.ambientColor,16);
                memcpy(pData + 16,&materialSettings_.diffuseColor,16);
                memcpy(pData + 32, &materialSettings_.specularColor,16);
                memcpy(pData + 48, &materialSettings_.shininess,4);
            }
        }

        /**
         * Обновление UBO буферов параметров отображения текстуры меша
         */
        void Mesh::updateTextureMappingBuffers()
        {
            if(uboTextureMapping_.isReady()){
                memcpy(pUboTextureMappingData_,&textureMapping_, sizeof(vk::scene::MeshTextureMapping));
            }
        }

        /**
         * Установить параметры материала
         * @param settings Параметры материала
         */
        void Mesh::setMaterialSettings(const MeshMaterialSettings &settings) {
            materialSettings_ = settings;
            this->updateMaterialSettingsBuffers();
        }

        /**
         * Получить параметры материала
         * @return Структура описывающая материал
         */
        MeshMaterialSettings Mesh::getMaterialSettings() const {
            return materialSettings_;
        }

        /**
         * Установить параметры отображения текстуры
         * @param textureMapping Параметры отображения текстуры
         */
        void Mesh::setTextureMapping(const MeshTextureMapping &textureMapping)
        {
            textureMapping_ = textureMapping;
            this->updateTextureMappingBuffers();
        }

        /**
         * Получить параметры отображения текстуры
         * @return Структура описывающая параметры отображения текстуры
         */
        MeshTextureMapping Mesh::getTextureMapping() const {
            return textureMapping_;
        }
    }
}

