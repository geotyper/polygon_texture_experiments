cmake_minimum_required(VERSION 3.5)

set(EXECUTABLE_NAME "jscpp_exp")
project(${EXECUTABLE_NAME}
    VERSION     0.15
    DESCRIPTION "SDL2 JS exp "
    LANGUAGES CXX
)

option(USE_JS "Build JS" OFF)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GCC")
  MESSAGE("GCC")
endif()

#if(CMAKE_COMPILER_IS_GNUCC)
#   option(USE_JS "Build JS" OFF)
#endif()

if(USE_JS)
  MESSAGE(STATUS "WITH_JS")
  add_definitions(-DUSE_JS)
endif(USE_JS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# GL Extension Wrangler (GLEW) get dev libraries from http://glew.sourceforge.net/
if(NOT USE_JS)
find_package(GLEW REQUIRED)
endif(NOT USE_JS)

#GLM LIB
set(GLM_DIR  ${CMAKE_SOURCE_DIR}/external/glm-0.9.9.8/cmake/glm)
find_package(GLM REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})
link_directories(${GLM_LIBRARY_DIRS})
add_definitions(${GLM_DEFINITIONS})

find_package(CGAL REQUIRED)

if(NOT GLM_FOUND)
        message(Error "GLM not found")
endif(NOT GLM_FOUND)

# IMGUI
#set(IMGUI_SOURCES ${IMGUI_ROOT}/imgui.cpp ${IMGUI_ROOT}/imgui_demo.cpp ${IMGUI_ROOT}/imgui_draw.cpp ${IMGUI_ROOT}/imgui_widgets.cpp ${IMGUI_ROOT}/imgui_tables.cpp)
#set(IMGUI_IMPL_SOURCES ${IMGUI_ROOT}/backends/imgui_impl_sdl.cpp ${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp)

set(CMAKE_CXX_FLAGS -DIMGUI_DISABLE_FILE_FUNCTIONS)


