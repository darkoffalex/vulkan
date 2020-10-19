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
tools::Timer* g_timer = nullptr;
/// Камера
tools::Camera* g_camera = nullptr;
/// Координаты мыши в последнем кадре
POINT g_lastMousePos = {0, 0};

// Макросы для проверки состояния кнопок
#define KEY_DOWN(vk_code) ((static_cast<uint16_t>(GetAsyncKeyState(vk_code)) & 0x8000u) ? true : false)
#define KEY_UP(vk_code) ((static_cast<uint16_t>(GetAsyncKeyState(vk_code)) & 0x8000u) ? false : true

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

        // Загрузка кода шейдеров (для растеризации)
        auto vsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.vert.spv"));
        auto gsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.geom.spv"));
        auto fsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.frag.spv"));

        // Для трассировки лучей
        auto rgCode = tools::LoadBytesFromFile(tools::ShaderDir().append("raytrace.rgen.spv"));
    	auto rmCode = tools::LoadBytesFromFile(tools::ShaderDir().append("raytrace.rmiss.spv"));
    	auto rmsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("raytrace-shadow.rmiss.spv"));
    	auto rhCode = tools::LoadBytesFromFile(tools::ShaderDir().append("raytrace.rchit.spv"));

        // Инициализация рендерера
        g_vkRenderer = new VkRenderer(g_hInstance, g_hwnd,
                                      vsCode, gsCode, fsCode,
                                      rgCode, rmCode, rmsCode, rhCode);

        /** Рендерер - загрузка ресурсов **/

        // Геометрия
        auto quadGeometry = vk::helpers::GenerateQuadGeometry(g_vkRenderer, 1.0f);
        auto cubeGeometry = vk::helpers::GenerateCubeGeometry(g_vkRenderer, 1.0f);
        auto headGeometry = vk::helpers::LoadVulkanGeometryMesh(g_vkRenderer, "head.obj");

        // Текстуры
        auto floorTextureColor = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Floor2/diffuse.png", true);
        auto floorTextureNormal = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Floor2/normal.png", true);
        auto floorTextureSpec = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Floor2/spec.png", true);
        auto floorTextureDisplace = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Floor2/height.png", true);

        auto wallTextureColor = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Wall1/color.jpg", true);
        auto wallTextureNormal = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Wall1/normal.jpg", true);
        auto wallTextureSpec = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Wall1/spec.jpg", true);
        auto wallTextureDisplace = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Wall1/disp.png", true);

        auto cubeTextureColor = vk::helpers::LoadVulkanTexture(g_vkRenderer, "crate.png", true);
        auto cubeTextureSpec = vk::helpers::LoadVulkanTexture(g_vkRenderer, "crate_spec.png", true);

        auto headTextureColor = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Head/diffuse.tga", true);
        auto headTextureNormal = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Head/nm_tangent.tga", true);
        auto headTextureSpec = vk::helpers::LoadVulkanTexture(g_vkRenderer, "Head/spec.tga", true);

        /** Рендерер - инициализация сцены **/

        // Пол
        auto floor = g_vkRenderer->addMeshToScene(quadGeometry,{floorTextureColor,floorTextureNormal,floorTextureSpec});
        floor->setTextureMapping({{0.0f,0.0f},{0.0f,0.0f},{10.0f,10.0f}});
        floor->setPosition({0.0f,0.0f,0.0f}, false);
        floor->setScale({10.0f,10.0f,1.0f}, false);
        floor->setOrientation({-90.0f,0.0f,0.0f});
        floor->setMaterialSettings({{0.05f, 0.05f, 0.05f},{0.9f, 0.9f, 0.9f},{0.6f, 0.6f, 0.6f},16.0f, 0.3f});

        // Кубы
        auto cube0 = g_vkRenderer->addMeshToScene(cubeGeometry, {cubeTextureColor, nullptr, cubeTextureSpec});
        cube0->setTextureMapping({{0.0f,0.0f},{0.0f,0.0f},{1.0f,1.0f},0.0f});
        cube0->setScale({1.0f,1.0f,1.0f}, false);
        cube0->setPosition({0.0f,0.5f,0.0f});
        cube0->setMaterialSettings({{0.05f, 0.05f, 0.05f},{0.9f, 0.9f, 0.9f},{0.6f, 0.6f, 0.6f},16.0f, 1.0f});

        auto cube1 = g_vkRenderer->addMeshToScene(cubeGeometry);
        cube1->setScale({0.5f,0.5f,0.5f}, false);
        cube1->setPosition({2.0f,0.25f,-2.0f});
        cube1->setMaterialSettings({{0.05f, 0.05f, 0.05f},{1.0f, 0.0f, 0.0f},{0.6f, 0.6f, 0.6f},16.0f, 0.5f});

        auto cube2 = g_vkRenderer->addMeshToScene(cubeGeometry);
        cube2->setScale({0.5f,0.5f,0.5f}, false);
        cube2->setPosition({-2.0f,0.25f,-2.0f});
        cube2->setMaterialSettings({{0.05f, 0.05f, 0.05f},{0.0f, 1.0f, 0.0f},{0.6f, 0.6f, 0.6f},16.0f, 0.5f});

        auto cube3 = g_vkRenderer->addMeshToScene(cubeGeometry);
        cube3->setScale({0.5f,0.5f,0.5f}, false);
        cube3->setPosition({2.0f,0.25f,2.0f});
        cube3->setMaterialSettings({{0.05f, 0.05f, 0.05f},{0.0f, 0.0f, 1.0f},{0.6f, 0.6f, 0.6f},16.0f, 0.5f});

        auto cube4 = g_vkRenderer->addMeshToScene(cubeGeometry);
        cube4->setScale({0.5f,0.5f,0.5f}, false);
        cube4->setPosition({-2.0f,0.25f,2.0f});
        cube4->setMaterialSettings({{0.05f, 0.05f, 0.05f},{1.0f, 1.0f, 0.0f},{0.6f, 0.6f, 0.6f},16.0f, 0.5f});

        // Построение TLAS на основе добавленных мешей (для трассировки лучей)
        g_vkRenderer->rtBuildTopLevelAccelerationStructure();
        g_vkRenderer->rtPrepareDescriptorSet();

        // Свет
        auto light1 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {-1.5f, 3.0f, -1.0f});
        light1->setRadius(0.3f);

        auto light2 = g_vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint, {1.5f, 3.0f, 1.0f});
        light2->setRadius(0.3f);

        /** MAIN LOOP **/

        // Управляемая камера
        g_camera = new tools::Camera();
        g_camera->position = {0.0f, 2.0f, 4.0f};
        g_camera->orientation = {-25.0f, 0.0f, 0.0f};

        // Таймер основного цикла (для выяснения временной дельты и FPS)
        g_timer = new tools::Timer();

        // Запуск цикла
        MSG msg = {};
        while (true)
        {
            // Обновить таймер
            g_timer->updateTimer();

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
            if (g_timer->isFpsCounterReady()){
                std::string fps = std::string("Vulkan samples (").append(std::to_string(g_timer->getFps())).append(" FPS)");
                SetWindowTextA(g_hwnd, fps.c_str());
            }

            /// Обновление сцены

            // Камера
            g_camera->translate(g_timer->getDelta());
            g_vkRenderer->getCameraPtr()->setPosition(g_camera->position, false);
            g_vkRenderer->getCameraPtr()->setOrientation(g_camera->orientation);

            /// Отрисовка и показ кадра

//            g_vkRenderer->draw();
            g_vkRenderer->raytrace();
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
