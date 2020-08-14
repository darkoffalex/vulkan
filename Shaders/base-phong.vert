#version 450
#extension GL_ARB_separate_shader_objects : enable

// Набор констант
#define MAX_SKELETON_BONES 50            // Максимальное кол-во костей скелета
#define MAX_BONE_WEIGHTS 4               // Максимальное кол-во весов кости на вершину

/*Схема входа-выхода*/

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUv;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in ivec4 inBoneIndices;
layout (location = 5) in vec4 inWeights;

layout (location = 0) out VS_OUT
{
    vec3 color;            // Цвет вершины
    vec3 position;         // Положение вершины в мировых координатах
    vec3 positionLocal;    // Положение вершины в локальных координатах
    vec3 normal;           // Вектор нормали вершины
    vec2 uv;               // Текстурные координаты
    mat3 normalMatrix;     // Матрица преобразования нормалей (не интерполируется, наверное)
} vs_out;

/*Вспомогательные типы*/

// Параметры отображения текстуры
struct TextureMapping
{
    vec2 offset;
    vec2 origin;
    vec2 scale;
    float angle;
};

/*Uniform*/

layout(set = 0, binding = 0, std140) uniform UniformCamera {
    mat4 _view;
    mat4 _proj;
    vec3 _camPosition;
};

layout(set = 2, binding = 0, std140) uniform UniformModel {
    mat4 _model;
};

layout(set = 2, binding = 1, std140) uniform UniformTextureMapping {
    TextureMapping _textureMapping;
};

layout(set = 2, binding = 5, std140) uniform SkeletonBoneCount {
    uint _boneCount;
};

layout(set = 2, binding = 6, std140) uniform SkeletonBoneTransforms {
    mat4 _boneTransforms[MAX_SKELETON_BONES];
};

/*Функции*/

// Получить матрицу поворота 2x2 на заданный угол
// Используется для вращения UV координат (для разворота текстуры)
mat2 rotate2D(float angle)
{
    return mat2(
        cos(angle),-sin(angle),
        sin(angle),cos(angle)
    );
}

// Подсчет UV координат с учетом параметров текстурирования
vec2 calcUV(vec2 uv, TextureMapping mapping)
{
    return ((rotate2D(radians(mapping.angle)) * (uv - mapping.origin)) + mapping.origin) * mapping.scale - mapping.offset;
}

// Основная функция вершинного шейдера
// Преобразование координат (и прочих параметров) вершины и передача их следующим этапам
void main()
{
    // Итоговые значения положения и нормали в пространстве модели
    vec4 position = vec4(0.0f);
    vec4 normal = vec4(0.0f);

    // Матрица преобразования нормалей
    // Учитывает только поворот, без искажения нормалей в процессе масштабирования
    mat3 normalMatrix = transpose(inverse(mat3(_model)));

    // Пройтись по всем компонентам вектора весов
    for(uint i = 0; i < MAX_BONE_WEIGHTS; i++)
    {
        // Получаем индекс кости и соответствующий вес
        int boneIndex = inBoneIndices[i];
        float weight = inWeights[i];

        // Если индек кости валиден - произвести приращение значения положения и нормали с учетом трансформации кости
        if(boneIndex >= 0 && boneIndex < MAX_SKELETON_BONES)
        {
            position += (_boneTransforms[boneIndex] * vec4(inPosition, 1.0)) * weight;
            normal += (_boneTransforms[boneIndex] * vec4(inNormal,0.0)) * weight;
        }
    }

    // Координаты вершины после всех преобразований
    gl_Position = _proj * _view * _model * position;

    // Передача матрицы нормалей
    vs_out.normalMatrix = normalMatrix;

    // Цвет вершины передается как есть
    vs_out.color = inColor;

    // Положение вершины в мировых координатах
    vs_out.position = (_model * position).xyz;

    // Положение вершины в локальном пространстве
    vs_out.positionLocal = position.xyz;

    // UV координаты текстуры для вершины
    vs_out.uv = calcUV(inUv, _textureMapping);

    // Нормаль вершины - используем матрицу нормалей для корректной передачи
    vs_out.normal = normalize(normalMatrix * normal.xyz);
}