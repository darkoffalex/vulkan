#version 450
#extension GL_ARB_separate_shader_objects : enable

// Набор констант
#define LIGHT_TYPE_POINT 0        // Тип источника - точеченый
#define LIGHT_TYPE_SPOT 1         // Тип источника - прожектор
#define LIGHT_TYPE_DIRECTIONAL 2  // Тип источника - направленный
#define MAX_LIGHTS 100            // Максимальное число источников
#define PI 3.14159265359

/*Схема входа-выхода*/

layout (location = 0) out vec4 outColor;

layout (location = 0) in GS_OUT
{
    vec3 color;            // Цвет фрагмента
    vec3 position;         // Положение фрагмента в мировых координатах
    vec3 positionLocal;    // Положение фрагмента в локальных координатах
    vec3 normal;           // Вектор нормали фрагмента
    vec2 uv;               // Текстурные координаты фрагмента
    mat3 tbnMatrix;        // Матрица для преобразования из касательного пространства треугольника в мировое
    mat3 tbnMatrixInverse; // Матрица для преобразования из мирового пространства в касательное
} fs_in;

/*Вспомогательные типы*/

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

// Параметры текущего фрагмента (Blinn-Phong)
struct Fragment
{
    vec3 position;      // Положение
    vec3 normal;        // Нормаль
    vec3 toView;        // Вектор в сторону камеры
    vec3 albedo;        // Собственный цвет фрагмента
    float roughness;    // Шероховатость фрагмента
    float metallic;     // Металличность фрагмента
};

// Параметры матриала (Blinn-Phong)
struct Material
{
    vec3 albedo;
    float roughness;
    float metallic;
};

/*Uniform*/

layout(set = 0, binding = 0) uniform UniformCamera {
    mat4 _view;
    mat4 _proj;
    vec3 _camPosition;
};

layout(set = 2, binding = 2, std140) uniform UniformModel {
    Material _materialSettings;
};

layout(set = 1, binding = 0, std140) uniform UniformLightCount {
    uint _lightCount;
};

layout(set = 1, binding = 1, std140) uniform UniformLightArray {
    LightSource _lightSources[MAX_LIGHTS];
};

/*Функции*/

// Затухание для точечного источника
float pointLightAttenuation(Fragment f, LightSource l)
{
    // Коэффициент затухания (использует расстояние до источника, а так же спец-коэффициенты)
    float dist = length(l.position - f.position);
    return 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));
}

// Затухание для источника типа "прожектор"
float spotLightAttenuation(Fragment f, LightSource l, vec3 fragmentToLight)
{
    // Коэффициент затухания (использует расстояние до источника, а так же спец-коэффициенты)
    float dist = length(l.position - f.position);
    float attenuation = 1.0f / (1.0f + l.attenuationLinear * dist + l.attenuationQuadratic * (dist * dist));

    // Косинус угла между вектором падения света и вектором направления источника
    float thetaCos = dot(fragmentToLight, normalize(-l.orientation));

    // Разница косинусов между углом внутреннего конуса и углом внешнего
    float epsilon = l.cutOffAngleCos - l.cutOffOuterAngleCos;

    // Свет наиболее интенсивен в центре (где thetaCos - 1, угол между лучем и направлением фонарика - 0)
    // К краям интенсивность спадает. Благодаря коэффициенту epsilon есть так же яркое пятно внутри (внутр. конус)
    float intensity = clamp((thetaCos - l.cutOffOuterAngleCos) / epsilon, 0.0, 1.0);
    return attenuation * intensity;
}

// Распределение микрограней для бликовой составляющей
float distributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// Распределения микрограней для самоздатенения и перекрытия
float geometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

// Коэффициент Френеля
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Основная функция фрагментного шейдера
// Вычисление итогового цвета фрагмента (пикселя)
void main()
{
    // Структура описывающая текущий фрагмент
    Fragment f;
    f.toView = normalize(_camPosition - fs_in.position);
    f.position = fs_in.position;
    f.normal = normalize(fs_in.normal);
    f.albedo = _materialSettings.albedo;
    f.roughness = _materialSettings.roughness;
    f.metallic = _materialSettings.metallic;

    // Коэффициент F0 для Френеля
    // Чем металичнее материал тем более коэффициент уходит в альбедо
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, f.albedo, f.metallic);

    // Сумарная освещенность точки (фрагмента)
    vec3 Lo = vec3(0.0f);

    // Пройтись по всем источникам света
    for(uint i = 0; i < _lightCount; i++)
    {
        // Вектор в сторону источника
        vec3 toLight = normalize(_lightSources[i].position - f.position);

        // Половинный вектор (между векторами к источнику и камере)
        vec3 h = normalize(toLight + f.toView);

        // Затухание источника
        float attenuation = 1.0f;
        switch(_lightSources[i].type)
        {
            case LIGHT_TYPE_POINT:
                attenuation = pointLightAttenuation(f,_lightSources[i]);
                break;
            case LIGHT_TYPE_SPOT:
                attenuation = spotLightAttenuation(f,_lightSources[i],toLight);
                break;
            case LIGHT_TYPE_DIRECTIONAL:
                attenuation = 1.0f;
                break;
        }
        
        // Облученность (интенсивность освещенности) точки (фрагмента) конкретным источником
        vec3 radiance = _lightSources[i].color * attenuation;

        // Подсчет коэффициентов для BRDF Кука-Торенса
        float G = geometrySchlickGGX(max(dot(f.toView,f.normal),0.0f), f.roughness) * geometrySchlickGGX(max(dot(toLight,f.normal),0.0f), f.roughness);
        float D = distributionGGX(f.normal, h, f.roughness);
        vec3 F = fresnelSchlick(max(dot(h, f.toView), 0.0), F0);

        // Подсчер BRDF (отраженный свет)
        vec3 spec = G * D * F / 4.0f * max(dot(f.toView,f.normal),0.0f) * max(dot(toLight,f.normal),0.0f) + 0.001f;

        // Коэфициенты specular состовляющей и diffuse состоавляющей (одно исключает другое)
        // Считаем что коэффициент Френеля и есть коэффициент отраженного света (для металлов он максимально уходит в albedo)
        vec3 kS = F;
        vec3 kD = vec3(1.0f) - F;

        // Для металического материала коэффициент kD равен нулю (весь свет отражается)
        kD *= (1.0f - f.metallic);

        // Вклад источника в освещение точки (фрагмента)
        Lo += (kD * f.albedo / PI + spec) * radiance * max(dot(f.normal,toLight),0.0f);
    }

    // Вывод цвета во вложение
    outColor = vec4(Lo,1.0f);
}
