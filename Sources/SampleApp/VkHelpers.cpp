#include "VkHelpers.h"
#include "Tools/Tools.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>
#include <rapidxml/rapidxml.hpp>

#include <sstream>
#include <unordered_map>

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

        /**
         * Заполнение массива float значений из узла XML в .dae (collada) файле
         * @param floatArray Указатель на массив значений
         * @param node Указатель на узел XML
         */
        void DaeGeomNodeFillFloatArray(glm::float32_t* floatArray, rapidxml::xml_node<>* node)
        {
            // Указатель на узел "accessor" содержащий правила чтения массива
            auto pAccessorNode = node->first_node("technique_common",16)
                    ->first_node("accessor",8);

            // Получить кол-во позиций
            auto count = static_cast<size_t>(std::stoi(pAccessorNode->first_attribute("count",5)->value()));
            // Получить шаг (сколько элементов массива считается одним элементом)
            auto stride = static_cast<size_t>(std::stoi(pAccessorNode->first_attribute("stride",6)->value()));

            // Доступ к набору чисел как к строковому потоку
            std::stringstream positionsStream(node->first_node("float_array",11)->value());

            // Заполнение массива
            for(size_t i = 0; i < count * stride; i++)
            {
                glm::float32_t val; positionsStream >> val;
                *(floatArray + i) = val;
            }
        }

        /**
         * Получить кол-во элементов в узле XML в .dae (collada) файле
         * @param node Указатель на узел XML
         * @return
         */
        size_t DaeGeomNodeGetCount(rapidxml::xml_node<>* node)
        {
            auto count = std::stoi(node->first_node("technique_common",16)
                    ->first_node("accessor",8)
                    ->first_attribute("count",5)->value());

            return static_cast<size_t>(count);
        }

        /**
         * Загрузка геометрии из .dae (collada) файла
         * @param pRenderer Указатель на рендерер
         * @param xmlString Строка содержащая XML
         * @param Сменить порядок обхода вершин
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr LoadGeometryFromDaeString(VkRenderer *pRenderer, char *xmlString, bool swapVertexOrder)
        {
            // Парсинг
            rapidxml::xml_document<> document;
            document.parse<0>(xmlString);

            // Указатель на узел "mesh" в XML файле .dae
            auto firstMeshNodePtr = document.first_node()
                    ->first_node("library_geometries",18)
                    ->first_node("geometry",8)
                    ->first_node("mesh",4);

            // Указатели на узлы с нужными параметрами в XML документе
            auto positionsNodePtr = firstMeshNodePtr->first_node();
            auto normalsNodePtr = positionsNodePtr->next_sibling();
            auto uvNodePtr = normalsNodePtr->next_sibling();

            // Загрузить данные из файла в массивы
            std::vector<glm::vec3> positions(DaeGeomNodeGetCount(positionsNodePtr));
            DaeGeomNodeFillFloatArray(reinterpret_cast<glm::float32_t*>(positions.data()),positionsNodePtr);

            std::vector<glm::vec3> normals(DaeGeomNodeGetCount(normalsNodePtr));
            DaeGeomNodeFillFloatArray(reinterpret_cast<glm::float32_t*>(normals.data()),normalsNodePtr);

            std::vector<glm::vec2> uvs(DaeGeomNodeGetCount(uvNodePtr));
            DaeGeomNodeFillFloatArray(reinterpret_cast<glm::float32_t*>(uvs.data()),uvNodePtr);

            // Узел "triangles", его атрибуты и под-узлы
            auto verticesNodePtr = firstMeshNodePtr->first_node("triangles",9);
            auto maxOffsetAttrPtr = verticesNodePtr->last_node("input",5)->first_attribute("offset",6);
            auto dataNodePtr = verticesNodePtr->first_node("p",1);

            // Количества
            auto trianglesCount = static_cast<size_t>(std::stoi(verticesNodePtr->first_attribute("count",5)->value()));
            auto vertexCount = trianglesCount * 3;
            auto elementsPerVertex = static_cast<size_t>(std::stoi(maxOffsetAttrPtr->value()) + 1);

            // Доступ к набору чисел как к строковому потоку
            std::stringstream indicesStream(dataNodePtr->value());

            // Итоговые массивы вершин и индексов
            std::vector<vk::tools::Vertex> vertices;
            std::vector<size_t> indices;

            // Ассоциативный массив для избежания добавления ранее добавленных вершин
            // Ключ массива - уникальная строка из индексов позиции, нормали, и UV координат
            std::unordered_map<std::string,size_t> indicesOfVertices;

            // Пройтись по всем вершинам
            for(size_t i = 0; i < vertexCount; i++)
            {
                // Индексы атрибутов вершин в массивах заданных выше
                size_t posIndex = 0;
                size_t normIndex = 0;
                size_t uvIndex = 0;

                // Массив указателей для доступа по индексу
                size_t* pIndices[3] = {&posIndex, &normIndex, &uvIndex};

                // Проход по атрибутам
                for(size_t j = 0; j < elementsPerVertex; j++)
                {
                    size_t value;
                    indicesStream >> value;

                    if(j < 3){
                        *(pIndices[j]) = value;
                    }
                }

                // Получить уникальный ключ-строку из индексов (ключ соответствует вершине)
                std::string keyString = std::to_string(posIndex).
                        append(std::to_string(normIndex)).
                        append(std::to_string(uvIndex));

                // Если такая вершина (с такими атрибутами) уже существует - добавить в массив индексов ее индекс
                if(indicesOfVertices.find(keyString) != indicesOfVertices.end())
                {
                    indices.push_back(indicesOfVertices.at(keyString));
                }
                // Если такой вершины еще не было
                else
                {
                    // Создать вершину
                    vk::tools::Vertex v{};
                    v.position = positions[posIndex];
                    v.normal = normals[normIndex];
                    v.uv = uvs[uvIndex];
                    v.color = {1.0f,1.0f,1.0f};

                    // Добавить в массив вершин
                    vertices.push_back(v);

                    // Получить индекс новой вершины
                    size_t index = vertices.size()-1;

                    // Добавить в ассоциативный массив этот индекс
                    indicesOfVertices[keyString] = index;

                    // Добавить в массив индексов
                    indices.push_back(index);
                }
            }

            if(swapVertexOrder){
                std::reverse(indices.begin(),indices.end());
            }

            // Отдать smart-pointer объекта ресурса геометрического буфера
            return pRenderer->createGeometryBuffer(vertices,indices);
        }
    }
}