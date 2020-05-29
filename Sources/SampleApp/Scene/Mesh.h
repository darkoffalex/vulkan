#pragma once

#include "SceneElement.h"
#include "../VkToolsGeometryBuffer.hpp"

namespace vk
{
    namespace scene
    {
        struct MeshMaterial
        {
            glm::vec3 albedo;
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

            //TODO: Свой UBO буфер
            //TODO: Свой набор дескрипторов

            /**
             * Обновление буферов UBO
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
             * @param geometryBufferPtr Smart-pointer на объект геом. буфера
             */
            explicit Mesh(const vk::tools::Device* pDevice, vk::tools::GeometryBufferPtr geometryBufferPtr);

            //TODO: запрет копирования

            //TODO: override методов управления пространственными параметрами (обновление UBO)

            /**
             * Деструктор
             */
            ~Mesh() override = default;

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
        };

        /**
         * Smart-pointer объекта меша
         * @rdetails Данный указатель может быть возвращен пользователю объектом рендерера при добавлении меша на сцену
         */
        typedef std::shared_ptr<Mesh> MeshPtr;
    }
}
