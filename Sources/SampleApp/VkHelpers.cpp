#include "VkHelpers.h"
#include "Tools/Tools.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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
         * Загрузка геометрии меша из файла 3D-моделей
         * @param pRenderer Указатель на рендерер
         * @param filename Имя файла в папке Models
         * @param loadWeightInformation Загружать информацию о весах и костях
         * @return Smart pointer объекта геометрического буфера
         */
        vk::resources::GeometryBufferPtr LoadVulkanGeometryMesh(VkRenderer *pRenderer, const std::string &filename, bool loadWeightInformation)
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

            // Инициализировать индексы костей и веса для вершин (если нужно и если они есть)
            if(loadWeightInformation && pFirstMesh->HasBones())
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

                    // Наиболее влияющие кости и соответствующие веса (считать изначально 0-вая кость оказывает максимальное влияние)
                    glm::ivec4 mostInfluencedBones(0,0,0,0);
                    glm::vec4 mostInfluenceWeights(1.0f,0.0f,0.0f,0.0f);

                    // Получить итоговые значения
                    for(size_t j = 0; j < maxWeightsPerVertex; j++)
                    {
                        if(!vertexWeights.empty())
                        {
                            // Найти индекс максимального веса
                            auto maxWeightIndex = std::distance(vertexWeights.begin(),std::max_element(vertexWeights.begin(), vertexWeights.end()));

                            // Добавить индекс кости которая влияет более всего на данную вершину
                            (&mostInfluencedBones.x)[j] = vertexBones[maxWeightIndex];
                            // Добавить вес кости которая влияет более всего на данную вершину
                            (&mostInfluenceWeights.x)[j] = vertexWeights[maxWeightIndex];

                            // Удалить элементы из массивов по данному индексу (на следующей итерации максимальный будет уже другой)
                            vertexBones.erase(vertexBones.begin() + maxWeightIndex);
                            vertexWeights.erase(vertexWeights.begin() + maxWeightIndex);
                        }
                    }

                    // Индексы костей у вершины
                    vertices[i].boneIndices = mostInfluencedBones;
                    // Соответствующие веса (поскольку в сумме веса должны давать единицу, необходимо нормализовать этот вектор)
                    vertices[i].weights = glm::normalize(mostInfluenceWeights);
                }
            }

            // Отдать smart-pointer объекта ресурса геометрического буфера
            return pRenderer->createGeometryBuffer(vertices,indices);
        }

        // Методы для конвертирования векторов, матриц и кватернионов из Assimp в GLM
        static inline glm::vec3 ToGlmVec3(const aiVector3D &v) { return glm::vec3(v.x, v.y, v.z); }
        static inline glm::vec2 ToGlmVec2(const aiVector3D &v) { return glm::vec2(v.x, v.y); }
        static inline glm::quat ToGlmQuat(const aiQuaternion &q) { return glm::quat(q.w, q.x, q.y, q.z); }
        static inline glm::mat4 ToGlmMat4(const aiMatrix4x4 &m) { return glm::transpose(glm::make_mat4(&m.a1)); }
        static inline glm::mat4 ToGlmMat4(const aiMatrix3x3 &m) { return glm::transpose(glm::make_mat3(&m.a1)); }

        /**
         * Рекурсивное заполнение данных скелета
         * @param assimpBoneName Наименование кости assimp
         * @param bone Текущая кость
         * @param assimpBones Ассоциативный массив костей assimp (ключ - имя кости)
         * @param assimpBoneIndices Ассоциативный массив индексов костей (ключ - имя кости)
         */
        void RecursivePopulateSkeleton(const std::string& assimpBoneName,
                                       const vk::scene::MeshSkeleton::BonePtr& bone,
                                       const std::unordered_map<std::string, aiBone*>& assimpBones,
                                       const std::unordered_map<std::string, size_t>& assimpBoneIndices,
                                       const aiScene* scene)
        {
            // Получить текущую кость assimp
            auto assimpBone = assimpBones.at(assimpBoneName);

            // Если у кости есть потомки
            if(assimpBone->mNode->mNumChildren > 0)
            {
                // Пройтись по ним
                for(size_t i = 0; i < assimpBone->mNode->mNumChildren; i++)
                {
                    // Получить необходимые данные о потомке
                    auto childNode = assimpBone->mNode->mChildren[i];

                    // Если такого индекса кости не обнаружено - пропуск итерации
                    if(assimpBoneIndices.find(childNode->mName.C_Str()) == assimpBoneIndices.end())
                        continue;

                    // Получить индекс
                    auto childIndex = assimpBoneIndices.at(childNode->mName.C_Str());

                    // Добавить нового потомка в текущую кость
                    auto child = bone->addChildBone(childIndex,ToGlmMat4(childNode->mTransformation),glm::mat4(1.0f));

                    // Рекурсивно выполнить эту функцию для потомка
                    RecursivePopulateSkeleton(childNode->mName.C_Str(), child, assimpBones, assimpBoneIndices, scene);
                }
            }
        }

        /**
         * Загрузка скелета из файла 3D-моделей
         * @param filename Имя файла в папке Models
         * @return Объект скелета
         */
        vk::scene::UniqueMeshSkeleton LoadVulkanMeshSkeleton(const std::string &filename)
        {
            // Итоговый скелет
            vk::scene::UniqueMeshSkeleton skeleton = std::make_unique<vk::scene::MeshSkeleton>();

            // Полный путь к файлу
            auto path = ::tools::ExeDir().append("..\\Models\\").append(filename);

            // Импортер Assimp
            Assimp::Importer importer;

            // Получить сцену
            const aiScene* scene = importer.ReadFile(path.c_str(),
                    aiProcess_Triangulate |
                    aiProcess_JoinIdenticalVertices |
                    //aiProcess_PreTransformVertices |
                    aiProcess_FlipWindingOrder |
                    aiProcess_PopulateArmatureData
            );

            // Если не удалось загрузить
            if(scene == nullptr){
                throw std::runtime_error(std::string("Can't load geometry from (").append(path).append(")").c_str());
            }

            // Если нет геометрических мешей
            if(!scene->HasMeshes()){
                throw std::runtime_error(std::string("Can't find any geometry meshes from (").append(path).append(")").c_str());
            }

            // Первый меш сцены
            auto pFirstMesh = scene->mMeshes[0];

            // Если у меша есть кости
            if(pFirstMesh->HasBones())
            {
                // Инициализировать скелет
                skeleton = std::make_unique<vk::scene::MeshSkeleton>(pFirstMesh->mNumBones);

                // Ассоциативный массив костей Assimp
                std::unordered_map<std::string, aiBone*> bones{};
                // Ассоциативный массив индексов костей
                std::unordered_map<std::string, size_t> indices{};

                // Пройтись по костям скелета и заполнить ассоциативные массив костей и индексов для доступа по именам
                for(size_t i = 0; i < pFirstMesh->mNumBones; i++)
                {
                    bones[pFirstMesh->mBones[i]->mName.C_Str()] = pFirstMesh->mBones[i];
                    indices[pFirstMesh->mBones[i]->mName.C_Str()] = i;
                }

                // Установить значение корневой кости скелета
                auto rootBone = pFirstMesh->mBones[0];
                skeleton->getRootBone()->setTransformations(ToGlmMat4(rootBone->mNode->mTransformation),glm::mat4(1.0f));

                // Добавление дочерних костей
                RecursivePopulateSkeleton(pFirstMesh->mBones[0]->mName.C_Str(), skeleton->getRootBone(), bones, indices, scene);
            }

            // Отдать скелет
            return skeleton;
        }
    }
}