cmake_minimum_required(VERSION 3.21)

project(Coursework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE debug)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

include("Depencies.cmake")

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_executable(101 
    "source/Main.cpp" 
    "source/engine/utility/CheckError.cpp"
    "source/engine/Window.cpp"
    "source/engine/rendering/SpriteRenderer.cpp"
    "source/engine/rendering/TextureWrapper.cpp"
    "source/engine/rendering/ShaderWrapper.cpp"
    "source/engine/Application.cpp"
    "source/engine/ResourceManager.cpp"
    "source/engine/Logger.cpp"
    "source/engine/Sprite.cpp"
    "source/engine/AnimatedSprite.cpp"
    "source/game/Program.cpp"
	"source/game/GameBoard.cpp"
)

target_link_libraries(101 PRIVATE glfw imgui_glfw glad glm::glm spdlog::spdlog)
target_compile_features(101 PRIVATE cxx_std_20)

add_custom_command(TARGET 101 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:101> "../../../bin/")

install(TARGETS 101)
