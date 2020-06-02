#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS

#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace vk
{
    namespace scene
    {
        /**
         * Конструктор по умолчанию
         */
        Camera::Camera():SceneElement({0.0f,0.0f,0.0f}),
                         isReady_(false),
                         pDevice_(nullptr),
                         pDescriptorPool_(nullptr),
                         pUboViewMatrixData_(nullptr),
                         pUboProjectionMatrixData_(nullptr),
                         projectionMatrix_({}),
                         aspectRatio_(1.0f),
                         projectionType_(CameraProjectionType::ePerspective),
                         zNear_(0.1f),
                         zFar_(1000.0f),
                         fov_(45.0f)
        {}

        /**
         * Конструктор перемещения
         * @param other R-value ссылка на другой объект
         * @details Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        Camera::Camera(Camera &&other) noexcept :Camera()
        {
            std::swap(isReady_,other.isReady_);
            std::swap(pDevice_,other.pDevice_);
            std::swap(pDescriptorPool_,other.pDescriptorPool_);
            std::swap(pUboViewMatrixData_, other.pUboViewMatrixData_);
            std::swap(pUboProjectionMatrixData_, other.pUboProjectionMatrixData_);

            std::swap(projectionMatrix_, other.projectionMatrix_);
            std::swap(projectionType_, other.projectionType_);
            std::swap(zNear_, other.zNear_);
            std::swap(zFar_, other.zFar_);
            std::swap(fov_, other.fov_);
            std::swap(aspectRatio_, other.aspectRatio_);

            descriptorSet_.swap(other.descriptorSet_);
            uboViewProjectionMatrix_ = std::move(other.uboViewProjectionMatrix_);
        }

        /**
         * Перемещение через присваивание
         * @param other R-value ссылка на другой объект
         * @return Ссылка на текущий объект
         */
        Camera &Camera::operator=(Camera &&other) noexcept
        {
            if (this == &other) return *this;

            this->destroyVulkanResources();
            isReady_ = false;
            pDevice_ = nullptr;
            pDescriptorPool_ = nullptr;
            pUboViewMatrixData_ = nullptr;
            pUboProjectionMatrixData_ = nullptr;

            std::swap(isReady_,other.isReady_);
            std::swap(pDevice_,other.pDevice_);
            std::swap(pDescriptorPool_,other.pDescriptorPool_);
            std::swap(pUboViewMatrixData_, other.pUboViewMatrixData_);
            std::swap(pUboProjectionMatrixData_, other.pUboProjectionMatrixData_);

            std::swap(projectionMatrix_, other.projectionMatrix_);
            std::swap(projectionType_, other.projectionType_);
            std::swap(zNear_, other.zNear_);
            std::swap(zFar_, other.zFar_);
            std::swap(fov_, other.fov_);
            std::swap(aspectRatio_, other.aspectRatio_);

            descriptorSet_.swap(other.descriptorSet_);
            uboViewProjectionMatrix_ = std::move(other.uboViewProjectionMatrix_);

            return *this;
        }

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
        Camera::Camera(const vk::tools::Device *pDevice,
                       const UniqueHandle<DescriptorPool,::vk::DispatchLoaderStatic> &descriptorPool,
                       const UniqueHandle<DescriptorSetLayout, ::vk::DispatchLoaderStatic> &descriptorSetLayout,
                       const glm::vec3 &position, const glm::vec3 &orientation, const glm::float32 &aspectRatio,
                       const CameraProjectionType& projectionType, const glm::float32 &zNear, const glm::float32 &zFar,
                       const glm::float32 &fov):
                SceneElement(position,orientation),
                isReady_(false),
                pDevice_(pDevice),
                pDescriptorPool_(&(descriptorPool.get())),
                pUboViewMatrixData_(nullptr),
                pUboProjectionMatrixData_(nullptr),
                projectionMatrix_({}),
                aspectRatio_(aspectRatio),
                projectionType_(projectionType),
                zNear_(zNear),
                zFar_(zFar),
                fov_(fov)
        {
            // Проверить устройство
            if(pDevice_ == nullptr || !pDevice_->isReady()){
                throw vk::DeviceLostError("Device is not available");
            }

            // Выделить буфер для матрицы модели
            uboViewProjectionMatrix_ = vk::tools::Buffer(pDevice_,
                    sizeof(glm::mat4) * 2,
                    vk::BufferUsageFlagBits::eUniformBuffer,
                    vk::MemoryPropertyFlagBits::eHostVisible|vk::MemoryPropertyFlagBits::eHostCoherent);

            // Разметить память буфера, получив указатели на регионы
            pUboViewMatrixData_ = uboViewProjectionMatrix_.mapMemory(0, sizeof(glm::mat4));
            pUboProjectionMatrixData_ = uboViewProjectionMatrix_.mapMemory(sizeof(glm::mat4), sizeof(glm::mat4));

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
                    {uboViewProjectionMatrix_.getBuffer().get(),0,VK_WHOLE_SIZE},
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
            };

            // Связываем дескрипторы с ресурсами (буферами)
            pDevice_->getLogicalDevice()->updateDescriptorSets(writes.size(),writes.data(),0, nullptr);

            // Обновляем матрицу проекции
            this->updateProjectionMatrix();

            // Обновить UBO буферы
            this->updateMatrixBuffers();

            // Инициализация завершена
            isReady_ = true;
        }

        /**
         * Деструктор
         */
        Camera::~Camera()
        {
            this->destroyVulkanResources();
        }

        /**
         * Де-инициализация ресурсов Vulkan
         */
        void Camera::destroyVulkanResources()
        {
            if(isReady_ && pDevice_!= nullptr && pDevice_->isReady())
            {
                // Вернуть в пул набор дескрипторов и освободить smart-pointer
                pDevice_->getLogicalDevice()->freeDescriptorSets(*pDescriptorPool_,{descriptorSet_.get()});
                descriptorSet_.release();

                // Очистить UBO буферы
                uboViewProjectionMatrix_.unmapMemory();
                uboViewProjectionMatrix_.destroyVulkanResources();

                // Обнулить указатели
                pDevice_ = nullptr;
                pDescriptorPool_ = nullptr;
                pUboViewMatrixData_ = nullptr;
                pUboProjectionMatrixData_ = nullptr;

                // Объект де-инициализирован
                isReady_ = false;
            }
        }

        /**
         * Обновление матрицы проекции с учетом всех параметров камеры
         */
        void Camera::updateProjectionMatrix()
        {
            switch (this->projectionType_)
            {
                case CameraProjectionType::ePerspective:
                    this->projectionMatrix_ = glm::perspective(glm::radians(fov_), aspectRatio_, zNear_, zFar_);
                    break;
                case CameraProjectionType::eOrthogonal:
                    this->projectionMatrix_ = glm::ortho(-fov_ * aspectRatio_ / 2.0f, fov_ * aspectRatio_ / 2.0f, -fov_ / 2.0f, fov_ / 2.0f, zNear_, zFar_);
                    break;
                default:
                    this->projectionMatrix_ = glm::mat4(1);
                    break;
            }
        }

        /**
         * Получить матрицу проекции
         * @return Матрица 4*4
         */
        const glm::mat4 &Camera::getProjectionMatrix() const {
            return projectionMatrix_;
        }

        /**
         * Установить тип проекции
         * @param projectionType Тип проекции (enum)
         */
        void Camera::setProjectionType(const CameraProjectionType& projectionType)
        {
            this->projectionType_ = projectionType;
            this->updateProjectionMatrix();
            this->updateMatrixBuffers(BufferUpdateFlagBits::eProjection);
        }

        /**
         * Получить тип проекции
         * @return Тип проекции (enum)
         */
        const CameraProjectionType& Camera::getProjectionType() const {
            return projectionType_;
        }

        /**
         * Установить ближнюю грань используемую при построении матрицы проекции
         * @param zNear Ближняя грань
         */
        void Camera::setZNear(const float &zNear)
        {
            this->zNear_ = zNear;
            this->updateProjectionMatrix();
            this->updateMatrixBuffers(BufferUpdateFlagBits::eProjection);
        }

        /**
         * Получить ближнюю грань используемую при построении матрицы проекции
         * @return Ближняя грань
         */
        const float &Camera::getZNear() const {
            return zNear_;
        }

        /**
         * Установить дальнюю грань используемую при построении матрицы проекции
         * @param zFar Дальняя грань
         */
        void Camera::setZFar(const float &zFar)
        {
            this->zFar_ = zFar;
            this->updateProjectionMatrix();
            this->updateMatrixBuffers(BufferUpdateFlagBits::eProjection);
        }

        /**
         * Получить дальнюю грань используемую при построении матрицы проекции
         * @return Дальняя грань
         */
        const float &Camera::getZFar() const {
            return zFar_;
        }

        /**
         * Установить угол (поле) обзора
         * @param fov Угол (при перспективной проекции) либо поле (при ортогональной) обзора
         */
        void Camera::setFov(const float &fov)
        {
            this->fov_ = fov;
            this->updateProjectionMatrix();
            this->updateMatrixBuffers(BufferUpdateFlagBits::eProjection);
        }

        /**
         * Получить угол (поле) обзора
         * @return Угол (при перспективной проекции) либо поле (при ортогональной) обзора
         */
        const float &Camera::getFov() const {
            return fov_;
        }

        /**
         * Установить соотношение сторон
         * @param aspectRatio
         */
        void Camera::setAspectRatio(const float &aspectRatio)
        {
            this->aspectRatio_ = aspectRatio;
            this->updateProjectionMatrix();
            this->updateMatrixBuffers(BufferUpdateFlagBits::eProjection);
        }

        /**
         * Получить соотношение сторон
         * @return Отношение ширины к высоте
         */
        const float &Camera::getAspectRatio() const {
            return aspectRatio_;
        }

        /**
         * Обновление UBO буферов матриц модели
         * @param updateFlags Флаги обновления буферов (не всегда нужно обновлять все матрицы)
         */
        void Camera::updateMatrixBuffers(const unsigned int updateFlags)
        {
            if(uboViewProjectionMatrix_.isReady())
            {
                if(updateFlags & BufferUpdateFlagBits::eView)
                    memcpy(pUboViewMatrixData_,&(this->getViewMatrix()),sizeof(glm::mat4));

                if(updateFlags & BufferUpdateFlagBits::eProjection)
                    memcpy(pUboProjectionMatrixData_,&(this->getProjectionMatrix()),sizeof(glm::mat4));
            }
        }

        /**
         * Обработка события обновления матриц
         */
        void Camera::onMatricesUpdated()
        {
            // Метод вызывается только после смены пространственных параметров (матрица вида)
            // Достаточно обновить в буфере только эту матрицу
            this->updateMatrixBuffers(BufferUpdateFlagBits::eView);
        }

        /**
         * Был ли объект инициализирован
         * @return Да или нет
         */
        bool Camera::isReady() const
        {
            return isReady_;
        }

        /**
         * Получить дескрипторный набор
         * @return Константная ссылка на объект дескрипторного набора
         */
        const vk::DescriptorSet& Camera::getDescriptorSet() const
        {
            return descriptorSet_.get();
        }
    }
}