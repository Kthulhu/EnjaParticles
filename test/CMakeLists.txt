IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(test CXX)

#change this later
SET( BASE_DIR ${test_SOURCE_DIR}/..)
SET( RTPS_DIR ${test_SOURCE_DIR}/../rtpslib)

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


SET(QT_USE_OPENGL TRUE)
FIND_PACKAGE (Qt4 REQUIRED)
INCLUDE(${QT_USE_FILE})
FIND_LIBRARY(ASSIMP_LIBRARIES assimp
    PATHS ENV LD_LIBRARY_PATH /usr/lib/
)
set (test_INCLUDES
    ${RTPS_DIR}
    ${GLUT_INCLUDE_PATH}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${BASE_DIR}/opencl10
    ${BASE_DIR}/test
)


#library
#set (test_render_SOURCES
#    ParamParser
#    test_render.cpp
#    )

#set (torusTest_SOURCES
#    torusTest.cpp
#    )

#set (torusRBTest_SOURCES
#    torusRBTest.cpp
#    )

set (test_SOURCES
    ParamParser
    TestApplication
    main_test.cpp
    )
set (test_mc_SOURCES
    test_marching_cubes.cpp
    )

set(moc_hdrs glwidget.h mainwindow.h floatslider.h sphparametergroup.h)
QT4_WRAP_CPP(_mocFiles ${moc_hdrs})
set (test_qt_SOURCES
    ParamParser
    aiwrapper
    glwidget.cpp
    mainwindow.cpp
    sphparametergroup.cpp
    floatslider.cpp
    mainqt.cpp
    )



#INCLUDE_DIRECTORIES(${test_INCLUDES})
#ADD_EXECUTABLE(test_render.x ${test_render_SOURCES})

#INCLUDE_DIRECTORIES(${test_INCLUDES})
#ADD_EXECUTABLE(torusTest.x ${torusTest_SOURCES})

#INCLUDE_DIRECTORIES(${test_INCLUDES})
#ADD_EXECUTABLE(torusRBTest.x ${torusRBTest_SOURCES})

INCLUDE_DIRECTORIES(${test_INCLUDES})
ADD_EXECUTABLE(test.x ${test_SOURCES})

INCLUDE_DIRECTORIES(${test_mc_INCLUDES})
ADD_EXECUTABLE(test_marching_cubes.x ${test_mc_SOURCES})

INCLUDE_DIRECTORIES(${test_qt_INCLUDES} ${QT_INCLUDE_DIR} ${QT_QTOPENGL_INCLUDE_DIR})
ADD_EXECUTABLE(test_qt.x  ${_mocFiles} ${test_qt_SOURCES})

#message("current build dir:\n")
#message("${BASE_DIR}/rtps/build")

LINK_DIRECTORIES(${rtpsuite_BINARY_DIR}/rtpslib)
#${rtpsuite_BINARY_DIR}/bin)

SET(LINK_LIBS
    #${BASE_DIR}/rtps/build/rtpslib/librtps.so
    rtps
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENCL_LIBRARIES}
    ${GLEW_LIBRARY}
    ${ASSIMP_LIBRARIES}
    # /opt/local/lib/libGLEW.a #for staticl linking when distributing. should do this more cmake way
    # /usr/lib/libGLEW.a #for static linking when distributing on linux. should do this more cmake way
)
message(RTPS_EXPORTS "${rtps_EXPORTS}")

#TARGET_LINK_LIBRARIES (test_render.x
#    ${LINK_LIBS}
#)
#TARGET_LINK_LIBRARIES (torusTest.x
#    ${LINK_LIBS}
#)
#TARGET_LINK_LIBRARIES (torusRBTest.x
#    ${LINK_LIBS}
#)
TARGET_LINK_LIBRARIES (test.x
    ${LINK_LIBS}
)
TARGET_LINK_LIBRARIES (test_marching_cubes.x
    ${LINK_LIBS}
)
TARGET_LINK_LIBRARIES (test_qt.x
    ${LINK_LIBS}
    ${QT_LIBRARIES}
    ${QT_QTOPENGL_LIBRARIES}
)
#IF(WIN32)
#install (TARGETS test_render.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (TARGETS torusTest.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (TARGETS torusRBTest.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS test.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS test_marching_cubes.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS test_qt.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (FILES MathFunctions.h DESTINATION include)

#ENDIF(WIN32)
