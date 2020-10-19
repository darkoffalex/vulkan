#version 460
#extension GL_EXT_ray_tracing : enable

/*Вспомогательные типы*/

// Полезная нагрузка луча
struct RayPayload
{
    vec3 hitColor;
    bool done;
    
    vec3 newRayOrigin;
    vec3 newRayDirection;
    float newRayStrength;
};

/* Ввод - вывод */

layout(location = 0) rayPayloadInEXT RayPayload rayData;

/*Функции*/

void main()
{
    // Цвет "неба"
    rayData.hitColor = vec3(0.0, 0.1, 0.3);
    // Луч завершен
    rayData.done = true;
}