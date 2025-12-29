cmake_minimum_required(VERSION 3.19)

project(kholst C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(SRC_FILES
    "src/main.cpp"
    "src/utils.cpp"
)

set(HEADER_FILES
    "src/utils.h"
)

set(SHADER_FILES
    ""
)

find_package(Python3 COMPONENTS Interpreter)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BOOTSTRAP_DIR ${ROOT_DIR}/external)
set(BOOTSTRAP_FILE ${BOOTSTRAP_DIR}/bootstrap.json) 
set(BOOTSTRAP_CACHE ${BOOTSTRAP_DIR}/.bootstrap.json)
set(LVK_SOURCE ${BOOTSTRAP_DIR}/src/lightweightvk)

execute_process(COMMAND ${Python3_EXECUTABLE} deploy_deps.py WORKING_DIRECTORY ${ROOT_DIR} COMMAND_ERROR_IS_FATAL ANY)
execute_process(COMMAND ${Python3_EXECUTABLE} deploy_deps.py WORKING_DIRECTORY ${LVK_SOURCE} COMMAND_ERROR_IS_FATAL ANY)

add_custom_command(COMMAND ${Python3_EXECUTABLE} deploy_deps.py OUTPUT ${BOOTSTRAP_CACHE}
                   DEPENDS ${BOOTSTRAP_FILE} WORKING_DIRECTORY ${ROOT_DIR})
add_custom_target(Dependencies DEPENDS ${BOOTSTRAP_FILE})
set_property(TARGET Dependencies PROPERTY FOLDER "external")

if(NOT EXISTS ${BOOTSTRAP_CACHE})
  message(FATAL_ERROR "Cannot bootstrap dependencies.")
endif()

include_directories(src)
include_directories(external/src)

include_directories(${LVK_SOURCE}/third-party/deps/src/taskflow)
include_directories(${LVK_SOURCE}/third-party/deps/src/glm)
include_directories(${LVK_SOURCE}/third-party/deps/src/imgui)
include_directories(${LVK_SOURCE}/third-party/deps/src/glfw)

add_subdirectory(${LVK_SOURCE}/third-party/deps/src/ldrutils)
set(KTX_FEATURE_DOC            OFF CACHE BOOL "")
set(KTX_FEATURE_GL_UPLOAD      OFF CACHE BOOL "")
set(KTX_FEATURE_JNI            OFF CACHE BOOL "")
set(KTX_FEATURE_KTX1            ON CACHE BOOL "")
set(KTX_FEATURE_KTX2            ON CACHE BOOL "")
set(KTX_FEATURE_LOADTEST_APPS  OFF CACHE BOOL "")
set(KTX_FEATURE_STATIC_LIBRARY  ON CACHE BOOL "")
set(KTX_FEATURE_TESTS          OFF CACHE BOOL "")
set(KTX_FEATURE_TOOLS          OFF CACHE BOOL "")
set(KTX_FEATURE_VK_UPLOAD      OFF CACHE BOOL "")
add_subdirectory(${LVK_SOURCE}/third-party/deps/src/ktx-software)
set(LVK_WITH_SAMPLES   OFF CACHE BOOL "")
set(LVK_WITH_TRACY     ON  CACHE BOOL "")
set(LVK_WITH_TRACY_GPU ON  CACHE BOOL "")
add_subdirectory(${LVK_SOURCE})
lvk_set_folder(ktx "third-party/ktx-software")
lvk_set_folder(ktx_read "third-party/ktx-software")
lvk_set_folder(ktx_version "third-party/ktx-software")
lvk_set_folder(obj_basisu_cbind "third-party/ktx-software")
lvk_set_folder(objUtil "third-party/ktx-software")
if(TARGET astcenc-avx2-static)
  lvk_set_folder(astcenc-avx2-static "third-party/ktx-software")
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(astcenc-avx2-static PRIVATE -Wno-overriding-option)
  endif()
endif()

if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${HEADER_FILES} ${SHADER_FILES})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET LUtils PROPERTY FOLDER "external")
if(WIN32)
  if(TARGET zlibstatic)
    set_property(TARGET zlibstatic PROPERTY FOLDER "external")
  endif()
  if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "external")
  endif()
endif()
# set_property(TARGET uninstall PROPERTY FOLDER "external")

# disable warnings in third-party code
if(MSVC)
	target_compile_options(assimp PRIVATE /wd4267)
	target_compile_options(SPIRV  PRIVATE /wd4267)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC LUtils)
target_link_libraries(${PROJECT_NAME} PUBLIC LVKLibrary)
target_link_libraries(${PROJECT_NAME} PUBLIC LVKstb)
target_link_libraries(${PROJECT_NAME} PUBLIC ktx)
