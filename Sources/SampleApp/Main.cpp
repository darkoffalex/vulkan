#include <iostream>

#include "VkRenderer.h"
#include "VkHelpers.h"
#include "Tools/Tools.hpp"

/// Дескриптор исполняемого модуля программы
HINSTANCE g_hInstance = nullptr;
/// Дескриптор основного окна
HWND g_hwnd = nullptr;
/// Указатель на рендерер
VkRenderer* g_vkRenderer = nullptr;
/// Таймер
tools::Timer* g_pTimer = nullptr;
/// Камера
tools::Camera* g_camera = nullptr;
/// Координаты мыши в последнем кадре
POINT g_lastMousePos = {0, 0};

// Макросы для проверки состояния кнопок
#define KEY_DOWN(vk_code) ((static_cast<uint16_t>(GetAsyncKeyState(vk_code)) & 0x8000u) ? true : false)
#define KEY_UP(vk_code) ((static_cast<uint16_t>(GetAsyncKeyState(vk_code)) & 0x8000u) ? false : true)

/**
 * Получение координат курсора
 * @param hwnd Дескриптор окна
 * @return Точка с координатами
 */
inline POINT CursorPos(HWND hwnd){
    POINT p;
    if (GetCursorPos(&p)) ScreenToClient(hwnd, &p);
    return p;
}

/**
 * Управление камерой
 * @param camSpeed Скорость камеры
 * @param mouseSensitivity Чувствительность мыши
 */
void Controls(float camSpeed = 0.001f, float mouseSensitivity = 0.2f);

/**
 * Точка входа
 * @param argc Кол-во аргументов
 * @param argv Аргументы
 * @return Код выполнения (выхода)
 */
int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        // Получение дескриптора исполняемого модуля программы
        g_hInstance = GetModuleHandle(nullptr);

        // Пытаемся зарегистрировать оконный класс
        if (!tools::RegisterWindowClass(g_hInstance, "AppWindowClass")) {
            throw std::runtime_error("Can't register window class.");
        }

        // Создание окна
        g_hwnd = CreateWindow(
                "AppWindowClass",
                "Vulkan samples",
                WS_OVERLAPPEDWINDOW,
                0, 0,
                800, 600,
                nullptr,
                nullptr,
                g_hInstance,
                nullptr);

        // Если не удалось создать окно
        if (!g_hwnd) {
            throw std::runtime_error("Can't create main application window.");
        }

        // Показать окно
        ShowWindow(g_hwnd, SW_SHOWNORMAL);

        // Размеры клиентской области окна
        RECT clientRect;
        GetClientRect(g_hwnd, &clientRect);

        /** Рендерер - инициализация **/

        // Загрузка кода шейдеров
        auto vsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base.vert.spv"));
        auto gsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base.geom.spv"));
        auto fsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-pbr.frag.spv"));
        auto vsCodePp = tools::LoadBytesFromFile(tools::ShaderDir().append("post-process.vert.spv"));
        auto fsCodePp = tools::LoadBytesFromFile(tools::ShaderDir().append("post-process.frag.spv"));

        // Инициализация рендерера
        g_vkRenderer = new VkRenderer(g_hInstance, g_hwnd, vsCode, gsCode, fsCode, vsCodePp, fsCodePp);

        /** Рендерер - загрузка ресурсов **/

        // Геометрия
        auto sphereGeometry = vk::helpers::GenerateSphereGeometry(g_vkRenderer,32,1.0f);

        // Текстуры
        auto albedoTex      = vk::helpers::LoadVulkanTexture(g_vkRenderer,"rusted_iron/albedo.png",true,true);
        auto roughnessTex   = vk::helpers::LoadVulkanTexture(g_vkRenderer,"rusted_iron/roughness.png",true);
        auto metallicTex    = vk::helpers::LoadVulkanTexture(g_vkRenderer,"rusted_iron/metallic.png",true);
        auto normalTex      = vk::helpers::LoadVulkanTexture(g_vkRenderer,"rusted_iron/normal.png",true);


        /** Рендерер - инициализация сцены **/

        // Сферы
        glm::float32_t spacing = 2.5f;
        glm::uint32_t colNum = 2;
        glm::uint32_t rowNum = 2;

        for(uint32_t row = 0; row < rowNum; row++)
        {
            for(uint32_t col = 0; col < colNum; col++)
            {
//                glm::vec3 albedo = {0.5f,0.0f,0.0f};
//                glm::float32_t roughness = glm::clamp(static_cast<float>(col)/static_cast<float>(colNum),0.05f,1.0f);
//                glm::float32_t metallic = static_cast<float>(row)/static_cast<float>(rowNum);

                auto sphere = g_vkRenderer->addMeshToScene(sphereGeometry,{albedoTex, roughnessTex, metallicTex, normalTex});
                sphere->setPosition({
                    (col * spacing) - ((static_cast<float>(colNum - 1) * spacing) / 2.0f),
                    (row * spacing) - ((static_cast<float>(rowNum - 1) * spacing) / 2.0f),
                    0.0f});
//                sphere->setMaterialSettings({
//                    albedo,
//                    roughness,
//                    metallic
//                });
            }
        }

        // Свет
        auto light0 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {0.0f, 0.0f, 10.0f},{150.0f,150.0f,150.0f});
