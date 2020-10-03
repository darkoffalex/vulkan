#pragma once

#include "SceneElement.h"
#include "../VkTools/Buffer.hpp"

namespace vk
{
    namespace scene
    {
        enum class CameraProjectionType
        {
            ePerspective,
            eOrthogonal
        };

        class Camera : public SceneElement
        {
        private:
            /// Флаги обновления буферов
            enum BufferUpdateFlagBits
            {
                eView = (1u << 0u),
                eProjection = ( 1u << 1u),
                eCamPosition = (1u << 2u),
                eCamModel = (1u << 3u),
                eFov = (1u << 4u)
            };

            /// Готово ли изображение
            bool isReady_;
            /// Указатель на устройство
            const vk::tools::Device* pDevice_;

            /// Матрица проекции
            glm::mat4 projectionMatrix_;
            /// Тип проекции
            CameraProjectionType projectionType_;
            /// Ближняя грань отсечения
            glm::float32 zNear_;
            /// Дальняя грань отсечения
            glm::float32 zFar_;
            /// Угол обзора либо размер области видимости (для ортогональной проекции)
            glm::float32 fov_;
            /// Соотношение сторон вью-порта
            glm::float32 aspectRatio_;

            /// UBO буфер для матриц вида-проекции
            vk::tools::Buffer uboCameraBuffer_;
            /// Указатель на размеченную область буфера UBO
            void* pUboMapped_;
            /// Указатель на размеченную область буфера UBO для матриц вида
            void* pUboViewMatrixData_;
            /// Указатель на размеченную область буфера UBO для матрицы проекции
            void* pUboProjectionMatrixData_;
            /// Указатель на размеченную область буфера UBO для матрицы модели камеры (обратная к матрице вида)
            void* pUboCamModelMatrixData_;
            /// Указатель на размеченную область буфера UBO для положения камеры
            void* pUboCamPositionData_;
            /// Указатель на размеченную область буфера UBO для FOV камеры (используется шейдерами тарссировки лучей)
            void* pUboFovData_;

            /// Указатель на пул дескрипторов, из которого выделяется набор дескрипторов меша
            const vk::DescriptorPool *pDescriptorPool_;
            /// Дескрипторный набор
            vk::UniqueDescriptorSet descriptorSet_;

            /**
             * Обновление матрицы проекции с учетом всех параметров камеры
             */
            void updateProjectionMatrix();

            /**
             * Обновление UBO буферов матриц модели
             * @param updateFlags Флаги обновления буферов (не всегда нужно обновлять все матрицы)
             */
            void updateUbo(unsigned updateFlags = BufferUpdateFlagBits::eView | BufferUpdateFlagBits::eProjection | BufferUpdateFlagBits::eCamPosition | BufferUpdateFlagBits::eCamModel | BufferUpdateFlagBits::eFov);

            /**
             * Событие смены положения
             * @param updateMatrices Запрос обновить матрицы
             */
            void onPlacementUpdated(bool updateMatrices) override;

        public:

            /**
             * Конструктор по умолчанию
             */
            Camera();

            /**
             * Основной конструктор
             * @param pDevice Указатель на объект устройства
             * @param descriptorPool Unique smart pointer объекта дескрипторного пула
             * @param descriptorSetLayout Unique smart pointer макета размещения дескрипторного набора меша
             * @param position Изначальное положение камеры
             * @param orientation Ориентация камеры
             * @param aspectRatio Пропорции
             * @param projectionType Тип проекции камеры
             * @param zNear Ближняя грань отсечения
             * @param zFar Дальняя грань отсечения
             * @param fov угол обзора / поле обзора
             */
            explicit Camera(const vk::tools::Device* pDevice,
                            const vk::UniqueDescriptorPool& descriptorPool,
                            const vk::UniqueDescriptorSetLayout& descriptorSetLayout,
                            const glm::vec3& position,
                            const glm::vec3& orientation,
                            const glm::float32& aspectRatio,
                            const CameraProjectionType& projectionType = CameraProjectionType::ePerspective,
                            const glm::float32& zNear = 0.1f,
                            const glm::float32& zFar = 1000.0f,
                            const glm::float32& fov = 45.0f);

            /**
             * Запрет копирования через инициализацию
             * @param other Ссылка на копируемый объекта
             */
            Camera(const Camera& other) = delete;

            /**
             * Запрет копирования через присваивание
             * @param other Ссылка на копируемый объекта
             * @return Ссылка на текущий объект
             */
            Camera& operator=(const Camera& other) = delete;

            /**
             * Конструктор перемещения
             * @param other R-value ссылка на другой объект
             * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
             */
            Camera(Camera&& other) noexcept;

            /**
             * Перемещение через присваивание
             * @param other R-value ссылка на другой объект
             * @return Ссылка на текущий объект
             */
            Camera& operator=(Camera&& other) noexcept;

            /**
             * Деструктор
             */
            ~Camera() override;

            /**
             * Получить матрицу проекции
             * @return Матрица 4*4
             */
            [[nodiscard]] const glm::mat4& getProjectionMatrix() const;

            /**
             * Установить тип проекции
             * @param projectionType Тип проекции (enum)
             */
            void setProjectionType(const CameraProjectionType& projectionType);

            /**
             * Получить тип проекции
             * @return Тип проекции (enum)
             */
            [[nodiscard]] const CameraProjectionType& getProjectionType() const;

            /**
             * Установить ближнюю грань используемую при построении матрицы проекции
             * @param zNear Ближняя грань
             */
            void setZNear(const float& zNear);

            /**
             * Получить ближнюю грань используемую при построении матрицы проекции
             * @return Ближняя грань
             */
            [[nodiscard]] const float& getZNear() const;

            /**
             * Установить дальнюю грань используемую при построении матрицы проекции
             * @param zFar Дальняя грань
             */
            void setZFar(const float& zFar);

            /**
             * Получить дальнюю грань используемую при построении матрицы проекции
             * @return Дальняя грань
             */
            [[nodiscard]] const float& getZFar() const;

            /**
             * Установить угол (поле) обзора
             * @param fov Угол (при перспективной проекции) либо поле (при ортогональной) обзора
             */
            void setFov(const float& fov);

            /**
             * Получить угол (поле) обзора
             * @return Угол (при перспективной проекции) либо поле (при ортогональной) обзора
             */
            [[nodiscard]] const float& getFov() const;

            /**
             * Установить соотношение сторон
             * @param aspectRatio
             */
            void setAspectRatio(const float& aspectRatio);

            /**
             * Получить соотношение сторон
             * @return Отношение ширины к высоте
             */
            [[nodiscard]] const float& getAspectRatio() const;

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
             * Получить дескрипторный набор
             * @return Константная ссылка на объект дескрипторного набора
             */
            const vk::DescriptorSet& getDescriptorSet() const;
        };
    }
}


