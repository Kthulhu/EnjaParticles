IF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
	MESSAGE(FATAL_ERROR "CMake generation must always be out-of-source!
    Remove the CMakeCache.txt file and try again from another folder")
ENDIF (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project(rtpslib C CXX)

#change this later
SET( CMAKE_MODULE_PATH ${RTPS_CMAKE_MODULE_PATH})
message("cmake_module_path: ${CMAKE_MODULE_PATH}\n")

#this mostly affects the path to cl files
SET(RTPS_DEBUG FALSE)
#SET(RTPS_DEBUG FALSE)

#only works on mac/linux
#SET(PREPROCESS_CL TRUE)
SET(PREPROCESS_CL FALSE)

###### SETTING RELATED TO GPU VS CPU IMPLEMENTATIONS ##########################
SET(WITH_OPENCL TRUE)

#ADD_DEFINITIONS(-DDEBUG)
IF(WITH_OPENCL)
    ADD_DEFINITIONS(-DGPU)
ENDIF(WITH_OPENCL)
#might want to have CPU even if we have OpenCL available
#ADD_DEFINITIONS(-DCPU)

##############################################################################

##### Source Paths ############################################################
#ADD_DEFINITIONS(-DSIMPLE_CL_SOURCE_DIR="cl_simple")
ADD_DEFINITIONS(-DSPH_CL_SOURCE_DIR="cl_sph")
ADD_DEFINITIONS(-DPARTICLE_RIGIDBODY_CL_SOURCE_DIR="cl_rigidbody")
ADD_DEFINITIONS(-DFLOCK_CL_SOURCE_DIR="cl_flock")
ADD_DEFINITIONS(-DCOMMON_CL_SOURCE_DIR="cl_common")
ADD_DEFINITIONS(-DGLSL_BIN_DIR="shaders")
ADD_DEFINITIONS(-DGLSL_SOURCE_DIR="render")
##############################################################################


FIND_PACKAGE (OpenGL)
FIND_PACKAGE (OPENCL)
FIND_PACKAGE (GLEW)

message("OPENCL INCLUDE: ${OPENCL_INCLUDE_DIR}\n")

set (rtpslib_INCLUDES
    #${GLUT_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_PATH}
    #${BASE_DIR}/opencl10
    ${OPENCL_INCLUDE_DIR}
    ./
    ./system
    ./system/sph
    ./system/flock
    ${rtpslib_SOURCE_DIR}/system/rigidbody
    ${rtpslib_SOURCE_DIR}/system/common
    ${rtpslib_SOURCE_DIR}/render
    ${rtpslib_SOURCE_DIR}/render/util
    ${rtpslib_SOURCE_DIR}/domain
    ${rtpslib_SOURCE_DIR}/opencl
    ${rtpslib_SOURCE_DIR}/matrix_library
    #${rtpslib_SOURCE_DIR}/opencl/bitonic_sort/src
)    

#library source files (don't explicitly give file extension, inferred from source type)
set (rtpslib_SOURCES
    ${rtpslib_SOURCE_DIR}/RTPS
    ${rtpslib_SOURCE_DIR}/RTPSSettings
    ${rtpslib_SOURCE_DIR}/util
    ${rtpslib_SOURCE_DIR}/structs
    #${rtpslib_SOURCE_DIR}/timege    #gordon's timer class
    ${rtpslib_SOURCE_DIR}/timer_eb  #evan's timer class
    ${rtpslib_SOURCE_DIR}/rtps_common

    #${rtpslib_SOURCE_DIR}/render/Render
    #${rtpslib_SOURCE_DIR}/render/SpriteRender
    #${rtpslib_SOURCE_DIR}/render/SSFRender
    #${rtpslib_SOURCE_DIR}/render/Sphere3DRender
    ${rtpslib_SOURCE_DIR}/render/Shader
    ${rtpslib_SOURCE_DIR}/render/Camera
    ${rtpslib_SOURCE_DIR}/render/Quaternion
    ${rtpslib_SOURCE_DIR}/render/MeshEffect
    ${rtpslib_SOURCE_DIR}/render/ParticleEffect
    ${rtpslib_SOURCE_DIR}/render/SSEffect
    ${rtpslib_SOURCE_DIR}/render/StreamlineEffect
    ${rtpslib_SOURCE_DIR}/render/ShaderLibrary
    ${rtpslib_SOURCE_DIR}/render/RenderUtils
    ${rtpslib_SOURCE_DIR}/render/util/stb_image.c
    ${rtpslib_SOURCE_DIR}/render/util/stb_image_write.h

    ${rtpslib_SOURCE_DIR}/domain/Domain
    ${rtpslib_SOURCE_DIR}/domain/IV
    # ${rtpslib_SOURCE_DIR}/system/common/Hose
    

    # common use for all systems
    ${rtpslib_SOURCE_DIR}/system/common/Hose
    ${rtpslib_SOURCE_DIR}/system/common/Hash
    ${rtpslib_SOURCE_DIR}/system/common/CellIndices
    ${rtpslib_SOURCE_DIR}/system/common/Permute
    ${rtpslib_SOURCE_DIR}/system/common/Sample
    ${rtpslib_SOURCE_DIR}/system/common/Gravity
    ${rtpslib_SOURCE_DIR}/system/common/MarchingCubes
	
    ${rtpslib_SOURCE_DIR}/system/System
    
    # Simple
    #${rtpslib_SOURCE_DIR}/system/Simple.cpp
    #${rtpslib_SOURCE_DIR}/system/simple/ForceField
    #${rtpslib_SOURCE_DIR}/system/simple/Euler
    	
	# SPH
    ${rtpslib_SOURCE_DIR}/system/SPH.cpp
    ${rtpslib_SOURCE_DIR}/system/SPHSettings.cpp


    ${rtpslib_SOURCE_DIR}/system/sph/Euler
    ${rtpslib_SOURCE_DIR}/system/sph/LeapFrog
    ${rtpslib_SOURCE_DIR}/system/sph/Lifetime
    ${rtpslib_SOURCE_DIR}/system/sph/Density
    ${rtpslib_SOURCE_DIR}/system/sph/Force
    ${rtpslib_SOURCE_DIR}/system/sph/RigidBodyForce
    ${rtpslib_SOURCE_DIR}/system/sph/Collision_wall
    ${rtpslib_SOURCE_DIR}/system/sph/Collision_triangle
    ${rtpslib_SOURCE_DIR}/system/sph/ColorField

    
    #### these are CPU only
    ${rtpslib_SOURCE_DIR}/system/sph/Pressure
    ${rtpslib_SOURCE_DIR}/system/sph/Viscosity
    ${rtpslib_SOURCE_DIR}/system/sph/XSPH
    ####


	# FLOCK
    ${rtpslib_SOURCE_DIR}/system/FLOCK.cpp
    ${rtpslib_SOURCE_DIR}/system/FLOCKSettings.cpp
    ${rtpslib_SOURCE_DIR}/system/flock/EulerIntegration
    ${rtpslib_SOURCE_DIR}/system/flock/Rules

    # Added by GE
    ${rtpslib_SOURCE_DIR}/system/boids
    ####

    ${rtpslib_SOURCE_DIR}/system/ParticleShape

    #ParticleRigidBody 
    ${rtpslib_SOURCE_DIR}/system/ParticleRigidBody
    # Rigidbody dynamics.
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBEuler
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBLeapFrog
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBForce
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBForceFluid
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBForceStatic
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBSegmentedScan
    ${rtpslib_SOURCE_DIR}/system/rigidbody/PRBUpdateParticles


    ${rtpslib_SOURCE_DIR}/system/common/MeshToParticles
    )

IF (WIN32)
    set (rtpslib_SOURCES 
    	${rtpslib_SOURCES}
    	${rtpslib_SOURCE_DIR}/gtod_windows
    )
ENDIF ()


IF(WITH_OPENCL)
SET(rtpslib_SOURCES ${rtpslib_SOURCES}
    ${rtpslib_SOURCE_DIR}/opencl/CLL
    ${rtpslib_SOURCE_DIR}/opencl/Kernel
    #${rtpslib_SOURCE_DIR}/opencl/Buffer #template class

    #${rtpslib_SOURCE_DIR}/opencl/bitonic_sort/src/oclBitonicSort_launcher
    #${rtpslib_SOURCE_DIR}/opencl/bitonic_sort/src/BitonicSort #template class
    #these should probably be in the OpenCL classes
    #${rtpslib_SOURCE_DIR}/system/sph/Scopy
    #${rtpslib_SOURCE_DIR}/system/flock/Scopy
    #${rtpslib_SOURCE_DIR}/system/sph/SetInt
    )
ENDIF(WITH_OPENCL)


INCLUDE_DIRECTORIES(${rtpslib_INCLUDES})
#ADD_LIBRARY(rtps SHARED ${rtpslib_SOURCES})
ADD_LIBRARY(rtps STATIC ${rtpslib_SOURCES})

TARGET_LINK_LIBRARIES (rtps
    #${GLUT_LIBRARIES}
   ${OPENGL_LIBRARIES}
   ${OPENCL_LIBRARIES}
   ${GLEW_LIBRARY}
   # /opt/local/lib/libGLEW.a #for static linking when distributing on mac. should do this more cmake way
   #/usr/lib/libGLEW.a #for static linking when distributing on linux. should do this more cmake way
)


#Doxygen
#find_package(Doxygen REQUIRED)
find_package(Doxygen)
set(DOXYGEN_INPUT   ${rtpslib_SOURCE_DIR}/doc/doxyfile)
set(DOXYGEN_OUTPUT  ${rtpslib_BINARY_DIR}/html/index.html)
file(MAKE_DIRECTORY ${rtpslib_BINARY_DIR}/html)

configure_file(
  ${rtpslib_SOURCE_DIR}/doc/doxyfile
  ${DOXYGEN_INPUT} @ONLY
)

add_custom_command(
  OUTPUT  ${DOXYGEN_OUTPUT}
  COMMAND ${DOXYGEN} ${DOXYGEN_INPUT}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${DOXYGEN_INPUT}
)

add_custom_target(doc DEPENDS ${DOXYGEN_OUTPUT})



ADD_SUBDIRECTORY(system/common)
#ADD_SUBDIRECTORY(system/simple)
ADD_SUBDIRECTORY(system/sph)
ADD_SUBDIRECTORY(system/flock)
ADD_SUBDIRECTORY(system/rigidbody)

MAKE_DIRECTORY(${rtpslib_BINARY_DIR}/shaders)
FILE(COPY ${rtpslib_SOURCE_DIR}/render/shaders/ DESTINATION ${rtpslib_BINARY_DIR}/shaders/)

#IF(WIN32)
install (TARGETS rtps DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (TARGETS rtps DESTINATION ${RTPS_INSTALL_DIR})
#this should probably be done in a different way
INSTALL(CODE "FILE(MAKE_DIRECTORY ${RTPS_INSTALL_DIR}/shaders)")
install (DIRECTORY ${rtpslib_BINARY_DIR}/shaders/ DESTINATION ${RTPS_INSTALL_DIR}/shaders/)
#ENDIF(WIN32)