//        auto light0 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {-10.0f, 10.0f, 10.0f},{300.0f,300.0f,300.0f});
//        auto light1 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {10.0f, 10.0f, 10.0f},{300.0f,300.0f,300.0f});
//        auto light2 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {-10.0f, -10.0f, 10.0f},{300.0f,300.0f,300.0f});
//        auto light3 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {10.0f, -10.0f, 10.0f},{300.0f,300.0f,300.0f});

        /** MAIN LOOP **/

        // Управляемая камера
        g_camera = new tools::Camera();
        g_camera->position = {0.0f, 0.0f, 10.0f};
        //g_camera->orientation = {-30.0f, 0.0f, 0.0f};

        // Таймер основного цикла (для выяснения временной дельты и FPS)
        g_pTimer = new tools::Timer();

        // Запуск цикла
        MSG msg = {};
        while (true)
        {
            // Обновить таймер
            g_pTimer->updateTimer();

            // Обработка клавиш управления
            Controls();

            // Обработка оконных сообщений
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
                if (msg.message == WM_QUIT) {
                    break;
                }
            }

            // Поскольку показ FPS на окне уменьшает FPS - делаем это только тогда когда счетчик готов (примерно 1 раз в секунду)
            if (g_pTimer->isFpsCounterReady()){
                std::string fps = std::string("Vulkan samples (").append(std::to_string(g_pTimer->getFps())).append(" FPS)");
                SetWindowTextA(g_hwnd, fps.c_str());
            }

            /// Обновление сцены

            // Камера
            g_camera->translate(g_pTimer->getDelta());
            g_vkRenderer->getCameraPtr()->setPosition(g_camera->position, false);
            g_vkRenderer->getCameraPtr()->setOrientation(g_camera->orientation);

            /// Отрисовка и показ кадра

            g_vkRenderer->draw();
        }

        // Уничтожение рендерера
        delete g_vkRenderer;
    }
    catch(vk::Error& error){
        std::cout << "vk::error: " << error.what() << std::endl;
        system("pause");
    }
    catch(std::runtime_error& error){
        std::cout << "app::error: " << error.what() << std::endl;
        system("pause");
    }

    // Уничтожение окна
    DestroyWindow(g_hwnd);

    // Вырегистрировать класс окна
    UnregisterClass("AppWindowClass", g_hInstance);

    return 0;
}

/**
 * Управление камерой
 * @param camSpeed Скорость камеры
 * @param mouseSensitivity Чувствительность мыши
 */
void Controls(float camSpeed, float mouseSensitivity)
{
    // Вектор движения камеры
    glm::vec3 camMovementRel = {0.0f, 0.0f, 0.0f};
    glm::vec3 camMovementAbs = {0.0f, 0.0f, 0.0f};
    glm::vec3 camRotation = {0.0f,0.0f,0.0f};

    auto t = GetAsyncKeyState(0x57u);
    if(static_cast<uint16_t>(t) & 0x8000u){}

    // Состояние клавиш клавиатуры
    if(KEY_DOWN(0x57u)) camMovementRel.z = -1.0f; // W
    if(KEY_DOWN(0x41u)) camMovementRel.x = -1.0f; // A
    if(KEY_DOWN(0x53u)) camMovementRel.z = 1.0f;  // S
    if(KEY_DOWN(0x44u)) camMovementRel.x = 1.0f;  // D
    if(KEY_DOWN(VK_SPACE)) camMovementAbs.y = 1.0f;
    if(KEY_DOWN(0x43u)) camMovementAbs.y = -1.0f; // С

    // Мышь
    auto currentMousePos = CursorPos(g_hwnd);
    if(KEY_DOWN(VK_LBUTTON)){
        POINT deltaMousePos = {g_lastMousePos.x - currentMousePos.x, g_lastMousePos.y - currentMousePos.y};
        g_camera->orientation.x += static_cast<float>(deltaMousePos.y) * mouseSensitivity;
        g_camera->orientation.y += static_cast<float>(deltaMousePos.x) * mouseSensitivity;
    }
    g_lastMousePos = currentMousePos;

    // Установить векторы движения камеры
    if(g_camera != nullptr){
        g_camera->setTranslation(camMovementRel * camSpeed);
        g_camera->setTranslationAbsolute(camMovementAbs * camSpeed);
    }
}

/**
 * Обработчик оконных сообщений (должна быть реализована в .cpp)
 * @param hWnd Дескриптор окна
 * @param message Сообщение
 * @param wParam Параметр сообщения
 * @param lParam Параметр сообщения
 * @return Код выполнения
 */
LRESULT CALLBACK tools::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Положение мыши
    static glm::ivec2 mousePositions;

    // Обработка оконных сообщений
    switch (message)
    {
        // Уничтожение окна
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

            // Закрытие окна
        case WM_CLOSE:
            if(g_vkRenderer != nullptr){
                g_vkRenderer->setRenderingStatus(false);
            }
            return DefWindowProc(hWnd, message, wParam, lParam);

            // Завершение изменения размера окна
        case WM_EXITSIZEMOVE:
            if(g_vkRenderer != nullptr){
                g_vkRenderer->onSurfaceChanged();
            }
            return DefWindowProc(hWnd, message, wParam, lParam);

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
