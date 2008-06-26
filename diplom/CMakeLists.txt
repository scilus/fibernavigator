file( GLOB _srcs "*.cpp" )

file( GLOB _srcs2 "GLSL/*.cpp" )
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

option( BUILD_OPTION_OPENMP "use openmp parallelization" OFF)

if(BUILD_OPTION_OPENMP)
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif(BUILD_OPTION_OPENMP)

INCLUDE( ${wxWidgets_USE_FILE} )
INCLUDE_DIRECTORIES( ${wxWidgets_INCLUDE_DIRS} )
ADD_DEFINITIONS( `wx-config --cppflags` )
add_executable( main MACOSX_BUNDLE ${_srcs} ${_srcs2} )
#target_link_libraries( main `wx-config --libs` )
target_link_libraries( main -lGL -lGLU -lGLEW )
target_link_libraries( main ${wxWidgets_LIBRARIES} )
target_link_libraries( main -lwx_gtk2u_gl-2.8 ) 
