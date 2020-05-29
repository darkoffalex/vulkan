#include "Tools.hpp"
#include "VkRenderer.h"

#include <iostream>

/// Дескриптор исполняемого модуля программы
HINSTANCE _hInstance = nullptr;
/// Дескриптор основного окна
HWND _hwnd = nullptr;
/// Указатель на рендерер
VkRenderer* _vkRenderer = nullptr;

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

        // Загрузить геометрический буфер для треугольника
        auto triangleGeometry = _vkRenderer->createGeometryBuffer({
                {
                        {-1.0f,-1.0f,0.0f},
                        {1.0f,0.0f,0.0f},
                        {0.0f,0.0f},
                        {0.0f,0.0f,1.0f}
                },
                {
                        {0.0f,1.0f,0.0f},
                        {0.0f,1.0f,0.0f},
                        {0.5f,1.0f},
                        {0.0f,0.0f,1.0f}
                },
                {
                        {1.0f,-1.0f,0.0f},
                        {0.0f,0.0f,1.0f},
                        {1.0f,1.0f},
                        {0.0f,0.0f,1.0f}
                },
            },{0,1,2});

        /** Рендерер - инициализация сцены **/

        auto triangleMesh = _vkRenderer->addMeshToScene(triangleGeometry);

        /** MAIN LOOP **/

        // Запуск цикла
        MSG msg = {};
        while (true)
        {
            // Обработка оконных сообщений
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT) {
                    break;
                }

                //TODO: здесь может быть обновление сцены (изменение положения объектов и прочее)

                // Рисование и показ сцены
                _vkRenderer->draw();
            }
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
 * Перед закрытием окна
 */
void tools::BeforeWindowClose()
{
    if(_vkRenderer != nullptr){
        _vkRenderer->setRenderingStatus(false);
    }
}

/**
 * После изменения размера окна
 */
void tools::OnWindowResized()
{
    if(_vkRenderer != nullptr){
        _vkRenderer->onSurfaceChanged();
    }
}
