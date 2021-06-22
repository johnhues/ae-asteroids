cmake_minimum_required(VERSION 3.10 FATAL_ERROR)
include(FetchContent)

# Build configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D_UNICODE -DUNICODE)
add_compile_options(-fmodules -fcxx-modules)
if(WIN32)
	set(EXE_TYPE WIN32) # Create a regular windowed application instead of the default console subsystem target
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup") # Use main instead of WinMain
endif()
# find_package(OpenGL REQUIRED)

# aether-game-utils
FetchContent_Declare(
	aether-game-utils
	GIT_REPOSITORY https://github.com/johnhues/aether-game-utils.git
	GIT_TAG 4b21f1a949abe2502fe323c6134b6df90cebb961
)
FetchContent_Populate(aether-game-utils) # Download but don't build
FetchContent_GetProperties(aether-game-utils SOURCE_DIR AE_SOURCE_DIR) # Store download path in AE_SOURCE_DIR
set(AE_INCLUDE_DIRS ${AE_SOURCE_DIR}/include) # aether.h path
list(APPEND AE_SRC ${AE_SOURCE_DIR}/include/ae/aether.h) # This line is only needed so aether.h shows up in IDE
if(APPLE)
	list(APPEND AE_SRC ae.mm) # Apple builds must use objective-c
else()
	list(APPEND AE_SRC ae.cpp) # Other builds use c++
endif()

# ae-asteroids
set(PROJECT_NAME ae-asteroids)
project(${PROJECT_NAME} LANGUAGES CXX VERSION 0.0.0)
add_executable(${PROJECT_NAME} ${EXE_TYPE} main.cpp ${AE_SRC}) # Create executable
target_include_directories(${PROJECT_NAME} PUBLIC ${AE_INCLUDE_DIRS}) # Includes for executable build
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES}) # Libraries to link in executable
