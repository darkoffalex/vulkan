#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_scalar_block_layout : enable

/*Константы*/

#define LIGHT_TYPE_POINT 0        // Тип источника - точеченый
#define LIGHT_TYPE_SPOT 1         // Тип источника - прожектор
#define LIGHT_TYPE_DIRECTIONAL 2  // Тип источника - направленный
#define MAX_LIGHTS 100            // Максимальное число источников

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

// Вершина
struct Vertex
{
    vec3 position;
    vec3 color;
    vec2 uv;
    vec3 normal;

    ivec4 boneIndices;
    vec4 wights;
};

// Параметры источника света
struct LightSource
{
    vec3 position;
    float radius;
    vec3 color;
    vec3 orientation;
    float attenuationQuadratic;
    float attenuationLinear;
    float cutOffAngleCos;
    float cutOffOuterAngleCos;
    uint type;
};

// Параметры матриала (Blinn-Phong)
struct Material
{
    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
    float reflectance;
};

/*Uniform*/

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;

layout(binding = 2, set = 0) buffer Indices { uint i[]; } _indices[];

layout(binding = 3, set = 0, scalar) buffer Vertices { Vertex v[]; } _vertices[];

layout(binding = 4, set = 0, std140) buffer ModelMatrices { mat4 m; } _matrices[];

layout(binding = 5, set = 0, std140) buffer Materials { Material m; } _materials[];

layout(binding = 0, set = 1, std140) uniform UniformCamera {
    mat4 _view;
    mat4 _proj;
    mat4 _camModel;
    vec3 _camPosition;
    float _fov;
};

layout(binding = 0, set = 2, std140) uniform UniformLightCount {
    uint _lightCount;
};

layout(binding = 1, set = 2, std140) uniform UniformLightArray {
    LightSource _lightSources[MAX_LIGHTS];
};

layout(binding = 0, set = 3, std140) uniform UniformFrameCounter {
    uint _frameCounter;
};

/* Ввод - вывод */

// Итоговое значение - на отправку
layout(location = 0) rayPayloadInEXT RayPayload rayData;

// Засчитано ли пересечение с преградой - на вход
layout(location = 1) rayPayloadEXT bool isShadowed;

// Барицентрические координаты на трекгольнике
hitAttributeEXT vec2 attribs;

/*Функции*/

// Получить интерполированные значения вершины
Vertex interpolatedVertex(Vertex[3] vertices, vec2 barycentric)
{
    // Результирующий объект с интерполированными значениями
    Vertex result;

    // Значения полей структуры по отдельности
    vec3 pos[3] = vec3[3](vertices[0].position,vertices[1].position,vertices[2].position);
    vec3 col[3] = vec3[3](vertices[0].color,vertices[1].color,vertices[2].color);
    vec2 uvs[3] = vec2[3](vertices[0].uv,vertices[1].uv,vertices[2].uv);
    vec3 nor[3] = vec3[3](vertices[0].normal,vertices[1].normal,vertices[2].normal);

    // Интерполяция с использованием барицентрических координат
    result.position = pos[0] + ((pos[1] - pos[0]) * barycentric.x) + ((pos[2] - pos[0]) * barycentric.y);
    result.color = col[0] + ((col[1] - col[0]) * barycentric.x) + ((col[2] - col[0]) * barycentric.y);
    result.uv = uvs[0] + ((uvs[1] - uvs[0]) * barycentric.x) + ((uvs[2] - uvs[0]) * barycentric.y);
    result.normal = nor[0] + ((nor[1] - nor[0]) * barycentric.x) + ((nor[2] - nor[0]) * barycentric.y);

    return result;
}

// Подсчет дифузной компоненты
vec3 calcDiffuseComponent(LightSource l, Material m, vec3 pointToLight, vec3 pointNormal, vec3 pointColor)
{
    // Освещенность - скалярное произведение (косинус угла) векторов нормали и направления к источнику (больше угол - меньше свет)
    float brightness = max(dot(pointNormal, pointToLight), 0.0);
    // Итоговый цвет получаем перемножением цвета источника, цвета материала и цвета точки (подразумевается текстура)
    return brightness * l.color * m.diffuseColor * pointColor;
}

