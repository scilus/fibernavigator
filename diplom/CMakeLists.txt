cmake_minimum_required(VERSION 2.4)

file( GLOB _srcs "*.cpp" )

file( GLOB _srcs2 "GLSL/*.cpp" )
file( GLOB _srcs3 "Fantom/*.cpp" )
file( GLOB _srcs4 "IsoSurface/*.cpp" )
file( GLOB _srcs5 "lic/*.cpp" )
PROJECT( ralph )
# # at least it is not defined when compiling on my mac
# #  add_definitions( -D__WXMAC__ )
# #  add_definitions( -D__WXCOCOA__ )
  set( wxWidgetsR_USE_LIBS base core gl )
  set( WXWIDGETS_USE_GL TRUE )
  find_package( wxWidgets REQUIRED )
  #set( wxWidgets_LIBRARIES ${wxWidgets_LIBRARIES} wx_mac_gl-2.8 )
  #LIST( SRC APPEND ${_srcs})
  #LIST( SRC APPEND ${_srcs2})
  #LIST( SRC APPEND ${_srcs3})
  #LIST( SRC APPEND ${_srcs4})
  #LIST( SRC APPEND ${_srcs5})

option( BUILD_OPTION_OPENMP "use openmp parallelization" OFF)

if(BUILD_OPTION_OPENMP)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif(BUILD_OPTION_OPENMP)

find_package( OpenGL REQUIRED )

find_path( GLEW_INCLUDE_DIR glew.h )
find_path( GLEW_LIBRARY libglew.so libGLEW.dylib )
link_directories( ${GLEW_DIR} )

INCLUDE_DIRECTORIES( ${GLEW_INCLUDE_DIR} )

INCLUDE( ${wxWidgets_USE_FILE} )
INCLUDE_DIRECTORIES( ${wxWidgets_INCLUDE_DIRS} )
ADD_DEFINITIONS( `wx-config --cppflags` )
add_executable( main MACOSX_BUNDLE ${_srcs} ${_srcs2} ${_srcs3} ${_srcs4} ${_srcs5})
#target_link_libraries( main `${wxWidgets_CONFIG_EXECUTABLE} --libs` )
#target_link_libraries( main -lGL -lGLU -lGLEW )
target_link_libraries( main ${wxWidgets_LIBRARIES} )
#target_link_libraries( main -lwx_gtk2u_gl-2.8 ) 

if(APPLE)
    target_link_libraries( main "-framework OpenGL" GLEW wx_macud_gl-2.8 )
	set( _BundleName "Fiber Navigator" )
	set( _BundleInfo "A MRI visualization tool")
	set( _BundleIcon "main.icns" )
	set_target_properties(main
	  PROPERTIES
	  MACOSX_BUNDLE_BUNDLE_NAME ${_BundleName}
	  MACOSX_BUNDLE_INFO_STRING ${_BundleInfo}
	  MACOSX_BUNDLE_ICON_FILE ${_BundleIcon}
	  MACOSX_BUNDLE_COPYRIGHT "(c) 2008, Ralph and Mario"
	  )
else(APPLE)
    target_link_libraries( main -L../diplom/glew/lib GLEW wx_gtk2u_gl-2.8 )
endif(APPLE)
