# Версия CMake
cmake_minimum_required(VERSION 3.15)

# Название приложения
set(TARGET_NAME "SampleApp")
set(TARGET_BIN_NAME "SampleApp")

# Добавляем .exe (проект в Visual Studio)
add_executable(${TARGET_NAME}
        "Main.cpp"
        "Tools/Tools.hpp" "Tools/Timer.hpp" "Tools/Camera.hpp"
        "VkRenderer.h" "VkRenderer.cpp"
        "VkHelpers.h" "VkHelpers.cpp"
        "VkExtensionLoader/ExtensionLoader.h" "VkExtensionLoader/ExtensionLoader.c"
        "VkTools/Tools.h" "VkTools/Tools.cpp" "VkTools/Device.hpp" "VkTools/Buffer.hpp" "VkTools/Image.hpp"
        "VkResources/FrameBuffer.hpp" "VkResources/GeometryBuffer.hpp" "VkResources/TextureBuffer.hpp"
        "VkScene/SceneElement.h" "VkScene/SceneElement.cpp" "VkScene/Mesh.h" "VkScene/Mesh.cpp" "VkScene/Camera.h" "VkScene/Camera.cpp" "VkScene/LightSource.h" "VkScene/LightSource.cpp" "VkScene/LightSourceSet.hpp")

# Директории с библиотеками (.lib)
if(${PLATFORM_BIT_SUFFIX} STREQUAL "x86")
    target_link_directories(${TARGET_NAME} PUBLIC "${VULKAN_PATH}Lib32")
else()
    target_link_directories(${TARGET_NAME} PUBLIC "${VULKAN_PATH}Lib")
endif()

# Директории с включаемыми файлами (.h)
target_include_directories(${TARGET_NAME} PUBLIC "${VULKAN_PATH}Include" "../../Include")

# Меняем название запускаемого файла в зависимости от типа сборки
set_property(TARGET ${TARGET_NAME} PROPERTY OUTPUT_NAME "${TARGET_BIN_NAME}$<$<CONFIG:Debug>:_Debug>_${PLATFORM_BIT_SUFFIX}")

# Статическая линковка рантайма и стандартных библиотек
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_property(TARGET ${TARGET_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_property(TARGET ${TARGET_NAME} PROPERTY LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
endif()

# Линковка Vulkan
target_link_libraries(${TARGET_NAME} PUBLIC vulkan-1.lib)
# Линковка с shlwapi.lib для получения путей к файлам (tools)
target_link_libraries(${TARGET_NAME} PUBLIC "shlwapi.lib")
