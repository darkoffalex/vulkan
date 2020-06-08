#pragma once

#include "LightSource.h"
#include "../VkTools/Buffer.hpp"

namespace vk
{
    namespace scene
    {
        class LightSourceSet
        {
        private:
            /// Флаги обновления буферов
            enum BufferUpdateFlagBits
            {
                eCount = (1u << 0u),
                eLightSources = ( 1u << 1u)
            };

            /// Готово ли изображение
            bool isReady_;
            /// Указатель на устройство
            const vk::tools::Device* pDevice_;
            /// Максимальное кол-во источников
            size_t maxLightSources_;

            /// Массив источников света
            std::vector<LightSourcePtr> lightSources_;

            /// UBO буфер для источников света
            vk::tools::Buffer uboLightSources_;
            /// Указатель на размеченную область буфера UBO для источников света
            void* pUboLightSourcesData_;

            /// UBO буфер для кол-ва источников
            vk::tools::Buffer uboLightSourceCount_;
            /// Указатель на размеченную область буфера UBO для кол-ва источников
            void* pUboLightSourceCount_;

            /// Указатель на пул дескрипторов, из которого выделяется набор дескрипторов
            const vk::DescriptorPool *pDescriptorPool_;
            /// Дескрипторный набор
            vk::UniqueDescriptorSet descriptorSet_;

            /**
             * Обновление смещений у объектов в массиве источников
             * @param start Начальный элемент
             *
             * @details Всякий раз при удалении источника из середины массива у объектов теряется соответствие
             * с областью в UBO буфере, поэтому смещения должны быть обновлены
             */
            void refreshLightSourceOffsets(size_t start = 0)
            {
                for(size_t i = start; i < lightSources_.size(); i++){
                    lightSources_[i]->uboOffset_ = i;
                }
            }

            /**
             * Обновление UBO буфера
             * @param updateFlags Тип обновляемого буфера (кол-во или сами источники)
             */
            void updateUbo(unsigned updateFlags = BufferUpdateFlagBits::eCount | BufferUpdateFlagBits::eLightSources)
            {
                if((updateFlags & BufferUpdateFlagBits::eCount) && pUboLightSourceCount_ != nullptr){
                    auto count = static_cast<glm::uint32>(lightSources_.size());
                    memcpy(pUboLightSourceCount_, &count, sizeof(glm::uint32));
                }

                if((updateFlags & BufferUpdateFlagBits::eLightSources) && pUboLightSourcesData_ != nullptr){
                    for(auto& lightSource : lightSources_){
                        lightSource->updateUboRegion();
                    }
                }
            }

        public:
            /**
             * Основной конструктор
             */
            LightSourceSet():
                    isReady_(false),
                    pDevice_(nullptr),
                    maxLightSources_(0),
                    pUboLightSourcesData_(nullptr),
                    pUboLightSourceCount_(nullptr),
                    pDescriptorPool_(nullptr){};

            /**
             * Основной конструктор
             * @param pDevice Указатель на объект устройства
             * @param descriptorPool Unique smart pointer объекта дескрипторного пула
             * @param descriptorSetLayout Unique smart pointer макета размещения дескрипторного набора для источников света
             * @param maxLightSources Максимальное число источников
             */
            LightSourceSet(const vk::tools::Device* pDevice,
                           const vk::UniqueDescriptorPool& descriptorPool,
                           const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                           size_t maxLightSources):
                    isReady_(false),
                    pDevice_(pDevice),
                    maxLightSources_(maxLightSources),
                    pUboLightSourcesData_(nullptr),
                    pUboLightSourceCount_(nullptr),
                    pDescriptorPool_(&(descriptorPool.get()))
            {
                // Проверить устройство
                if(pDevice_ == nullptr || !pDevice_->isReady()){
                    throw vk::DeviceLostError("Device is not available");
                }

                // Выделить буфер для кол-ва источников света
                uboLightSourceCount_ = vk::tools::Buffer(pDevice_,
                        sizeof(glm::uint32),
                        vk::BufferUsageFlagBits::eUniformBuffer,
                        vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

                // Выделить буфер для массива источников света
                uboLightSources_ = vk::tools::Buffer(pDevice_,
                        vk::scene::LIGHT_ENTRY_SIZE * maxLightSources_,
                        vk::BufferUsageFlagBits::eUniformBuffer,
                        vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

                // Разметить память буферов, получив указатели на регионы
                pUboLightSourceCount_ = uboLightSourceCount_.mapMemory(0, sizeof(glm::uint32));
                pUboLightSourcesData_ = uboLightSources_.mapMemory(0, vk::scene::LIGHT_ENTRY_SIZE * maxLightSources_);

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
                        {uboLightSourceCount_.getBuffer().get(),0,VK_WHOLE_SIZE},
                        {uboLightSources_.getBuffer().get(),0,VK_WHOLE_SIZE},
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
                                &(bufferInfos[0]),
                                nullptr
                        },
                        {
                                descriptorSet_.get(),
                                1,
                                0,
                                1,
                                vk::DescriptorType::eUniformBuffer,
                                nullptr,
                                &(bufferInfos[1]),
                                nullptr
                        },
                };

                // Связываем дескрипторы с ресурсами (буферами)
                pDevice_->getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

                // Инициализация завершена
                isReady_ = true;
            }

