IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(test CXX)

#change this later
SET( BASE_DIR ${test_SOURCE_DIR}/..)
SET( RTPS_DIR ${test_SOURCE_DIR}/../rtpslib)

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

set (test_INCLUDES
    ${RTPS_DIR}
    ${GLUT_INCLUDE_PATH}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    ${BASE_DIR}/opencl10
)


#library
set (test_SOURCES
    main.cpp
    )

set (test_multi_SOURCES
    main_multi.cpp
    )


#set (test_simple_SOURCES
#    main_simple.cpp
#    )

set (test_flock_SOURCES
    main_flock.cpp
    )

set (test_rigidbody_SOURCES
    main_rigidbody.cpp
    )

set (test_rb_sph_SOURCES
    main_rb_sph.cpp
    )
set (test_rb_sph_SOURCES
    test_rb_sph_mass.cpp
    )

INCLUDE_DIRECTORIES(${test_INCLUDES})
ADD_EXECUTABLE(sph.x ${test_SOURCES})
ADD_EXECUTABLE(multi.x ${test_multi_SOURCES})
#ADD_EXECUTABLE(simple.x ${test_simple_SOURCES})
ADD_EXECUTABLE(flock.x ${test_flock_SOURCES})
ADD_EXECUTABLE(rigidbody.x ${test_rigidbody_SOURCES})
ADD_EXECUTABLE(rb_sph.x ${test_rb_sph_SOURCES})
ADD_EXECUTABLE(rb_sph_mass.x ${test_rb_sph_SOURCES})

message("current build dir:\n")
message("${BASE_DIR}/rtps/build")

LINK_DIRECTORIES(${rtpsuite_BINARY_DIR}/rtpslib)
#${rtpsuite_BINARY_DIR}/bin)

SET(LINK_LIBS
    #${BASE_DIR}/rtps/build/rtpslib/librtps.so
    rtps
    ${GLUT_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${OPENCL_LIBRARIES}
    ${GLEW_LIBRARY}
    # /opt/local/lib/libGLEW.a #for staticl linking when distributing. should do this more cmake way
    # /usr/lib/libGLEW.a #for static linking when distributing on linux. should do this more cmake way
)
message(RTPS_EXPORTS "${rtps_EXPORTS}")

TARGET_LINK_LIBRARIES (sph.x
    ${LINK_LIBS}
)


TARGET_LINK_LIBRARIES (multi.x
    ${LINK_LIBS}
)

#TARGET_LINK_LIBRARIES (simple.x
#    ${LINK_LIBS}
#)

TARGET_LINK_LIBRARIES (flock.x
    ${LINK_LIBS}
)
TARGET_LINK_LIBRARIES (rigidbody.x
    ${LINK_LIBS}
)
TARGET_LINK_LIBRARIES (rb_sph.x
    ${LINK_LIBS}
)
TARGET_LINK_LIBRARIES (rb_sph_mass.x
    ${LINK_LIBS}
)
#IF(WIN32)
install (TARGETS sph.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS multi.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (TARGETS simple.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS flock.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS rigidbody.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS rb_sph.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
install (TARGETS rb_sph_mass.x DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (FILES MathFunctions.h DESTINATION include)

#ENDIF(WIN32)
