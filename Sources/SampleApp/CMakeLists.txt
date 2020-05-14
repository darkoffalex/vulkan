# Версия CMake
cmake_minimum_required(VERSION 3.15)

# Название приложения
set(TARGET_NAME "SampleApp")
set(TARGET_BIN_NAME "SampleApp")

# Добавляем .exe (проект в Visual Studio)
add_executable(${TARGET_NAME}
        "Main.cpp" "Tools.hpp"
        "ext_loader/vulkan_ext.h" "ext_loader/vulkan_ext.c"
        "VkTools.h" "VkTools.cpp" "VkToolsDevice.hpp" "VkToolsImage.hpp" "VkToolsFrameBuffer.hpp"
        "VkRenderer.h" "VkRenderer.cpp")

# Директории с библиотеками (.lib)
if(${PLATFORM_BIT_SUFFIX} STREQUAL "x86")
    target_link_directories(${TARGET_NAME} PUBLIC "${VULKAN_PATH}Lib32")
else()
    target_link_directories(${TARGET_NAME} PUBLIC "${VULKAN_PATH}Lib")
endif()

# Директории с включаемыми файлами (.h)
target_include_directories(${TARGET_NAME} PUBLIC ${VULKAN_PATH}Include)

# Меняем название запускаемого файла в зависимости от типа сборки
set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME "${TARGET_BIN_NAME}$<$<CONFIG:Debug>:_Debug>_${PLATFORM_BIT_SUFFIX}")

# Статическая линковка рантайма и стандартных библиотек
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_property(TARGET ${TARGET_NAME} PROPERTY LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

# Линковка Vulkan
target_link_libraries(${TARGET_NAME} vulkan-1.lib)