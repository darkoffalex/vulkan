#version 450
#extension GL_ARB_separate_shader_objects : enable

/*Схема входа-выхода*/

layout (location = 0) out vec4 outColor;

layout (location = 0) in VS_OUT
{
    vec3 color;            // Цвет фрагмента
    vec3 position;         // Положение фрагмента в мировых координатах
    vec3 positionLocal;    // Положение фрагмента в локальных координатах
    vec3 normal;           // Вектор нормали фрагмента
    vec2 uv;               // Текстурные координаты фрагмента
    mat3 normalMatrix;     // Матрица преобразования нормалей
} fs_in;

/*Вспомогательные типы*/

// Параметры материала
struct MaterialSettings
{
    vec3 albedo;
    float metallic;
    float roughness;
};

/*Uniform*/

layout(set = 1, binding = 2) uniform UniformModel {
    MaterialSettings _materialSettings;
};

layout(set = 1, binding = 3) uniform sampler2D _texture;

/*Функции*/

// Основная функция вершинного шейдера
// Вычисление итогового цвета фрагмента (пикселя)
void main() {

    //TODO: Сделать базовое фонговое освещение

    // Получить размер текстуры
    ivec2 texSize = textureSize(_texture,0);

    // Отдаем либо цвет материала либо цвет текстуры (если она есть)
    outColor = texSize.x > 1 ? vec4(texture(_texture,fs_in.uv).rgb, 1.0f) : vec4(_materialSettings.albedo, 1.0f);
}
