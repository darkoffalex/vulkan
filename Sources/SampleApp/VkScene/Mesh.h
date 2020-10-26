#pragma once

#include "SceneElement.h"
#include "MeshSkeleton.hpp"
#include "../VkResources/GeometryBuffer.hpp"
#include "../VkResources/TextureBuffer.hpp"

namespace vk
{
    namespace scene
    {
        /**
         * Размер UBO буфера материала меша
         * С учетом выравнивания std140
         */
        const size_t MATERIAL_UBO_SIZE = 24;

        /**
         * Количество костей скелетной анимации
         * Определяет размер буфера uboBoneTransforms_
         */
        const size_t MAX_SKELETON_BONES = 50;

        // Типы текстур
        // Порядок индексов должен соответствовать порядку MeshTextureSet
        const size_t TEXTURE_TYPE_ALBEDO    = 0;
        const size_t TEXTURE_TYPE_ROUGHNESS = 1;
        const size_t TEXTURE_TYPE_METALLIC  = 2;
        const size_t TEXTURE_TYPE_NORMAL    = 3;
        const size_t TEXTURE_TYPE_DISPLACE  = 4;

        struct MeshTextureSet
        {
            vk::resources::TextureBufferPtr albedo = nullptr;
            vk::resources::TextureBufferPtr roughness = nullptr;
            vk::resources::TextureBufferPtr metallic = nullptr;
            vk::resources::TextureBufferPtr normal = nullptr;
            vk::resources::TextureBufferPtr displace = nullptr;
        };

        struct MeshMaterialSettings
        {
            glm::vec3 albedo = {1.0f, 1.0f, 1.0f};
            glm::float32_t roughness = 1.0f;
            glm::float32_t metallic = 0.0f;
        };

        struct MeshTextureMapping
        {
            glm::vec2 offset = {0.0f,0.0f};
            glm::vec2 origin = {0.0f,0.0f};
            glm::vec2 scale = {1.0f,1.0f};
            glm::float32 angle = 0.0f;
        };

        class Mesh : public SceneElement
        {
        private:
            /// Готово ли к использованию
            bool isReady_;
            /// Указатель на устройство
            const vk::tools::Device* pDevice_;
            /// Указатель на геометрический буфер
            vk::resources::GeometryBufferPtr geometryBufferPtr_;
            /// Набор указателей текстурных буферов
            vk::scene::MeshTextureSet textureSet_;
            /// Параметры материала меша
            vk::scene::MeshMaterialSettings materialSettings_;
            /// Параметры отображения текстуры меша
            vk::scene::MeshTextureMapping textureMapping_;
            /// Параметры использования текстур
            glm::uint32 textureUsage_[5] = {0,0,0,0,0};
            /// Параметры скелета
            UniqueMeshSkeleton skeleton_;

            /// UBO буфер для матрицы модели
            vk::tools::Buffer uboModelMatrix_;
            /// Указатель на размеченную область буфера UBO матрицы модели
            void* pUboModelMatrixData_;

            /// UBO буфер для параметров материала
            vk::tools::Buffer uboMaterial_;
            /// Указатель на размеченную область буфера UBO материала
            void* pUboMaterialData_;

            /// UBO буфер для параметров материала меша
            vk::tools::Buffer uboTextureMapping_;
            /// Указатель на размеченную область буфера UBO параметров материала
            void* pUboTextureMappingData_;

            /// UBO буфер для параметров использования текстур
            vk::tools::Buffer uboTextureUsage_;
            /// Указатель на размеченную область буфера UBO параметров использования текстур
            void* pUboTextureUsageData_;

            /// UBO буфер для кол-ва костей скелетной анимации
            vk::tools::Buffer uboBoneCount_;
            /// Указатель на размеченную область буфера UBO кол-ва костей скелетной анимации
            void* pUboBoneCountData_;

