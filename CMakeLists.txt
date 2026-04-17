# Initialise cmake script
cmake_minimum_required(VERSION 3.12)
project(cits3003_project)

set(CMAKE_CXX_STANDARD 17)

# GLAD
add_subdirectory(lib/glad)
# end GLAD

# GLFW
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)
# end GLFW

# glm
add_subdirectory(lib/glm)
# end glm

# assimp
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(lib/assimp)
# end assimp

# stb
add_subdirectory(lib/stb)
# end stb

# ImGui
add_subdirectory(lib/imgui)
# end ImGui

# JSON
set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(lib/json)
# end JSON

# tinyfiledialogs
add_subdirectory(lib/tinyfiledialogs)
# end tinyfiledialogs

# Function to configure dependencies and post-build steps for a project executable
function(configure_lab_target TARGET_NAME)
    target_include_directories(${TARGET_NAME} PRIVATE src)

    target_link_libraries(${TARGET_NAME}
            glfw
            glad
            glm
            assimp
            stb
            imgui
            nlohmann_json::nlohmann_json
            tinyfiledialogs
    )

    if (APPLE)
        target_link_libraries(${TARGET_NAME} "-framework Cocoa" "-framework IOKit")
    endif()

    # Copy executable post build
    add_custom_command(TARGET ${TARGET_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${TARGET_NAME}> ${CMAKE_SOURCE_DIR}
    )

    if (WIN32)
        # Due to symbolic links requiring higher privileges on Windows, manually create Junctions instead
        file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/res" _dstResDir)
        file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/res" _srcResDir)

        file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/config" _dstConfigDir)
        file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/config" _srcConfigDir)

        # Link the res and config dirs into where the build puts the exe
        add_custom_command(TARGET ${TARGET_NAME}
                POST_BUILD
                COMMAND cmd.exe /c if not exist "${_dstResDir}" mklink /J "${_dstResDir}" "${_srcResDir}"
                COMMAND cmd.exe /c if not exist "${_dstConfigDir}" mklink /J "${_dstConfigDir}" "${_srcConfigDir}"
        )
    else()
        # Link the res and config dirs into where the build puts the exe
        add_custom_command(TARGET ${TARGET_NAME}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res
                COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}/config
        )
    endif()
endfunction()

# MAIN PROJECT ========================================================================================
add_executable(cits3003_project
        src/main.cpp
        src/rendering/resources/MeshHierarchy.cpp
        src/rendering/resources/TextureLoader.cpp
        src/rendering/resources/TextureHandle.cpp
        src/rendering/resources/ModelLoader.cpp
        src/rendering/scene/MasterRenderScene.cpp
        src/rendering/scene/Animator.cpp
        src/rendering/scene/Lights.cpp
        src/rendering/renders/MasterRenderer.cpp
        src/rendering/renders/shaders/ShaderInterface.cpp
        src/rendering/renders/shaders/BaseEntityShader.cpp
        src/rendering/renders/shaders/BaseLitEntityShader.cpp
        src/rendering/renders/EntityRenderer.cpp
        src/rendering/renders/AnimatedEntityRenderer.cpp
        src/rendering/renders/EmissiveEntityRenderer.cpp
        src/rendering/cameras/PanningCamera.cpp
        src/rendering/cameras/FlyingCamera.cpp
        src/rendering/imgui/ImGuiImpl.cpp
        src/rendering/imgui/ImGuiManager.cpp
        src/system_interfaces/Window.cpp
        src/system_interfaces/WindowManager.cpp
        src/utility/PerformanceCounter.cpp
        src/utility/OpenGL.cpp
        src/utility/SyncManager.cpp
        src/scene/BasicStaticScene.cpp
        src/scene/EditorScene.cpp
        src/scene/SceneManager.cpp
        src/scene/editor_scene/EntityElement.cpp
        src/scene/editor_scene/AnimatedEntityElement.cpp
        src/scene/editor_scene/PointLightElement.cpp
        src/scene/editor_scene/GroupElement.cpp
        src/scene/editor_scene/EmissiveEntityElement.cpp
        src/scene/editor_scene/SceneElement.cpp
)
configure_lab_target(cits3003_project)

# OPTIONAL EXTRA EXECUTABLES ==========================================================================
# Example:
# add_executable(project_demo demos/project_demo.cpp)
# configure_lab_target(project_demo)