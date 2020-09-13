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
        auto gsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.geom.spv"));
        auto fsCode = tools::LoadBytesFromFile(tools::ShaderDir().append("base-phong.frag.spv"));

        // Инициализация рендерера
        _vkRenderer = new VkRenderer(_hInstance,_hwnd,vsCode,gsCode,fsCode);

        /** Рендерер - загрузка ресурсов **/

        // Геометрия
        auto quadGeometry = vk::helpers::GenerateQuadGeometry(_vkRenderer,1.0f);
        auto cubeGeometry = vk::helpers::GenerateCubeGeometry(_vkRenderer,1.0f);
        auto headGeometry = vk::helpers::LoadVulkanGeometryMesh(_vkRenderer, "head.obj");
//        auto ar2rGeometry = vk::helpers::LoadVulkanGeometryMesh(_vkRenderer,"Ar2r-Devil-Pinky.dae", true);

        // Скелет и анимации
//        auto skeleton = vk::helpers::LoadVulkanMeshSkeleton("Ar2r-Devil-Pinky.dae");
//        auto animations = vk::helpers::LoadVulkanMeshSkeletonAnimations("Ar2r-Devil-Pinky.dae");

        // Текстуры
        auto floorTextureColor = vk::helpers::LoadVulkanTexture(_vkRenderer,"Floor2/diffuse.png",true);
        auto floorTextureNormal = vk::helpers::LoadVulkanTexture(_vkRenderer,"Floor2/normal.png",true);
        auto floorTextureSpec = vk::helpers::LoadVulkanTexture(_vkRenderer,"Floor2/spec.png",true);
        auto floorTextureDisplace = vk::helpers::LoadVulkanTexture(_vkRenderer,"Floor2/height.png",true);

        auto wallTextureColor = vk::helpers::LoadVulkanTexture(_vkRenderer,"Wall1/color.jpg", true);
        auto wallTextureNormal = vk::helpers::LoadVulkanTexture(_vkRenderer,"Wall1/normal.jpg", true);
        auto wallTextureSpec = vk::helpers::LoadVulkanTexture(_vkRenderer,"Wall1/spec.jpg", true);
        auto wallTextureDisplace = vk::helpers::LoadVulkanTexture(_vkRenderer,"Wall1/disp.png", true);

        auto cubeTextureColor = vk::helpers::LoadVulkanTexture(_vkRenderer,"crate.png", true);
        auto cubeTextureSpec = vk::helpers::LoadVulkanTexture(_vkRenderer,"crate_spec.png", true);

        auto headTextureColor = vk::helpers::LoadVulkanTexture(_vkRenderer,"Head/diffuse.tga", true);
        auto headTextureNormal = vk::helpers::LoadVulkanTexture(_vkRenderer,"Head/nm_tangent.tga", true);
        auto headTextureSpec = vk::helpers::LoadVulkanTexture(_vkRenderer,"Head/spec.tga", true);

        /** Рендерер - инициализация сцены **/

        // Пол
        auto floor = _vkRenderer->addMeshToScene(quadGeometry,{floorTextureColor,floorTextureNormal,floorTextureSpec});
        floor->setTextureMapping({{0.0f,0.0f},{0.0f,0.0f},{10.0f,10.0f}});
        floor->setPosition({0.0f,0.0f,0.0f}, false);
        floor->setScale({10.0f,10.0f,1.0f}, false);
        floor->setOrientation({-90.0f,0.0f,0.0f});

        // Стена
        auto wall = _vkRenderer->addMeshToScene(quadGeometry,{wallTextureColor,wallTextureNormal,wallTextureSpec});
        wall->setTextureMapping({{0.0f,0.0f},{0.0f,0.0f},{10.0f,2.5f},0.0f});
        wall->setPosition({0.0f,1.25f,-1.5f}, false);
        wall->setScale({10.0f,2.5f,1.0f});

        // Куб
        auto cube = _vkRenderer->addMeshToScene(cubeGeometry, {cubeTextureColor, nullptr,cubeTextureSpec});
        cube->setTextureMapping({{0.0f,0.0f},{0.0f,0.0f},{1.0f,1.0f},0.0f});
        cube->setScale({0.5f,0.5f,0.5f}, false);
        cube->setOrientation({0.0f,45.0f,0.0f}, false);
        cube->setPosition({0.0f,0.25f,0.0f});

        // Голова
        auto head = _vkRenderer->addMeshToScene(headGeometry, {headTextureColor, headTextureNormal, headTextureSpec});
        head->setScale({0.35f,0.35f,0.35f}, false);
        head->setOrientation({0.0f,0.0f,0.0f}, false);
        head->setPosition({0.0f,0.75f,0.15f});

        // Построение TLAS на основе добавленных мешей (для трассировки лучей)
        _vkRenderer->rtBuildTopLevelAccelerationStructure();
        _vkRenderer->rtPrepareDescriptorSet();

        // Ar2r-Devil-Pinky (первая версия)
//        auto Ar2r = _vkRenderer->addMeshToScene(ar2rGeometry);
//        Ar2r->setPosition({0.0f, 0.0f, 0.0f}, false);
//        Ar2r->setScale({2.0f, 2.0f, 2.0f});
//        Ar2r->setSkeleton(std::move(skeleton));
//        Ar2r->getSkeletonPtr()->setCurrentAnimation(animations[0]);
//        Ar2r->getSkeletonPtr()->setAnimationState(vk::scene::MeshSkeleton::AnimationState::ePlaying);
//        Ar2r->getSkeletonPtr()->applyAnimationFrameBoneTransforms(0.0f);

        // Свет
        auto light1 = _vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint,{2.5f,1.5f,0.0f});
        auto light2 = _vkRenderer->addLightToScene(vk::scene::LightSourceType::ePoint,{-2.5f,1.5f,0.0f});

        /** MAIN LOOP **/

        // Управляемая камера
        _camera = new tools::Camera();
        _camera->position = {0.0f,2.0f,4.0f};
        _camera->orientation = {-30.0f,0.0f,0.0f};

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

            // Камера
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
