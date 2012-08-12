IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(benchmarks CXX)

#change this later
SET( BASE_DIR ${test_SOURCE_DIR}/../..)
SET( RTPS_DIR ${test_SOURCE_DIR}/../../rtpslib)

ADD_DEFINITIONS(-DGLSL_BIN_DIR="./bin/shaders")

message("base dir: ${BASE_DIR}")

#need to include FindOPENCL.cmake to
SET( CMAKE_MODULE_PATH
     ${BASE_DIR}/cmake
     )

message("${CMAKE_MODULE_PATH}\n")

FIND_PACKAGE (GLUT)
FIND_PACKAGE (OpenGL)
FIND_PACKAGE (OPENCL)
FIND_PACKAGE (GLEW)

set (benchmark_INCLUDES
    ${RTPS_DIR}
    ${GLUT_INCLUDE_PATH}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${BASE_DIR}/opencl10
    ${BASE_DIR}/test
)

set (benchmark_SOURCES
    ../ParamParser
    ../aiwrapper
    ../TestApplication
    )

INCLUDE_DIRECTORIES(${benchmark_INCLUDES})
ADD_EXECUTABLE(rb_benchmark.x benchmark_main.cpp rb_benchmark  ${benchmark_SOURCES})
ADD_EXECUTABLE(rb_sph_benchmark.x rb_sph_benchmark_main.cpp rb_sph_benchmark  ${benchmark_SOURCES})

LINK_DIRECTORIES(${rtpsuite_BINARY_DIR}/rtpslib)

SET(LINK_LIBS
    rtps
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENCL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${ASSIMP_LIBRARIES}
)
message(RTPS_EXPORTS "${rtps_EXPORTS}")

TARGET_LINK_LIBRARIES (rb_benchmark.x
    ${LINK_LIBS}
)
TARGET_LINK_LIBRARIES (rb_sph_benchmark.x
    ${LINK_LIBS}
)
install (TARGETS rb_benchmark.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES rb_benchmark.xml DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES ../demo_mesh_scene.obj DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES ../demo_mesh_scene.mtl DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (FILES ../demo_mesh_box_scene.obj DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (FILES ../demo_mesh_box_scene.mtl DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (FILES ../demo_mesh_sphere_scene.obj DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (FILES ../demo_mesh_sphere_scene.mtl DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES ../demo_scene_plane.obj DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES ../demo_scene_plane.mtl DESTINATION ${rtpsuite_BINARY_DIR}/bin)

install (TARGETS rb_sph_benchmark.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES rb_sph_benchmark.xml DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES ../demo_scene_big_box.obj DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (FILES ../demo_scene_big_box.mtl DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#ENDIF(WIN32)
