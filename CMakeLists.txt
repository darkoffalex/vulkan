# Версия CMake
cmake_minimum_required(VERSION 3.15)

# Название проекта (решение в Visual Studio)
project(VkRenderer)

# Стандарт С/С++
set(CMAKE_CXX_STANDARD 14)

# Устанавливаем каталоги для бинарников
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

# Путь к Vulkan
set(VULKAN_PATH "C:\\VulkanSDK\\1.2.154.1\\")

# Определить разрядность платформы
if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(PLATFORM_BIT_SUFFIX "x86")
else()
    set(PLATFORM_BIT_SUFFIX "x64")
endif()

# Стандартные библиотеки для GNU/MinGW
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
endif()

# Демо-приложение с примером использования
add_subdirectory("Sources/SampleApp")

