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

file(GLOB_RECURSE ASTEROID_SOURCES CONFIGURE_DEPENDS "src/*.h" "src/*.cpp")
if(APPLE)
	list(APPEND ASTEROID_SOURCES src/ae.mm) # Apple builds must use objective-c
endif()

# aether-game-utils
FetchContent_Declare(
	aether-game-utils
	GIT_REPOSITORY https://github.com/johnhues/aether-game-utils.git
	GIT_TAG e4f65d3616137dcae2130bdd6f52d9e98c450cca
)
FetchContent_Populate(aether-game-utils) # Download but don't build
FetchContent_GetProperties(aether-game-utils SOURCE_DIR AE_SOURCE_DIR) # Store download path in AE_SOURCE_DIR
list(APPEND ASTEROID_INC_DIRS ${AE_SOURCE_DIR}/include) # aether.h path
list(APPEND ASTEROID_SOURCES ${AE_SOURCE_DIR}/include/ae/aether.h) # This line is only needed so aether.h shows up in IDE

# entt
FetchContent_Declare(
	entt
	GIT_REPOSITORY https://github.com/skypjack/entt.git
	GIT_TAG 9460e04ea547e7f4f3923c5458b187781c604220
)
FetchContent_Populate(entt) # Download but don't build
FetchContent_GetProperties(entt SOURCE_DIR ENTT_SOURCE_DIR)
list(APPEND ASTEROID_INC_DIRS ${ENTT_SOURCE_DIR}/src)
list(APPEND ASTEROID_SOURCES ${ENTT_SOURCE_DIR}/src/entt/entt.hpp)

# openfbx
FetchContent_Declare(
	openfbx
	GIT_REPOSITORY https://github.com/nem0/OpenFBX.git
	GIT_TAG 5ce5c3490a90d346366abcda48e1d77526d09a15
)
FetchContent_Populate(openfbx) # Download but don't build
FetchContent_GetProperties(openfbx SOURCE_DIR OPENFBX_SOURCE_DIR)
file(GLOB OPENFBX_SOURCES CONFIGURE_DEPENDS "${OPENFBX_SOURCE_DIR}/src/*")
add_library(openfbx ${OPENFBX_SOURCES})
target_include_directories(openfbx INTERFACE "${OPENFBX_SOURCE_DIR}/src")

# ae-asteroids
set(PROJECT_NAME ae-asteroids)
project(${PROJECT_NAME} LANGUAGES CXX C VERSION 0.0.0)
add_executable(${PROJECT_NAME} ${EXE_TYPE} ${ASTEROID_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC ${ASTEROID_INC_DIRS}) # Includes for executable build
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} openfbx) # Libraries to link in executable

# App bundle
if(APPLE)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		MACOSX_BUNDLE ON
		OUTPUT_NAME ${PROJECT_NAME}
		XCODE_GENERATE_SCHEME ON
		XCODE_SCHEME_WORKING_DIRECTORY "$(PROJECT_DIR)"
	)
endif()
