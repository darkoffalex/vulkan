#include "Mesh.h"

#include <utility>

/**
 * Конструктор по умолчанию
 */
vk::scene::Mesh::Mesh():SceneElement(),isReady_(false),pDevice_(nullptr){}

/**
 * Основной конструктор
 * @param pDevice Указатель на объект устройства
 * @param geometryBufferPtr Smart-pointer на объект геом. буфера
 */
vk::scene::Mesh::Mesh(const vk::tools::Device* pDevice, vk::tools::GeometryBufferPtr geometryBufferPtr):SceneElement(),
isReady_(false),
pDevice_(pDevice),
geometryBufferPtr_(std::move(geometryBufferPtr))
{
    // Проверить устройство
    if(pDevice_ == nullptr || !pDevice_->isReady()){
        throw vk::DeviceLostError("Device is not available");
    }

    //TODO: основная инициализация и создание дескрипторного набора меша и выделение всех необходимых UBO буферов

    isReady_ = true;
}

/**
 * Де-инициализация ресурсов Vulkan
 */
void vk::scene::Mesh::destroyVulkanResources()
{
    if(isReady_ && pDevice_!= nullptr && pDevice_->isReady())
    {
        //TODO: Уничтожение выделенных в конструкторе ресурсов
        isReady_ = false;
    }
}

/**
 * Был ли объект инициализирован
 * @return Да или нет
 */
bool vk::scene::Mesh::isReady() const {
    return isReady_;
}

/**
 * Получить буфер геометрии
 * @return Константная ссылка на smart-pointer объекта буфера
 */
const vk::tools::GeometryBufferPtr &vk::scene::Mesh::getGeometryBuffer() const {
    return geometryBufferPtr_;
}

/**
 * Обновление буферов UBO
 */
void vk::scene::Mesh::onMatricesUpdated()
{
    //TODO: Обновление UBO буфера
}
