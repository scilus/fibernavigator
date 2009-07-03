CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
PROJECT( FiberNavigator )

# Include local CMake modules
LIST( APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../tools/CMakeModules )

# First of all find out which platform we are on and set our own platform variable
SET( FIBERNAVIGATOR_PLATFORM_LINUX "Linux" )
SET( FIBERNAVIGATOR_PLATFORM_WINDOWS "Windows" )
IF( CMAKE_HOST_SYSTEM MATCHES Linux )
  SET( FIBERNAVIGATOR_PLATFORM
       ${FIBERNAVIGATOR_PLATFORM_LINUX}
       CACHE STRING "The platform we are building on." )
ENDIF( CMAKE_HOST_SYSTEM MATCHES Linux )
IF( CMAKE_HOST_SYSTEM MATCHES Windows )
  SET( FIBERNAVIGATOR_PLATFORM
    ${FIBERNAVIGATOR_PLATFORM_WINDOWS}
    CACHE STRING "The platform we are building on." )
ENDIF( CMAKE_HOST_SYSTEM MATCHES Windows )

# Platform dependencies
IF( FIBERNAVIGATOR_PLATFORM MATCHES ${FIBERNAVIGATOR_PLATFORM_WINDOWS} )
  SET( LIBRARY_OUTPUT_PATH
       ${PROJECT_BINARY_DIR}/bin
       CACHE PATH "The libraries will be put into this directory.")
ELSE( FIBERNAVIGATOR_PLATFORM MATCHES ${FIBERNAVIGATOR_PLATFORM_WINDOWS} )
  SET( LIBRARY_OUTPUT_PATH
       ${PROJECT_BINARY_DIR}/lib
       CACHE PATH "The libraries will be put into this directory.")
ENDIF( FIBERNAVIGATOR_PLATFORM MATCHES ${FIBERNAVIGATOR_PLATFORM_WINDOWS} )
MARK_AS_ADVANCED( LIBRARY_OUTPUT_PATH )
SET( EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin CACHE PATH "The executables will be put into this directory.")
MARK_AS_ADVANCED( EXECUTABLE_OUTPUT_PATH )

# Set default build type
IF ( NOT CMAKE_BUILD_TYPE )
    SET( CMAKE_BUILD_TYPE Release CACHE STRING"Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel." FORCE )
    MARK_AS_ADVANCED( CMAKE_BUILD_TYPE )
ENDIF ( NOT CMAKE_BUILD_TYPE )

# Package dependencies:
FIND_PACKAGE( GLEW REQUIRED )
FIND_PACKAGE( wxWidgets REQUIRED )
SET( WXWIDGETS_USE_GL TRUE )

#Cg
find_path( NVIDIA_CG_INCLUDE_DIR cg.h /usr/include/Cg )
find_library( NVIDIA_CG_LIBRARIES Cg )
find_library( NVIDIA_CGGL_LIBRARIES CgGL )
IF( NVIDIA_CG_INCLUDE_DIR AND NVIDIA_CG_LIBRARIES AND NVIDIA_CGGL_LIBRARIES )
    SET( NVIDIA_CG_CONFIGURED TRUE )
ENDIF( NVIDIA_CG_INCLUDE_DIR AND NVIDIA_CG_LIBRARIES AND NVIDIA_CGGL_LIBRARIES)

# Includes:
INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_PATH} )
INCLUDE( ${wxWidgets_USE_FILE} )
INCLUDE_DIRECTORIES( . ${wxWidgets_INCLUDE_DIRS} )
ADD_DEFINITIONS( `${wxWidgets_CONFIG_EXECUTABLE} --cppflags` )

#hiding this for now
MARK_AS_ADVANCED( wxWidgets_wxrc_EXECUTABLE )
MARK_AS_ADVANCED( wxWidgets_USE_DEBUG )

# Global compiler flags:
#-----------------------
# -Wno-long-long    since on Ubuntu 8.10 it won't compile without it
# -ansi             force ISO-C++98 compliance (not GNU++98)
SET( CMAKE_CXX_FLAGS "-Wall -ansi -Wno-long-long" )
SET( CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG -O0" )
SET( CMAKE_CXX_FLAGS_RELEASE "-O3" )
SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO "-g -DDEBUG -O2" )

SET( target "fibernavigator")

file( GLOB GUI_SRC "gui/*.cpp" )
file( GLOB GFX_SRC "gfx/*.cpp" )
file( GLOB DS_SRC "dataset/*.cpp" )
file( GLOB FANTOM_SRC "misc/Fantom/*.cpp" )
file( GLOB ISO_SRC "misc/IsoSurface/*.cpp" )
file( GLOB LIC_SRC "misc/lic/*.cpp" )
file( GLOB NIFTI_SRC "misc/nifti/*.c" )
IF( NVIDIA_CG_CONFIGURED )
    file( GLOB GLYPHS_SRC "gfx/ImplicitSurfaceGlyphs/*.cpp" )
    add_definitions( -DCG_GLYPHS )
    include_directories( ${NVIDIA_CG_INCLUDE_DIR} ) 
ENDIF( NVIDIA_CG_CONFIGURED )


ADD_EXECUTABLE( ${target} main.cpp ${DS_SRC} ${GFX_SRC} ${GUI_SRC} ${FANTOM_SRC} ${ISO_SRC} ${LIC_SRC} ${NIFTI_SRC} ${GLYPHS_SRC} )

TARGET_LINK_LIBRARIES( ${target} ${GLEW_LIBRARY} ${wxWidgets_LIBRARIES} wx_gtk2u_gl-2.8 ${NVIDIA_CG_LIBRARIES} ${NVIDIA_CGGL_LIBRARIES} )

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/GLSL ${CMAKE_BINARY_DIR}/bin/GLSL)
