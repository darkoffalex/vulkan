#include <iostream>

#include "VkRenderer.h"
#include "VkHelpers.h"
#include "Tools/Tools.hpp"

/// Дескриптор исполняемого модуля программы
HINSTANCE _hInstance = nullptr;
/// Дескриптор основного окна
HWND _hwnd = nullptr;
/// Указатель на рендерер
VkRenderer* _vkRenderer = nullptr;
/// Таймер
tools::Timer* _timer = nullptr;
/// Камера
tools::Camera* _camera = nullptr;
/// Координаты мыши в последнем кадре
POINT _lastMousePos = {0,0};

// Макросы для проверки состояния кнопок
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

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
        _hInstance = GetModuleHandle(nullptr);

        // Пытаемся зарегистрировать оконный класс
        if (!tools::RegisterWindowClass(_hInstance,"AppWindowClass")) {
            throw std::runtime_error("Can't register window class.");
        }

        // Создание окна
        _hwnd = CreateWindow(
                "AppWindowClass",
                "Vulkan samples",
                WS_OVERLAPPEDWINDOW,
                0, 0,
                800, 600,
                nullptr,
                nullptr,
                _hInstance,
                nullptr);

        // Если не удалось создать окно
        if (!_hwnd) {
            throw std::runtime_error("Can't create main application window.");
        }

        // Показать окно
        ShowWindow(_hwnd, SW_SHOWNORMAL);

        // Размеры клиентской области окна
        RECT clientRect;
        GetClientRect(_hwnd, &clientRect);

        /** Рендерер - инициализация **/

        // Загрузка кода шейдеров
        auto vsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.vert.spv"));
        auto fsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.frag.spv"));

        // Инициализация рендерера
        _vkRenderer = new VkRenderer(_hInstance,_hwnd,vsCode,fsCode);

        /** Рендерер - загрузка ресурсов **/

        // Геометрия
        auto quadGeometry = vk::helpers::GenerateQuadGeometry(_vkRenderer,1.0f);
        auto cubeGeometry = vk::helpers::GenerateCubeGeometry(_vkRenderer,1.0f);

        // Текстуры
        auto floorTexture = vk::helpers::LoadVulkanTexture(_vkRenderer,"Floor2/diffuse.png",true);
        auto cubeTexture = vk::helpers::LoadVulkanTexture(_vkRenderer,"crate.png", true);

        /** Рендерер - инициализация сцены **/

        // Пол
        auto floor = _vkRenderer->addMeshToScene(quadGeometry, floorTexture);
        floor->setTextureMapping({{0.0f,0.0f},{0.0f,0.0f},{10.0f,10.0f}});
        floor->setPosition({0.0f,0.0f,0.0f}, false);
        floor->setScale({10.0f,10.0f,0.0f}, false);
        floor->setOrientation({-90.0f,0.0f,0.0f});

        // Куб
        auto cube = _vkRenderer->addMeshToScene(cubeGeometry,cubeTexture);
        cube->setScale({0.5f,0.5f,0.5f});
        cube->setPosition({0.0f,0.25f,0.0f});

        /** MAIN LOOP **/

        // Управляемая камера
        _camera = new tools::Camera();
        _camera->position = {0.0f,2.0f,0.0f};
        _camera->orientation = {0.0f,0.0f,0.0f};

        // Таймер основного цикла (для выяснения временной дельты и FPS)
        _timer = new tools::Timer();

        // Запуск цикла
        MSG msg = {};
        while (true)
        {
            // Обновить таймер
            _timer->updateTimer();

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
            if (_timer->isFpsCounterReady()){
                std::string fps = std::string("Vulkan samples (").append(std::to_string(_timer->getFps())).append(" FPS)");
                SetWindowTextA(_hwnd, fps.c_str());
            }

            /// Обновление сцены

            _camera->translate(_timer->getDelta());
            _vkRenderer->getCameraPtr()->setPosition(_camera->position, false);
            _vkRenderer->getCameraPtr()->setOrientation(_camera->orientation);

            /// Отрисовка и показ кадра

            _vkRenderer->draw();
        }

        // Уничтожение рендерера
        delete _vkRenderer;
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
    DestroyWindow(_hwnd);

    // Вырегистрировать класс окна
    UnregisterClass("AppWindowClass",_hInstance);

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
    if(KEY_DOWN(0x57)) camMovementRel.z = -1.0f; // W
    if(KEY_DOWN(0x41)) camMovementRel.x = -1.0f; // A
    if(KEY_DOWN(0x53)) camMovementRel.z = 1.0f;  // S
    if(KEY_DOWN(0x44)) camMovementRel.x = 1.0f;  // D
    if(KEY_DOWN(VK_SPACE)) camMovementAbs.y = 1.0f;
    if(KEY_DOWN(0x43)) camMovementAbs.y = -1.0f; // С

    // Мышь
    auto currentMousePos = CursorPos(_hwnd);
    if(KEY_DOWN(VK_LBUTTON)){
        POINT deltaMousePos = {_lastMousePos.x - currentMousePos.x, _lastMousePos.y - currentMousePos.y};
        _camera->orientation.x += static_cast<float>(deltaMousePos.y) * mouseSensitivity;
        _camera->orientation.y += static_cast<float>(deltaMousePos.x) * mouseSensitivity;
    }
    _lastMousePos = currentMousePos;

    // Установить векторы движения камеры
    if(_camera != nullptr){
        _camera->setTranslation(camMovementRel * camSpeed);
        _camera->setTranslationAbsolute(camMovementAbs * camSpeed);
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
            if(_vkRenderer != nullptr){
                _vkRenderer->setRenderingStatus(false);
            }
            return DefWindowProc(hWnd, message, wParam, lParam);

            // Завершение изменения размера окна
        case WM_EXITSIZEMOVE:
            if(_vkRenderer != nullptr){
                _vkRenderer->onSurfaceChanged();
            }
            return DefWindowProc(hWnd, message, wParam, lParam);

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