// Подсчет бликовой компоненты
vec3 calculateSpecularComponent(LightSource l, Material m, vec3 pointToLight, vec3 pointNormal, vec3 pointToView, bool blinnPhong, float pointSpecularity)
{
    // Использовать метод Блинна-Фонга
    if(blinnPhong)
    {
        // Половинный вектор (между векторами к зрителю и к источнику света)
        vec3 halfwayDir = normalize(pointToLight + pointToView);
        // Яркость блика - угол между половинным вектором и нормалью (меньше угол - ярче), степень (pow) - размер пятна
        float brightness = pow(max(dot(pointNormal, halfwayDir), 0.0), m.shininess);
        // Итоговый цвет получаем перемножением цвета источника, цвета материала и интенсивности блика точки
        return brightness * l.color * m.specularColor * pointSpecularity;
    }
    // Использовать метод Фонга
    else
    {
        // Отраженный вектор от источника к точки относительно нормали точки
        vec3 reflectedLightDir = reflect(-pointToLight, pointNormal);
        // Яркость блика - угол между векторами к зрителю и отраженным (меньше угол - ярче), степень (pow) - размер пятна
        float brightness = pow(max(dot(pointToView, reflectedLightDir), 0.0), m.shininess);
        // Итоговый цвет получаем перемножением цвета источника, цвета материала и интенсивности блика точки
        return brightness * l.color * m.specularColor * pointSpecularity;
    }
}

// Генерация псевдо-случайного unsigned int значения из двух unsigned int значений,
// используя 16 пар (Tiny Encryption Algorithm). Подробнее - Zafar, Olano, and Curtis,
// "GPU Random Numbers via the Tiny Encryption Algorithm"
uint tea(uint val0, uint val1)
{
  uint v0 = val0;
  uint v1 = val1;
  uint s0 = 0;

  for(uint n = 0; n < 16; n++)
  {
    s0 += 0x9e3779b9;
    v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
    v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
  }

  return v0;
}

// Генерация псевдо-случайного unsigned int значения в диапозоне [0, 2^24)
uint lcg(inout uint prev)
{
  uint LCG_A = 1664525u;
  uint LCG_C = 1013904223u;
  prev       = (LCG_A * prev + LCG_C);
  return prev & 0x00FFFFFF;
}

// Генерация случайного float в диапозоне [0, 1)
float rnd(inout uint prev)
{
  return (float(lcg(prev)) / float(0x01000000));
}

// Получить случайный вектор от точки до случайной точки на диске сферы источника света
vec3 randomVectorToLightSphere(vec3 pointPosition, vec3 lightPosition, float lightRadius, inout uint seed)
{
    // Вектор от точки до источника света (нормированный)
    vec3 toLight = normalize(lightPosition - pointPosition);

    // Рандомизированные значения для вектора смещения и радиуса
    float rndX = rnd(seed) - 0.5f;
    float rndY = rnd(seed) - 0.5f;
    float rndZ = rnd(seed) - 0.5f;
    float rndR = rnd(seed);

    // Случайный перпендикулярный вектор к вектору от точки до источника
    vec3 randomBias = vec3(rndX,rndY,rndZ);
    vec3 randomPperpL = normalize(cross(toLight, toLight + randomBias));

    // Случайная точка на диске сферы источника света
    vec3 randomPointOnLightDisk = lightPosition + randomPperpL * lightRadius * rndR;

    // Вектор до случайной точки на диске источника света
    return normalize(randomPointOnLightDisk - pointPosition);
}

