#include "VkHelpers.h"
#include "Tools/Tools.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
         * Загрузка геометрии меша из файла 3D-моделей
         * @param pRenderer Указатель на рендерер
         * @param filename Имя файла в папке Models
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr LoadVulkanGeometryMesh(VkRenderer *pRenderer, const std::string &filename)
        {
            // Полный путь к файлу
            auto path = ::tools::ExeDir().append("..\\Models\\").append(filename);

            // Импортер Assimp
            Assimp::Importer importer;

            // Получить сцену
            const aiScene* scene = importer.ReadFile(path.c_str(),
                    aiProcess_Triangulate |
                    aiProcess_JoinIdenticalVertices |
                    //aiProcess_PreTransformVertices |
                    aiProcess_FlipWindingOrder
            );

            // Если не удалось загрузить
            if(scene == nullptr){
                throw std::runtime_error(std::string("Can't load geometry from (").append(path).append(")").c_str());
            }

            // Если нет геометрических мешей
            if(!scene->HasMeshes()){
                throw std::runtime_error(std::string("Can't find any geometry meshes from (").append(path).append(")").c_str());
            }

            // Результирующие массивы вершин и индексов
            std::vector<vk::tools::Vertex> vertices;
            std::vector<size_t> indices;

            // Первый меш сцены
            auto pFirstMesh = scene->mMeshes[0];

            // Заполнить массив вершин
            for(size_t i = 0; i < pFirstMesh->mNumVertices; i++)
            {
                vk::tools::Vertex v{};
                v.position = {pFirstMesh->mVertices[i].x,pFirstMesh->mVertices[i].y,pFirstMesh->mVertices[i].z};
                v.normal = {pFirstMesh->mNormals[i].x,pFirstMesh->mNormals[i].y,pFirstMesh->mNormals[i].z};
                if(pFirstMesh->HasTextureCoords(0)) v.uv = {pFirstMesh->mTextureCoords[0][i].x,pFirstMesh->mTextureCoords[0][i].y};
                v.color = {1.0f,1.0f,1.0f};

                vertices.push_back(v);
            }

            // Заполнить массив индексов
            for(size_t i = 0; i < pFirstMesh->mNumFaces; i++)
            {
                auto face = pFirstMesh->mFaces[i];
                for(size_t j = 0; j < face.mNumIndices; j++){
                    indices.push_back(face.mIndices[j]);
                }
            }

            // Инициализировать индексы костей и веса для вершин (если есть)
            if(pFirstMesh->HasBones())
            {
                // Массив массивов весов для каждой вершины (на каждую вершину - массив весов, индексы совпадают с массивом vertices)
                std::vector<std::vector<glm::float32_t>> weights(vertices.size());
                // Массив массивов индексов костей для каждой вершины (на каждую вершину - массив индексов, индексы совпадают с массивом vertices)
                std::vector<std::vector<size_t>> bones(vertices.size());

                // Пройтись по костям скелета и наполнить массив весов
                for(size_t i = 0; i < pFirstMesh->mNumBones; i++)
                {
                    auto bone = pFirstMesh->mBones[i];
                    for(size_t j = 0; j < bone->mNumWeights; j++)
                    {
                        auto weightData = bone->mWeights[j];

                        // Добавляем вершине вес
                        weights[weightData.mVertexId].push_back(weightData.mWeight);
                        // Добавляем вершине индекс кости соответствующей весу
                        bones[weightData.mVertexId].push_back(i);
                    }
                }

                // Максимальное кол-во весов у вершины
                const size_t maxWeightsPerVertex = 4;

                // Инициализация весов у вершин
                for(size_t i = 0; i < weights.size(); i++)
                {
                    // Массивы весов и костей для вершины под индексом i
                    auto& vertexWeights = weights[i];
                    auto& vertexBones = bones[i];

                    // Несколько наиболее значимых весов и костей (итоговые значения передаваемые в вершинных атрибутах)
                    std::vector<glm::float32_t> mostInfluenceWeights;
                    std::vector<size_t> mostInfluencedBones;

                    // Получить итоговые значения
                    for(size_t j = 0; j < maxWeightsPerVertex; j++)
                    {
                        if(!vertexWeights.empty())
                        {
                            // Найти индекс максимального веса
                            auto maxWeightIndex = std::distance(vertexWeights.begin(),std::max_element(vertexWeights.begin(), vertexWeights.end()));

                            // Добавить индекс кости которая влияет более всего на данную вершину
                            mostInfluencedBones.push_back(vertexBones[maxWeightIndex]);
                            // Добавить вес кости которая влияет более всего на данную вершину
                            mostInfluenceWeights.push_back(vertexWeights[maxWeightIndex]);

                            // Удалить элементы из массивов по данному индексу (на следующей итерации максимальный будет уже другой)
                            vertexBones.erase(vertexBones.begin() + maxWeightIndex);
                            vertexWeights.erase(vertexWeights.begin() + maxWeightIndex);
                        }
                        else
                        {
                            // Отрицательное значение индекса кости - кость отсутствует
                            mostInfluencedBones.push_back(-1);
                            // При отсутствии кости вес не имеет значения, указывается 0
                            mostInfluenceWeights.push_back(0.0f);
                        }
                    }

                    // Индексы костей у вершины
                    vertices[i].boneIndices = {
                            mostInfluencedBones[0],
                            mostInfluencedBones[1],
                            mostInfluencedBones[2],
                            mostInfluencedBones[3]
                    };

                    // Соответствующие веса (поскольку в сумме веса должны давать единицу, необходимо нормализовать этот вектор)
                    vertices[i].weights = glm::normalize(glm::vec4({
                            mostInfluenceWeights[0],
                            mostInfluenceWeights[1],
                            mostInfluenceWeights[2],
                            mostInfluenceWeights[3]
                    }));
                }
            }

            // Отдать smart-pointer объекта ресурса геометрического буфера
            return pRenderer->createGeometryBuffer(vertices,indices);
        }

        /**
         * Временный тестовый метод по загрузке информации о скелете
         * @param filename Имя файла в папке Models
         */
        void LoadSkeleton(const std::string &filename)
        {
            //TODO: загрузка информации о скелете
        }
    }
}