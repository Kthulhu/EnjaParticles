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
SET(PREPROCESS_CL TRUE)

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
    ./system/rigidbody
    ./system/common
    ./render
    ./render/util
    ./domain
    ./opencl
    ./matrix_library
    #./opencl/bitonic_sort/src
)    

#library source files (don't explicitly give file extension, inferred from source type)
set (rtpslib_SOURCES
    ./RTPS
    ./RTPSSettings
    ./util
    ./structs
    #./timege    #gordon's timer class
    ./timer_eb  #evan's timer class
    ./rtps_common

    #./render/Render
    #./render/SpriteRender
    #./render/SSFRender
    #./render/Sphere3DRender
    ./render/Shader
    ./render/ParticleEffect
    ./render/SSEffect
    ./render/StreamlineEffect
    ./render/ShaderLibrary
    ./render/RenderUtils
    ./render/util/stb_image.c
    ./render/util/stb_image_write.h

    ./domain/Domain
    ./domain/IV
    # ./system/common/Hose
    

    # common use for all systems
    ./system/common/Hose
    ./system/common/Hash
    ./system/common/CellIndices
    ./system/common/Permute
    ./system/common/Sample
    ./system/common/Gravity
	
    ./system/System
    
    # Simple
    #./system/Simple.cpp
    #./system/simple/ForceField
    #./system/simple/Euler
    	
	# SPH
    ./system/SPH.cpp
    ./system/SPHSettings.cpp


    ./system/sph/Euler
    ./system/sph/LeapFrog
    ./system/sph/Lifetime
    ./system/sph/Density
    ./system/sph/Force
    ./system/sph/RigidBodyForce
    ./system/sph/Collision_wall
    ./system/sph/Collision_triangle

    
    #### these are CPU only
    ./system/sph/Pressure
    ./system/sph/Viscosity
    ./system/sph/XSPH
    ####


	# FLOCK
    ./system/FLOCK.cpp
    ./system/FLOCKSettings.cpp
    ./system/flock/EulerIntegration
    ./system/flock/Rules

    # Added by GE
    ./system/boids
    ####

    ./system/ParticleShape

    #ParticleRigidBody 
    ./system/ParticleRigidBody
    # Rigidbody dynamics.
    ./system/rigidbody/PRBEuler
    ./system/rigidbody/PRBLeapFrog
    ./system/rigidbody/PRBForce
    ./system/rigidbody/PRBForceFluid
    ./system/rigidbody/PRBSegmentedScan
    ./system/rigidbody/PRBUpdateParticles


    ./system/common/MeshToParticles
    )

IF (WIN32)
    set (rtpslib_SOURCES 
    	${rtpslib_SOURCES}
    	./gtod_windows
    )
ENDIF ()


IF(WITH_OPENCL)
SET(rtpslib_SOURCES ${rtpslib_SOURCES}
    ./opencl/CLL
    ./opencl/Kernel
    #./opencl/Buffer #template class

    #./opencl/bitonic_sort/src/oclBitonicSort_launcher
    #./opencl/bitonic_sort/src/BitonicSort #template class
    #these should probably be in the OpenCL classes
    #./system/sph/Scopy
    #./system/flock/Scopy
    #./system/sph/SetInt
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
set(DOXYGEN_INPUT   ./doc/doxyfile)
set(DOXYGEN_OUTPUT  ${rtpslib_BINARY_DIR}/html/index.html)
file(MAKE_DIRECTORY ${rtpslib_BINARY_DIR}/html)

configure_file(
  ./doc/doxyfile
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
FILE(COPY ./render/shaders/ DESTINATION ${rtpslib_BINARY_DIR}/shaders/)

#IF(WIN32)
install (TARGETS rtps DESTINATION ${rtpsuite_BINARY_DIR}/bin)
#install (TARGETS rtps DESTINATION ${RTPS_INSTALL_DIR})
#this should probably be done in a different way
INSTALL(CODE "FILE(MAKE_DIRECTORY ${RTPS_INSTALL_DIR}/shaders)")
install (DIRECTORY ${rtpslib_BINARY_DIR}/shaders/ DESTINATION ${RTPS_INSTALL_DIR}/shaders/)
#ENDIF(WIN32)
