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
        pUboTextureMappingData_(nullptr),
        pUboTextureUsageData_(nullptr),
        pUboBoneCountData_(nullptr),
        pUboBoneTransformsData_(nullptr),
        skeleton_(nullptr){}

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
            std::swap(pUboTextureUsageData_,other.pUboTextureUsageData_);
            std::swap(pUboBoneCountData_,other.pUboBoneCountData_);
            std::swap(pUboBoneTransformsData_, other.pUboBoneTransformsData_);
            std::swap(materialSettings_,other.materialSettings_);
            std::swap(textureMapping_,other.textureMapping_);
            std::swap(textureSet_, other.textureSet_);
            std::swap(skeleton_, other.skeleton_);

            geometryBufferPtr_.swap(other.geometryBufferPtr_);
            descriptorSet_.swap(other.descriptorSet_);
            uboModelMatrix_ = std::move(other.uboModelMatrix_);
            uboMaterial_ = std::move(other.uboMaterial_);
            uboTextureMapping_ = std::move(other.uboTextureMapping_);
            uboTextureUsage_ = std::move(other.uboTextureUsage_);
            uboBoneCount_ = std::move(other.uboBoneCount_);
            uboBoneTransforms_ = std::move(other.uboBoneTransforms_);
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
            pUboTextureUsageData_ = nullptr;
            pUboBoneCountData_ = nullptr;
            pUboBoneTransformsData_ = nullptr;
            materialSettings_ = {};
            textureMapping_ = {};

            std::swap(isReady_,other.isReady_);
            std::swap(pDevice_,other.pDevice_);
            std::swap(pDescriptorPool_,other.pDescriptorPool_);
            std::swap(pUboModelMatrixData_, other.pUboModelMatrixData_);
            std::swap(pUboMaterialData_, other.pUboMaterialData_);
            std::swap(pUboTextureMappingData_, other.pUboTextureMappingData_);
            std::swap(pUboTextureUsageData_,other.pUboTextureUsageData_);
            std::swap(pUboBoneCountData_,other.pUboBoneCountData_);
            std::swap(pUboBoneTransformsData_, other.pUboBoneTransformsData_);
            std::swap(materialSettings_,other.materialSettings_);
            std::swap(textureMapping_,other.textureMapping_);
            std::swap(textureSet_, other.textureSet_);
            std::swap(skeleton_,other.skeleton_);

            geometryBufferPtr_.swap(other.geometryBufferPtr_);
            descriptorSet_.swap(other.descriptorSet_);
            uboModelMatrix_ = std::move(other.uboModelMatrix_);
            uboMaterial_ = std::move(other.uboMaterial_);
            uboTextureMapping_ = std::move(other.uboTextureMapping_);
            uboTextureUsage_ = std::move(other.uboTextureUsage_);
            uboBoneCount_ = std::move(other.uboBoneCount_);
            uboBoneTransforms_ = std::move(other.uboBoneTransforms_);

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
         * @param descriptorSetLayout Unique smart pointer макета размещения дескрипторного набора меша
         * @param geometryBufferPtr Smart-pointer на объект геом. буфера
         * @param defaultTexturePtr Smart-pointer на объект текстурного буфера
         * @param textureSet Набор текстур меша
         * @param materialSettings Параметры материала
         * @param textureMappingSettings Параметры отображения текстуры
         */
        Mesh::Mesh(const vk::tools::Device* pDevice,
                   const vk::UniqueDescriptorPool& descriptorPool,
                   const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                   vk::resources::GeometryBufferPtr geometryBufferPtr,
                   const vk::resources::TextureBufferPtr& defaultTexturePtr,
                   vk::scene::MeshTextureSet textureSet,
                   const vk::scene::MeshMaterialSettings& materialSettings,
                   const vk::scene::MeshTextureMapping& textureMappingSettings): SceneElement(),
        isReady_(false),
        pDevice_(pDevice),
        pDescriptorPool_(&(descriptorPool.get())),
        geometryBufferPtr_(std::move(geometryBufferPtr)),
        textureSet_(std::move(textureSet)),
        materialSettings_(materialSettings),
        skeleton_(new MeshSkeleton())
        {
            // Проверить устройство
            if(pDevice_ == nullptr || !pDevice_->isReady()){
                throw vk::DeviceLostError("Device is not available");
            }

            // Выделить буфер для матрицы модели
            uboModelMatrix_ = vk::tools::Buffer(pDevice_,
                    sizeof(glm::mat4),
                    vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для параметров материала
            uboMaterial_ = vk::tools::Buffer(pDevice_,
                    MATERIAL_UBO_SIZE,
                    vk::BufferUsageFlagBits::eUniformBuffer|vk::BufferUsageFlagBits::eStorageBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для параметров отображения текстуры
            uboTextureMapping_ = vk::tools::Buffer(pDevice_,
                    sizeof(vk::scene::MeshTextureMapping),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для параметров использования текстур (какие текстуры используются а какие нет)
            uboTextureUsage_ = vk::tools::Buffer(pDevice_,
                    sizeof(glm::uvec4),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для кол-ва костей скелетной анимации
            uboBoneCount_ = vk::tools::Buffer(pDevice_,
                    sizeof(glm::uint32),
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Выделить буфер для матриц костей скелетной анимации
            uboBoneTransforms_ = vk::tools::Buffer(pDevice_,
                    sizeof(glm::mat4) * MAX_SKELETON_BONES,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Разметить память буферов, получив указатели
            pUboModelMatrixData_ = uboModelMatrix_.mapMemory();
            pUboMaterialData_ = uboMaterial_.mapMemory();
            pUboTextureMappingData_ = uboTextureMapping_.mapMemory();
            pUboTextureUsageData_ = uboTextureUsage_.mapMemory();
            pUboBoneCountData_ = uboBoneCount_.mapMemory();
            pUboBoneTransformsData_ = uboBoneTransforms_.mapMemory();

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
                    {uboMaterial_.getBuffer().get(),0,VK_WHOLE_SIZE},
                    {uboTextureUsage_.getBuffer().get(),0,VK_WHOLE_SIZE},
                    {uboBoneCount_.getBuffer().get(),0,VK_WHOLE_SIZE},
                    {uboBoneTransforms_.getBuffer().get(),0,VK_WHOLE_SIZE}
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
                    {
                            descriptorSet_.get(),
                            4,
                            0,
                            1,
                            vk::DescriptorType::eUniformBuffer,
                            nullptr,
                            bufferInfos.data() + 3,
                            nullptr
                    },
                    {
                            descriptorSet_.get(),
                            5,
                            0,
                            1,
                            vk::DescriptorType::eUniformBuffer,
                            nullptr,
                            bufferInfos.data() + 4,
                            nullptr
                    },
                    {
                            descriptorSet_.get(),
                            6,
                            0,
                            1,
                            vk::DescriptorType::eUniformBuffer,
                            nullptr,
                            bufferInfos.data() + 5,
                            nullptr
                    }
            };

            // Массив с информацией о текстурах привязываемых к дескриптору
            std::vector<vk::DescriptorImageInfo> descriptorImageInfos = {};

            // Превращаем набор текстурных указателей в массив (для более удобной работы)
            std::vector<vk::resources::TextureBufferPtr> texturePointers(4);
            texturePointers[TEXTURE_TYPE_COLOR] = textureSet_.color;
            texturePointers[TEXTURE_TYPE_NORMAL] = textureSet_.normal;
            texturePointers[TEXTURE_TYPE_SPECULAR] = textureSet_.specular;
            texturePointers[TEXTURE_TYPE_DISPLACE] = textureSet_.displace;

            // Заполнение массива описаний изображений
            for(glm::uint32 i = 0; i < texturePointers.size(); i++)
            {
                // Указатель на ресурс текстуры
                vk::resources::TextureBufferPtr texture;

                // Если текстура указана - использовать, отметить что она используется
                if(texturePointers[i].get() != nullptr){
                    texture = texturePointers[i];
                    this->textureUsage_[i] = static_cast<glm::uint32>(true);
                }
                // Если текстура не указана - использовать текстуру по умолчанию, отметить что она НЕ используется
                else{
                    texture = defaultTexturePtr;
                    this->textureUsage_[i] = static_cast<glm::uint32>(false);
                }

                // Если в итоге текстура готова - добавить информацию для дескриптора
                if(texture.get() != nullptr)
                {
                    descriptorImageInfos.emplace_back(
                            vk::DescriptorImageInfo(
                                    texture->getSampler()->get(),
                                    texture->getImage().getImageView().get(),
                                    vk::ImageLayout::eShaderReadOnlyOptimal));
                }
            }

            // Добавить запись для дескрипторов текстур (массив дескрипторов)
            if(!descriptorImageInfos.empty()){
                // Связь дескриптора с изображением
                writes.emplace_back(vk::WriteDescriptorSet(
                        descriptorSet_.get(),
                        3,
                        0,
                        descriptorImageInfos.size(),
                        vk::DescriptorType::eCombinedImageSampler,
                        descriptorImageInfos.data(),
                        nullptr,
                        nullptr
                ));
            }

            // Связываем дескрипторы с ресурсами (буферами и изображениями)
            pDevice_->getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

            // Обновить UBO буферы
            this->updateMatrixUbo();
            this->updateMaterialSettingsUbo();
            this->updateTextureMappingUbo();
            this->updateTextureUsageUbo();
            this->updateSkeletonBoneCountUbo();
            this->updateSkeletonBoneTransformsUbo();

            // Установить callback функция скелету, вызываемую при обновлении данных костей
            skeleton_->setUpdateCallback([&](){
                this->updateSkeletonBoneTransformsUbo();
            });

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

                uboTextureUsage_.unmapMemory();
                uboTextureUsage_.destroyVulkanResources();

                uboBoneCount_.unmapMemory();
                uboBoneCount_.destroyVulkanResources();

                uboBoneTransforms_.unmapMemory();
                uboBoneTransforms_.destroyVulkanResources();

                // Обнулить указатели
                pDevice_ = nullptr;
                pDescriptorPool_ = nullptr;
                pUboModelMatrixData_ = nullptr;
                pUboMaterialData_ = nullptr;
                pUboTextureMappingData_ = nullptr;
                pUboBoneCountData_ = nullptr;
                pUboBoneTransformsData_ = nullptr;

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
                this->updateMatrixUbo();
            }
        }

        /**
         * Обновление UBO буферов матриц модели
         */
        void Mesh::updateMatrixUbo()
        {
            if(uboModelMatrix_.isReady()){
                memcpy(pUboModelMatrixData_,&(this->getModelMatrix()), sizeof(glm::mat4));
            }
        }

        /**
         * Обновление UBO буферов параметров материала меша
         */
        void Mesh::updateMaterialSettingsUbo()
        {
            if(uboMaterial_.isReady()){
                // Преобразование указателя (для возможности указывать побайтовое смещение)
                auto pData = reinterpret_cast<unsigned char*>(pUboMaterialData_);

                // Копирование в буфер (с учетом выравнивания std140)
                memcpy(pData + 0,&materialSettings_.ambientColor,16);
                memcpy(pData + 16,&materialSettings_.diffuseColor,16);
                memcpy(pData + 32, &materialSettings_.specularColor,16);
                memcpy(pData + 44, &materialSettings_.shininess,4);
                memcpy(pData + 48, &materialSettings_.reflectance,4);
            }
        }

        /**
         * Обновление UBO буферов параметров отображения текстуры меша
         */
        void Mesh::updateTextureMappingUbo()
        {
            if(uboTextureMapping_.isReady()){
                memcpy(pUboTextureMappingData_,&textureMapping_, sizeof(vk::scene::MeshTextureMapping));
            }
        }

        /**
         * Обновление UBO параметров использования текстур
         */
        void Mesh::updateTextureUsageUbo()
        {
            if(uboTextureUsage_.isReady()){
                memcpy(pUboTextureUsageData_,textureUsage_, sizeof(glm::uvec4));
            }
        }

        /**
         * Обновление UBO параметров скелетной анимации (кол-во костей)
         */
        void Mesh::updateSkeletonBoneCountUbo()
        {
            if(uboBoneCount_.isReady() && this->skeleton_ != nullptr)
            {
                glm::uint32 count = this->skeleton_->getBonesCount();
                memcpy(pUboBoneCountData_, &count, sizeof(glm::uint32));
            }
        }

        /**
         * Обновление UBO параметров скелетной анимации (матрицы трансформации костей)
         */
        void Mesh::updateSkeletonBoneTransformsUbo()
        {
            if(uboBoneTransforms_.isReady() && this->skeleton_ != nullptr){
                memcpy(pUboBoneTransformsData_,this->skeleton_->getFinalBoneTransforms().data(), this->skeleton_->getTransformsDataSize());
            }
        }

        /**
         * Установить параметры материала
         * @param settings Параметры материала
         */
        void Mesh::setMaterialSettings(const MeshMaterialSettings &settings) {
            materialSettings_ = settings;
            this->updateMaterialSettingsUbo();
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
            this->updateTextureMappingUbo();
        }

        /**
         * Получить параметры отображения текстуры
         * @return Структура описывающая параметры отображения текстуры
         */
        MeshTextureMapping Mesh::getTextureMapping() const {
            return textureMapping_;
        }

        /**
         * Установка нового скелета мешу
         * @param skeleton Unique-smart-pointer объекта скелета
         */
        void Mesh::setSkeleton(UniqueMeshSkeleton skeleton)
        {
            // Сменить скелеты
            this->skeleton_ = std::move(skeleton);
            // Пересчитать матрицы
            this->skeleton_->getRootBone()->calculateBranch(false);

            // Обновить UBO
            this->updateSkeletonBoneCountUbo();
            this->updateSkeletonBoneTransformsUbo();

            // Установить callback функция скелету, вызываемую при обновлении данных костей
            this->skeleton_->setUpdateCallback([&](){
                this->updateSkeletonBoneTransformsUbo();
            });
        }

        /**
         * Получить указатель на скелет меша
         * @return Ссылка на unique-smart-pointer объекта скелета
         */
        const UniqueMeshSkeleton &Mesh::getSkeletonPtr() {
            return this->skeleton_;
        }

        /**
         * Получить UBO буфер модельной матрицы
         * @return Ссылка на объект буфера
         */
        const vk::tools::Buffer &Mesh::getModelMatrixUbo() {
            return uboModelMatrix_;
        }

        /**
         * Получить UBO буфер параметров материала
         * @return Ссылка на объект буфера
         */
        const vk::tools::Buffer &Mesh::getMaterialSettingsUbo() {
            return uboMaterial_;
        }
    }
}

