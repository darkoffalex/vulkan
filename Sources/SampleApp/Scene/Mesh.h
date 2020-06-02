#pragma once

#include "SceneElement.h"
#include "../VkToolsGeometryBuffer.hpp"
#include "../VkToolsTextureBuffer.hpp"

namespace vk
{
    namespace scene
    {
        struct MeshMaterialSettings
        {
            glm::vec3 albedo = {1.0f,1.0f,1.0f};
            glm::float32 metallic = 0.0f;
            glm::float32 roughness = 1.0f;
        };

        class Mesh : public SceneElement
        {
        private:
            /// Готово ли изображение
            bool isReady_;
            /// Указатель на устройство
            const vk::tools::Device* pDevice_;
            /// Указатель на геометрический буфер
            vk::tools::GeometryBufferPtr geometryBufferPtr_;
            /// Указатель на текстурный буфер
            vk::tools::TextureBufferPtr textureBufferPtr_;
            /// Параметры материала меша
            vk::scene::MeshMaterialSettings materialSettings_;

            /// UBO буфер для матрицы модели
            vk::tools::Buffer uboModelMatrix_;
            /// Указатель на размеченную область буфера UBO матрицы модели
            void* pUboModelMatrixData_;

            /// UBO буфер для параметров материала
            vk::tools::Buffer uboMaterial_;
            /// Указатель на размеченную область буфера UBO материала
            void* pUboMaterialData_;

            /// Указатель на пул дескрипторов, из которого выделяется набор дескрипторов меша
            const vk::DescriptorPool *pDescriptorPool_;
            /// Дескрипторный набор
            vk::UniqueDescriptorSet descriptorSet_;

            /**
             * Обновление UBO буферов матриц модели
             */
            void updateMatrixBuffers();

            /**
             * Обновление UBO буферов параметров материала меша
             */
            void updateMaterialSettingsBuffers();

            /**
             * Обработка события обновления матриц
             */
            void onMatricesUpdated() override;

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
             * @param materialSettings Параметры материала
             */
            explicit Mesh(const vk::tools::Device* pDevice,
                    const vk::UniqueDescriptorPool& descriptorPool,
                    const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                    vk::tools::GeometryBufferPtr geometryBufferPtr,
                    vk::tools::TextureBufferPtr textureBufferPtr = nullptr,
                    const vk::scene::MeshMaterialSettings& materialSettings = {{1.0f,1.0f,1.0f},0.0f,1.0f});

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
            const vk::tools::GeometryBufferPtr& getGeometryBuffer() const;

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
        };

        /**
         * Smart-pointer объекта меша
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при добавлении меша на сцену
         */
        typedef std::shared_ptr<Mesh> MeshPtr;
    }
}