            /// UBO буфер для матриц костей скелетной анимации
            vk::tools::Buffer uboBoneTransforms_;
            /// Указатель на размеченную область буфера UBO матриц костей скелетной анимации
            void* pUboBoneTransformsData_;

            /// Указатель на пул дескрипторов, из которого выделяется набор дескрипторов меша
            const vk::DescriptorPool *pDescriptorPool_;
            /// Дескрипторный набор
            vk::UniqueDescriptorSet descriptorSet_;

            /**
             * Обновление UBO буферов матриц модели
             */
            void updateMatrixUbo();

            /**
             * Обновление UBO буферов параметров материала меша
             */
            void updateMaterialSettingsUbo();

            /**
             * Обновление UBO буферов параметров отображения текстуры меша
             */
            void updateTextureMappingUbo();

            /**
             * Обновление UBO параметров использования текстур
             */
            void updateTextureUsageUbo();

            /**
             * Обновление UBO параметров скелетной анимации (кол-во костей)
             */
            void updateSkeletonBoneCountUbo();

            /**
             * Обновление UBO параметров скелетной анимации (матрицы трансформации костей)
             */
            void updateSkeletonBoneTransformsUbo();

            /**
             * Событие смены положения
             * @param updateMatrices Запрос обновить матрицы
             */
            void onPlacementUpdated(bool updateMatrices) override;

        public:
            /**
             * Конструктор по умолчанию
             */
            Mesh();

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
            explicit Mesh(const vk::tools::Device* pDevice,
                    const vk::UniqueDescriptorPool& descriptorPool,
                    const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                    vk::resources::GeometryBufferPtr geometryBufferPtr,
                    const vk::resources::TextureBufferPtr& defaultTexturePtr,
                    vk::scene::MeshTextureSet textureSet = {},
                    const vk::scene::MeshMaterialSettings& materialSettings = {{1.0f, 1.0f, 1.0f},1.0f,0.0f},
                    const vk::scene::MeshTextureMapping& textureMappingSettings = {{0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 1.0f}, 0.0f});

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            Mesh(const Mesh& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            Mesh& operator=(const Mesh& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            Mesh(Mesh&& other) noexcept;

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            Mesh& operator=(Mesh&& other) noexcept;

            /**
             * Деструктор
             */
            ~Mesh() override;

            /**
             * Де-инициализация ресурсов Vulkan
             */
            void destroyVulkanResources();

            /**
             * Был ли объект инициализирован
             * @return Да или нет
             */
            bool isReady() const;

            /**
             * Получить буфер геометрии
             * @return Константная ссылка на smart-pointer объекта буфера
             */
            const vk::resources::GeometryBufferPtr& getGeometryBuffer() const;

            /**
             * Получить дескрипторный набор
             * @return Константная ссылка на объект дескрипторного набора
             */
            const vk::DescriptorSet& getDescriptorSet() const;

            /**
             * Установить параметры материала
             * @param settings Параметры материала
             */
            void setMaterialSettings(const MeshMaterialSettings& settings);

            /**
             * Получить параметры материала
             * @return Структура описывающая материал
             */
            MeshMaterialSettings getMaterialSettings() const;

            /**
             * Установить параметры отображения текстуры
             * @param textureMapping Параметры отображения текстуры
             */
            void setTextureMapping(const MeshTextureMapping& textureMapping);

            /**
             * Получить параметры отображения текстуры
             * @return Структура описывающая параметры отображения текстуры
             */
            MeshTextureMapping getTextureMapping() const;

            /**
             * Установка нового скелета мешу
             * @param skeleton Unique-smart-pointer объекта скелета
             */
            void setSkeleton(UniqueMeshSkeleton skeleton);

            /**
             * Получить указатель на скелет меша
             * @return Ссылка на unique-smart-pointer объекта скелета
             */
            const UniqueMeshSkeleton& getSkeletonPtr();
        };

        /**
         * Smart-pointer объекта меша
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при добавлении меша на сцену
         */
        typedef std::shared_ptr<Mesh> MeshPtr;
    }
}
