#include "VkHelpers.h"
#include "Tools/Tools.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

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
        vk::resources::TextureBufferPtr LoadVulkanTexture(VkRenderer *pRenderer, const std::string &filename, bool mip, bool sRgb)
        {
            // Полный путь к файлу
            auto path = ::tools::ExeDir().append("..\\Textures\\").append(filename);

            // Параметры изображения
            int width, height, channels;
            // Включить вертикальный flip
            stbi_set_flip_vertically_on_load(true);
            // Загрузить
            unsigned char* bytes = stbi_load(path.c_str(),&width,&height,&channels,STBI_rgb_alpha);

            // Если не удалось загрузить
            if(bytes == nullptr){
                throw std::runtime_error(std::string("Can't load texture (").append(path).append(")").c_str());
            }

            //TODO: сделать определение кол-ва каналов и байт на пиксель

            // Создать ресурс текстуры
            // Для простоты пока будем считать что кол-вао байт на пиксель равно кол-ву каналов
            auto texture = pRenderer->createTextureBuffer(
                    bytes,
                    static_cast<size_t>(width),
                    static_cast<size_t>(height),
                    static_cast<size_t>(4),
                    mip,
                    sRgb);

            // Очистить память
            stbi_image_free(bytes);

            // Отдать smart-pointer объекта ресурса текстуры
            return texture;
        }

        /**
         * Генерация геометрии квадрата
         * @param pRenderer Указатель на рендерер
         * @param size Размер стороны квадрата
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr GenerateQuadGeometry(VkRenderer *pRenderer, float size)
        {
            // Вершины
            std::vector<vk::tools::Vertex> vertices = {
                    { { (size / 2),(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,1.0f} },
                    { { (size / 2),-(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,1.0f} },
                    { { -(size / 2),-(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,1.0f} },
                    { { -(size / 2),(size / 2),0.0f },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,1.0f} },
            };

            // Индексы
            std::vector<unsigned> indices = {
                    0,1,2, 0,2,3,
            };

            // Отдать smart-pointer объекта ресурса геометрического буфера
            return pRenderer->createGeometryBuffer(vertices,indices);
        }

        /**
         * Генерация геометрии куба
         * @param pRenderer Указатель на рендерер
         * @param size Размер стороны куба
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr GenerateCubeGeometry(VkRenderer *pRenderer, float size)
        {
            // Вершины
            std::vector<vk::tools::Vertex> vertices = {
                    { { (size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,1.0f} },
                    { { (size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,1.0f} },
                    { { -(size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,1.0f} },
                    { { -(size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,1.0f} },

                    { { (size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {1.0f,0.0f,0.0f} },
                    { { (size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {1.0f,0.0f,0.0f} },
                    { { (size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {1.0f,0.0f,0.0f} },
                    { { (size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {1.0f,0.0f,0.0f} },

                    { { (size / 2),  (size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f },{0.0f,1.0f,0.0f} },
                    { { (size / 2), (size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f },{0.0f,1.0f,0.0f} },
                    { { -(size / 2), (size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,1.0f,0.0f} },
                    { { -(size / 2),  (size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,1.0f,0.0f} },

                    { { -(size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,-1.0f}  },
                    { { -(size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,-1.0f}  },
                    { { (size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,-1.0f}  },
                    { { (size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,-1.0f}  },

                    { { -(size / 2),(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {-1.0f,0.0f,0.0f} },
                    { { -(size / 2),-(size / 2),(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {-1.0f,0.0f,0.0f} },
                    { { -(size / 2),-(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {-1.0f,0.0f,0.0f} },
                    { { -(size / 2),(size / 2),-(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {-1.0f,0.0f,0.0f} },

                    { { -(size / 2),  -(size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,-1.0f,0.0f} },
                    { { -(size / 2), -(size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,-1.0f,0.0f}  },
                    { { (size / 2), -(size / 2), (size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,-1.0f,0.0f}  },
                    { { (size / 2),  -(size / 2), -(size / 2) },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,-1.0f,0.0f}  },
            };

            // Индексы
            std::vector<unsigned> indices = {
                    0,1,2, 0,2,3,
                    4,5,6, 4,6,7,
                    8,9,10, 8,10,11,
                    12,13,14, 12,14,15,
                    16,17,18, 16,18,19,
                    20,21,22, 20,22,23,
            };

            // Отдать smart-pointer объекта ресурса геометрического буфера
            return pRenderer->createGeometryBuffer(vertices,indices);
        }

        /**
         * Генерация геометрии сферы
         * @param pRenderer Указатель на рендерер
         * @param segments Кол-во сегментов
         * @param radius Радиус
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr GenerateSphereGeometry(VkRenderer *pRenderer, unsigned segments, float radius)
        {
            const float pi = 3.14159265359f;

            std::vector<vk::tools::Vertex> vertices = {};
            std::vector<unsigned> indices = {};

            // Вершины
            for (unsigned int y = 0; y <= segments; ++y)
            {
                for (unsigned int x = 0; x <= segments; ++x)
                {
                    // В какой части окружности (по 2 осям) мы находимся на конкретной итерации
                    const float xSegment = static_cast<float>(x) / static_cast<float>(segments);
                    const float ySegment = static_cast<float>(y) / static_cast<float>(segments);

                    // Вычисляем положение вершины
                    const float xPos = std::cos(xSegment * 2.0f * pi) * std::sin(ySegment * pi);
                    const float yPos = std::cos(ySegment * pi);
                    const float zPos = std::sin(xSegment * 2.0f * pi) * std::sin(ySegment * pi);

                    // Добавляем вершину
                    vertices.push_back({
                            {xPos * radius,yPos * radius,zPos * radius},
                            {1.0f,1.0f,1.0f},
                            {xSegment,ySegment},
                            {xPos,yPos,zPos}
                    });
                }
            }

            // Индексы
            for (unsigned latNumber = 0; latNumber < segments; latNumber++) {
                for (unsigned int longNumber = 0; longNumber < segments; longNumber++) {
                    const unsigned first = latNumber * (segments + 1) + longNumber;
                    const unsigned second = first + segments + 1;

                    indices.push_back(first);
                    indices.push_back(second);
                    indices.push_back(first + 1);

                    indices.push_back(second);
                    indices.push_back(second + 1);
                    indices.push_back(first + 1);
                }
            }

            // Отдать smart-pointer объекта ресурса геометрического буфера
            return pRenderer->createGeometryBuffer(vertices,indices);
        }
    }
}