// Подсчет освещенности фрагмента сферическим источником
vec3 sphericalLight(LightSource l, Material m, vec3 pointPosition, vec3 pointNormal, vec3 pointToView, vec3 pointColor, float pointSpecularity, uint shadowSamples, inout uint rndSeed)
{
    // Вектор от точки до источника света
    vec3 pointToLight = l.position - pointPosition;

    // Вектор от точки до источника света (нормированный)
    vec3 toLight = normalize(pointToLight);

    // Коэффициент затухания (использует расстояние до источника, а так же спец-коэффициенты)
    float dist = length(l.position - pointPosition);
    float attenuation = 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));

    // Компоненты освещенности
    vec3 diffuse = calcDiffuseComponent(l, m, toLight, pointNormal, pointColor);
    vec3 specular = calculateSpecularComponent(l, m, toLight, pointNormal, pointToView, false, pointSpecularity);
    vec3 ambient = l.color * m.ambientColor;

    // Множитель интенсивности освещенности
    float lightIntensity = 1.0f;

    // Если отбрасывает тень
    if(shadowSamples > 0)
    {
        // Доля интенсивности приходящаяся на 1 луч теневой проверки
        float lightIntPerSample = 1.0 / float(shadowSamples);

        // Параметры луча (точка начала, минимальная и максимальные длины)
        vec3 origin  = pointPosition;
        float tmin   = 0.001f;
        float tmax   = length(pointToLight);
        
        // Запустить лучи
        for(uint i = 0; i < shadowSamples; i++)
        {
            // Случайное направление впределах конуса
            vec3 toLightRandom = randomVectorToLightSphere(pointPosition, l.position, l.radius, rndSeed);

            // В тени по умолчанию
            isShadowed = true;

            // Запуск луча в сторону источника
            traceRayEXT(
                topLevelAS, 
                gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT, 
                0xFF, 
                0, 
                0, 
                1, 
                origin, 
                tmin, 
                toLightRandom, 
                tmax, 
                1);

            if(isShadowed) lightIntensity -= lightIntPerSample;
        }
    }

    // Вернуть суммарную интенсивность
    return lightIntensity * attenuation * (diffuse + specular + ambient);
}

// Основная функция шейдера
void main()
{
    // ID текущего объекта
    uint objId = gl_InstanceID;

    // Матрица модели
    mat4 modelMatrix = _matrices[objId].m;

    // Матрица преобразования нормалей
    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));

    // Индексы вершин в индексном буфере для треугольника, с которым произошло пересечение (gl_PrimitiveID - индекс треугольника)
    ivec3 ind = ivec3(_indices[objId].i[3 * gl_PrimitiveID + 0],
                      _indices[objId].i[3 * gl_PrimitiveID + 1],
                      _indices[objId].i[3 * gl_PrimitiveID + 2]);

    // Вершины треугольников в буфере вершин (достаем при помощи полученных индексов)
    Vertex[3] vertices = Vertex[3](_vertices[objId].v[ind.x],
                                   _vertices[objId].v[ind.y],
                                   _vertices[objId].v[ind.z]);

    // Интерполированное значение в точке пересечения
    Vertex interpolated = interpolatedVertex(vertices, attribs);

    // Перевести в глобальное пространство из пространства модели
    interpolated.normal = normalize(normalMatrix * interpolated.normal);
    interpolated.position = (modelMatrix * vec4(interpolated.position,1.0f)).xyz;

    // Инициализация случайного числа
    uint rndSeed = tea(gl_LaunchIDEXT.y * gl_LaunchSizeEXT.x + gl_LaunchIDEXT.x, _frameCounter);

    // Вектор от точки в сторону камеры
    vec3 toView = normalize(-gl_WorldRayDirectionEXT);

    // Материал
    Material m = _materials[objId].m;

    // Цвет и бликовость поверхности по умолчанию
    vec3 pointColor = interpolated.color;
    float pointSpec = 1.0f;

    // Итоговый цвет
    vec3 result = vec3(0.0f);

    // Если есть что-то кроме отраженной компоненты
    if(m.reflectance < 1.0f)
    {
        // Пройтись по всем источникам света
        for(uint i = 0; i < _lightCount; i++)
        {
            result += sphericalLight(
                _lightSources[i], 
                m, 
                interpolated.position, 
                interpolated.normal, 
                toView, 
                pointColor, 
                pointSpec, 
                1, 
                rndSeed);
        }
    }

    // Если есть отраженная компонента
    if(m.reflectance > 0.0f)
    {
        // Требуется новый луч
        rayData.done = false;
        // Цвет текущей поверхности отдается частично (остальная часть - отражение)
        rayData.hitColor = result * (1.0f - m.reflectance);
        // Сила нового луча зависит от силы отражения
        rayData.newRayStrength = m.reflectance;
        // Начало и направление нового луча
        rayData.newRayOrigin = interpolated.position;
        rayData.newRayDirection = reflect(gl_WorldRayDirectionEXT, interpolated.normal);
    }
    // Если отражения нет - считать трассировку завершенной, вернуть цвет
    else
    {
        rayData.done = true;
        rayData.hitColor = result;
    }
}
