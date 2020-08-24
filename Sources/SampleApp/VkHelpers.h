#pragma once

#include "VkRenderer.h"
#include "VkResources/TextureBuffer.hpp"
#include "VkResources/GeometryBuffer.hpp"

#include "VkScene/MeshSkeleton.hpp"
#include "VkScene/MeshSkeletonAnimation.hpp"

namespace vk
{
    namespace helpers
    {
        /**
         * Создание ресурса текстуры Vulkan из файла изображения
         * @param pRenderer Указатель на рендерер
         * @param filename Имя файла в папке Textures
         * @param mip Генерировать мип-уровни
         * @param sRgb Использовать цветовое пространство sRGB (гамма-коррекция)
         * @return Smart pointer объекта буфера текстуры
         */
        vk::resources::TextureBufferPtr LoadVulkanTexture(
                VkRenderer* pRenderer,
                const std::string &filename,
                bool mip = false,
                bool sRgb = false);

        /**
         * Генерация геометрии квадрата
         * @param pRenderer Указатель на рендерер
         * @param size Размер стороны квадрата
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr GenerateQuadGeometry(VkRenderer* pRenderer, float size);

        /**
         * Генерация геометрии куба
         * @param pRenderer Указатель на рендерер
         * @param size Размер стороны куба
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr GenerateCubeGeometry(VkRenderer* pRenderer, float size);

        /**
         * Генерация геометрии сферы
         * @param pRenderer Указатель на рендерер
         * @param segments Кол-во сегментов
         * @param radius Радиус
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr GenerateSphereGeometry(VkRenderer* pRenderer, unsigned segments, float radius);

        /**
         * Загрузка геометрии меша из файла 3D-моделей
         * @param pRenderer Указатель на рендерер
         * @param filename Имя файла в папке Models
         * @param loadWeightInformation Загружать информацию о весах и костях
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr LoadVulkanGeometryMesh(VkRenderer* pRenderer, const std::string &filename, bool loadWeightInformation = false);

        /**
         * Загрузка скелета из файла 3D-моделей
         * @param filename Имя файла в папке Models
         * @return Объект скелета
         */
         vk::scene::UniqueMeshSkeleton LoadVulkanMeshSkeleton(const std::string &filename);

         /**
          * Загрузка набора скелетных анимаций из файла 3D-моделей
          * @param filename Имя файла в папке Models
          * @return Массив указателей на скелетные анимации
          */
         std::vector<vk::scene::MeshSkeletonAnimationPtr> LoadVulkanMeshSkeletonAnimations(const std::string &filename);
    }
}
