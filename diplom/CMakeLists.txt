cmake_minimum_required(VERSION 2.4)
PROJECT( FiberNavigator )

file( GLOB _srcs "*.cpp" )

file( GLOB _srcs2 "GLSL/*.cpp" )
file( GLOB _srcs3 "Fantom/*.cpp" )
file( GLOB _srcs4 "IsoSurface/*.cpp" )
file( GLOB _srcs5 "lic/*.cpp" )
file( GLOB _srcs6 "nifti/*.c" )

if(APPLE)
set( wxWidgets_USE_LIBS base core gl xml aui )
endif(APPLE)
set( WXWIDGETS_USE_GL TRUE )
find_package( wxWidgets REQUIRED )

option( BUILD_OPTION_OPENMP "use openmp parallelization" OFF)

if(BUILD_OPTION_OPENMP)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif(BUILD_OPTION_OPENMP)

find_package( OpenGL REQUIRED )

#find_path( GLEW_INCLUDE_DIR glew.h )
#find_library( GLEW_LIBRARY libglew.so libGLEW.dylib )
#link_directories( ${GLEW_DIR} )

#INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_DIR} )

INCLUDE( ${wxWidgets_USE_FILE} )
  INCLUDE_DIRECTORIES( . ${wxWidgets_INCLUDE_DIRS} )
ADD_DEFINITIONS( `${wxWidgets_CONFIG_EXECUTABLE} gl --cppflags` )

if(APPLE)
    set( target "FiberNavigator")
    include_directories( "~/glew/glew-10.5/include" )
    link_directories( "~/glew/glew-10.5/lib" )
    add_executable( ${target} MACOSX_BUNDLE ${_srcs} ${_srcs2} ${_srcs3} ${_srcs4} ${_srcs5} ${_srcs6})
    target_link_libraries( ${target} "-framework OpenGL" GLEW )
    set( _BundleName "Fiber Navigator" )
    set( _BundleInfo "A MRI visualization tool")
    set( _BundleIcon "main.icns" )
    set_target_properties( ${target}
       PROPERTIES
	  MACOSX_BUNDLE_BUNDLE_NAME ${_BundleName}
	  MACOSX_BUNDLE_INFO_STRING ${_BundleInfo}
	  MACOSX_BUNDLE_ICON_FILE ${_BundleIcon}
	  MACOSX_BUNDLE_COPYRIGHT "(c) 2008, Ralph and Mario"
	  )
else(APPLE)
    set( target "main")
    add_executable( ${target} ${_srcs} ${_srcs2} ${_srcs3} ${_srcs4} ${_srcs5} ${_srcs6})
	target_link_libraries( main -L../diplom/glew/lib GLEW wx_gtk2u_gl-2.8 )
	#target_link_libraries( ${target} -lGL -lGLU -lGLEW )
endif(APPLE)

target_link_libraries( ${target} ${wxWidgets_LIBRARIES} )

