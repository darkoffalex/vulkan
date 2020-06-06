#version 450
#extension GL_ARB_separate_shader_objects : enable

/*Схема входа-выхода*/

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 normal;

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

layout(set = 0, binding = 0) uniform UniformViewProjection {
    mat4 _view;
    mat4 _proj;
};

layout(set = 1, binding = 0) uniform UniformModel {
    mat4 _model;
};

layout(set = 1, binding = 1) uniform UniformTextureMapping {
    TextureMapping _textureMapping;
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
    return ((rotate2D(mapping.angle) * (uv - mapping.origin)) + mapping.origin)*mapping.scale - mapping.offset;
}

// Основная функция вершинного шейдера
// Преобразование координат (и прочих параметров) вершины и передача их следующим этапам
void main()
{
    // Координаты вершины после всех преобразований
    gl_Position =  _proj * _view * _model * vec4(position, 1.0);

    // Матрица преобразования нормалей
    // Учитывает только поворот, без искажения нормалей в процессе масштабирования
    vs_out.normalMatrix = mat3(transpose(inverse(_model)));

    // Цвет вершины передается как есть
    vs_out.color = color;

    // Положение вершины в мировых координатах
    vs_out.position = (_model * vec4(position, 1.0)).xyz;

    // Положение вершины в локальном пространстве
    vs_out.positionLocal = position;

    // UV координаты текстуры для вершины
    vs_out.uv = calcUV(uv, _textureMapping);

    // Нормаль вершины - используем матрицу нормалей для корректной передачи
    vs_out.normal = normalize(vs_out.normalMatrix * normal);
}