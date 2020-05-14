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

        //TODO: Здесь может быть загрузка исходных кодов шейдеров

        _vkRenderer = new VkRenderer(_hInstance,_hwnd);

        /** Рендерер - загрузка ресурсов **/

        //TODO: Здесь будет загрузка ресурсов (текстуры, геометрия)

        /** Рендерер - инициализация сцены **/

        //TODO: Построение начальной сцены (объекты, источники света, камера и прочее)

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
                //TODO: здесь может быть рисование сцены
            }
        }

    }
    catch(vk::Error& error){
        std::cout << "vk::error: " << error.what() << std::endl;
        system("pause");
    }
    catch(std::runtime_error& error){
        std::cout << "app::error: " << error.what() << std::endl;
        system("pause");
    }

    // Уничтожение рендерера
    delete _vkRenderer;

    // Уничтожение окна
    DestroyWindow(_hwnd);

    // Вырегистрировать класс окна
    UnregisterClass("AppWindowClass",_hInstance);

    return 0;
}