# IMGUI LIB
set (IMGUILIB ImGuiLibrary)
set(IMGUI_ROOT ${CMAKE_SOURCE_DIR}/external/imgui-1.85)
set(IMGUI_SOURCES ${IMGUI_ROOT}/imgui.cpp ${IMGUI_ROOT}/imgui_demo.cpp ${IMGUI_ROOT}/imgui_draw.cpp ${IMGUI_ROOT}/imgui_widgets.cpp ${IMGUI_ROOT}/imgui_tables.cpp)
set(IMGUI_IMPL_SOURCES ${IMGUI_ROOT}/backends/imgui_impl_sdl.cpp ${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp)
#add_library(${IMGUILIB} STATIC ${IMGUI_SOURCES} ${IMGUI_IMPL_SOURCES} )
#target_include_directories(${IMGUILIB} PUBLIC "${IMGUI_ROOT}")
#target_include_directories(${IMGUILIB} PUBLIC "${IMGUI_ROOT}/backends")

SET(TARGET_H
    src/Constants.hpp
    src/structs.hpp

    src/support/shaderutils.hpp
    src/support/arcball_camera.hpp
    src/support/Buffer.h
    src/support/ComputeProcess.h
    src/support/ConfigParser.h
    src/support/Icosphere.h
    src/support/NoiseSettings.h
    src/support/texture.hpp

    src/support/meshData.h
    src/support/meshCreator.h

    src/graphmodule.hpp
    src/solver.hpp

)

SET(TARGET_SRC

    src/support/shaderutils.cpp
    src/support/Buffer.cpp
    src/support/ComputeProcess.cpp
    src/support/ConfigParser.cpp
    src/support/Icosphere.cpp
    src/support/NoiseSettings.cpp
    src/support/texture.cpp

    src/support/meshData.cpp

    src/graphmodule.cpp
    src/graphmodule_support.cpp
    src/graphmodule_render.cpp

    src/solver.cpp

    main.cpp
)

SET(pTARGET_H
   src/poly.hpp
  # src/poly/ofxPolygonDetector.h
   #src/poly/PolyDetector.h
   #src/poly/geom.h
   #src/poly/swipline.hpp
   src/poly/SweepLine/sweepline.hpp
   src/poly/SweepLine/segment.hpp
   src/poly/SweepLine/event.hpp
   src/poly/SweepLine/point.hpp
   src/poly/SweepLine/constants.hpp
   src/poly/clipper.hpp
   src/poly/polygoncreator.hpp
   #src/poly/Polygon.h
   #src/poly/gpc.h
   src/poly/polypartition.h
   src/poly/delaunator.hpp
   src/poly/colormap/colormap.h

)

SET(pTARGET_SRC
  src/poly.cpp
  #src/poly/ofxPolygonDetector.cpp
  #src/poly/PolyDetector.cpp
  #src/poly/swipline.cpp
  src/poly/SweepLine/sweepline.cpp
  src/poly/SweepLine/segment.cpp
  src/poly/SweepLine/point.cpp
  src/poly/SweepLine/event.cpp
  src/poly/clipper.cpp
  src/poly/polygoncreator.cpp
  #src/poly/Polygon.cpp
  #src/poly/gpc.c
  src/poly/polypartition.cpp

)

add_library(polyLib STATIC  ${pTARGET_SRC} ${pTARGET_H})
target_link_libraries(polyLib PUBLIC CGAL::CGAL)




#FUNCTIONSPACE LIB
set(FUNCTIONSPACE_PATH ${CMAKE_SOURCE_DIR}/src/functionspace)

set(FUNCTIONSPACE_H
                ${FUNCTIONSPACE_PATH}/ColorMap.h
                ${FUNCTIONSPACE_PATH}/FunctionPool.h
                ${FUNCTIONSPACE_PATH}/GenerativeArt.h
                ${FUNCTIONSPACE_PATH}/RandomFunction.h
                ${FUNCTIONSPACE_PATH}/colormap/colormap.h

                )
set(FUNCTIONSPACE_SRC
                ${FUNCTIONSPACE_PATH}/FunctionPool.cpp

                )
set (FunctionSpaceLIB functionspaceLib)
add_library(${FunctionSpaceLIB} STATIC ${FUNCTIONSPACE_H} ${FUNCTIONSPACE_SRC} )


target_include_directories(${FunctionSpaceLIB} PRIVATE ${FUNCTIONSPACE_H})
#target_include_directories(${THISLIB} PRIVATE ${PROJECT_SOURCE_DIR}/external/CLI11)
#target_include_directories(${THISLIB} PRIVATE ${PROJECT_SOURCE_DIR}/external/png++)

#target_link_libraries(${THISLIB} pthread ${LIBPNG_LIBRARIES} -lpng)
#target_link_libraries(${THISLIB} PRIVATE ${LIBPNG_LIBRARIES} -lpng)
#target_sources(${FunctionSpaceLIB} PRIVATE ${FUNCTIONSPACE_SRC})


add_executable(${EXECUTABLE_NAME} ${TARGET_SRC} ${TARGET_H}
                       ${IMGUI_SOURCES} ${IMGUI_IMPL_SOURCES})


target_link_libraries(${EXECUTABLE_NAME} polyLib)

target_include_directories(${EXECUTABLE_NAME} PUBLIC "${IMGUI_ROOT}")
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${IMGUI_ROOT}/backends")


target_link_libraries(${EXECUTABLE_NAME} ${FunctionSpaceLIB}  )

if(NOT USE_JS)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)
target_link_libraries(${EXECUTABLE_NAME} OpenGL::GL)
target_include_directories(${EXECUTABLE_NAME} PUBLIC OpenGL::GL)

target_link_libraries(${EXECUTABLE_NAME} "${GLEW_LIBRARIES}" -ldl -lpthread )
target_include_directories(${EXECUTABLE_NAME} PUBLIC "${GLEW_INCLUDE_DIRS}")

endif()


#target_link_libraries(${EXECUTABLE_NAME} ${GLM_LIBRARIES} CGAL::CGAL)

# Windows prereq: get dev libraries from https://www.libsdl.org/download-2.0.php
#find_package(SDL2 REQUIRED)
#include_directories(${SDL2_INCLUDE_DIRS})
# workaround for CMP0004 issue with the SDL2 libraries on Linux
#string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)

#-frounding-math
target_link_libraries(${EXECUTABLE_NAME} ${SDL2_LIBRARIES})
if(USE_JS)
set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-sUSE_SDL=2  -sALLOW_MEMORY_GROWTH -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -s DEMANGLE_SUPPORT=1 --preload-file ${CMAKE_BINARY_DIR}/assets --bind")
#set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-sUSE_SDL=2  -sUSE_BOOST_HEADERS=1")
#set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -s  --preload-file ${CMAKE_BINARY_DIR}/assets --bind")
endif(USE_JS)