            /**
            * Запрет копирования через инициализацию
            * @param other Ссылка на копируемый объекта
            */
            LightSourceSet(const LightSourceSet& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            LightSourceSet& operator=(const LightSourceSet& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            LightSourceSet(LightSourceSet&& other) noexcept :LightSourceSet()
            {
                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_, other.pDevice_);
                std::swap(maxLightSources_, other.maxLightSources_);
                std::swap(pUboLightSourceCount_, other.pUboLightSourceCount_);
                std::swap(pUboLightSourcesData_, other.pUboLightSourcesData_);
                std::swap(pDescriptorPool_, other.pDescriptorPool_);

                lightSources_.swap(other.lightSources_);
                descriptorSet_.swap(other.descriptorSet_);

                uboLightSources_ = std::move(other.uboLightSources_);
                uboLightSourceCount_ = std::move(other.uboLightSourceCount_);
            }

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            LightSourceSet& operator=(LightSourceSet&& other) noexcept
            {
                if (this == &other) return *this;

                this->destroyVulkanResources();
                isReady_ = false;
                pDevice_ = nullptr;
                pDescriptorPool_ = nullptr;
                pUboLightSourcesData_ = nullptr;
                pUboLightSourceCount_ = nullptr;
                maxLightSources_ = 0;

                std::swap(isReady_,other.isReady_);
                std::swap(pDevice_, other.pDevice_);
                std::swap(maxLightSources_, other.maxLightSources_);
                std::swap(pUboLightSourceCount_, other.pUboLightSourceCount_);
                std::swap(pUboLightSourcesData_, other.pUboLightSourcesData_);
                std::swap(pDescriptorPool_, other.pDescriptorPool_);

                lightSources_.swap(other.lightSources_);
                descriptorSet_.swap(other.descriptorSet_);

                uboLightSources_ = std::move(other.uboLightSources_);
                uboLightSourceCount_ = std::move(other.uboLightSourceCount_);

                return *this;
            }

            /**
             * Деструктор
             */
            ~LightSourceSet(){
                destroyVulkanResources();
            }

            /**
             * Де-инициализация ресурсов Vulkan
             */
            void destroyVulkanResources()
            {
                if(isReady_ && pDevice_!= nullptr && pDevice_->isReady())
                {
                    // Вернуть в пул набор дескрипторов и освободить smart-pointer
                    pDevice_->getLogicalDevice()->freeDescriptorSets(*pDescriptorPool_,{descriptorSet_.get()});
                    descriptorSet_.release();

                    // Очистить UBO буферы
                    uboLightSourceCount_.unmapMemory();
                    uboLightSourceCount_.destroyVulkanResources();

                    uboLightSources_.unmapMemory();
                    uboLightSources_.destroyVulkanResources();

                    // Обнулить указатели
                    pDevice_ = nullptr;
                    pDescriptorPool_ = nullptr;
                    pUboLightSourcesData_ = nullptr;
                    pUboLightSourceCount_ = nullptr;

                    // Объект де-инициализирован
                    isReady_ = false;
                }
            }

            /**
             * Добавить источник света
             * @param type Тип источника света
             * @param position Положение источника света
             * @param color Цвет источника света
             * @param attenuationLinear Линейный коэффициент затухания
             * @param attenuationQuadratic Квадратичный коэффициент затухания
             * @param cutOffAngle Внутренний угол отсечения света (для типа eSpot)
             * @param cutOffOuterAngle Внешний угол отсечения света (для типа eSpot)
             * @return Shared smart pointer на объект источника
             */
            LightSourcePtr addLightSource(const LightSourceType& type,
                                          const glm::vec3& position = {0.0f,0.0f,0.0f},
                                          const glm::vec3& color = {1.0f,1.0f,1.0f},
                                          glm::float32 attenuationLinear = 0.20f,
                                          glm::float32 attenuationQuadratic = 0.22f,
                                          glm::float32 cutOffAngle = 40.0f,
                                          glm::float32 cutOffOuterAngle = 45.0f)
            {
                // Создать источник света
                auto lightSource = std::make_shared<vk::scene::LightSource>(
                        reinterpret_cast<unsigned char*>(pUboLightSourcesData_),
                        lightSources_.size(),
                        type,
                        position,
                        color,
                        attenuationLinear,
                        attenuationQuadratic,
                        cutOffAngle,
                        cutOffOuterAngle);

                // Добавить в массив
                lightSources_.push_back(lightSource);

                // Обновить UBO
                lightSource->updateUboRegion();
                this->updateUbo(BufferUpdateFlagBits::eCount);

                // Отдать smart pointer
                return lightSource;
            }

            /**
             * Удалить источник освещения
             * @param lightSourcePtr Shared smart pointer на объект источника
             */
            void removeLightSource(const LightSourcePtr& lightSourcePtr)
            {
                // Удаляем из списка
                lightSources_.erase(std::remove_if(lightSources_.begin(), lightSources_.end(), [&](const vk::scene:: LightSourcePtr& entryPtr){
                    return lightSourcePtr.get() == entryPtr.get();
                }), lightSources_.end());

                // Обновить смещения в массиве объектов
                this->refreshLightSourceOffsets();

                // Обновить UBO
                this->updateUbo(BufferUpdateFlagBits::eCount|BufferUpdateFlagBits::eLightSources);
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
             * Получить дескрипторный набор
             * @return Константная ссылка на объект дескрипторного набора
             */
            const vk::DescriptorSet& getDescriptorSet() const
            {
                return descriptorSet_.get();
            }
        };
    }
}

