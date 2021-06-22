cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

set(PROJECT_NAME ae-asteroids)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_UNICODE -DUNICODE)

if(WIN32)
	set(EXE_TYPE WIN32) # Create a regular windowed application instead of the default console subsystem target
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup") # Use main instead of WinMain
endif()

FetchContent_Declare(
    aether-game-utils
    GIT_REPOSITORY https://github.com/johnhues/aether-game-utils.git
    GIT_TAG 8f200b36afc683bdc2ceeb817834e97bb4fcb6e1
)
FetchContent_Populate(aether-game-utils)
FetchContent_GetProperties(aether-game-utils SOURCE_DIR AE_SOURCE_DIR)
set(AE_INCLUDE_DIRS ${AE_SOURCE_DIR}/include)

find_package(OpenGL REQUIRED)
add_executable(${PROJECT_NAME} ${EXE_TYPE} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${AE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
