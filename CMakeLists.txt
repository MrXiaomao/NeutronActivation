#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(Activation)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

find_package(HDF5 REQUIRED COMPONENTS C CXX)
set(HDF5_INCLUDE_DIRS /usr/local/hdf5/include)

include_directories(${HDF5_INCLUDE_DIRS})

message(STATUS "INCLUDE LOCATION:" ${HDF5_INCLUDE_DIRS})

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#


include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_SOURCE_DIR}/HDF5/Hdf5ReadValue.cc ${PROJECT_SOURCE_DIR}/HDF5/Hdf5WriteValue.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh ${PROJECT_SOURCE_DIR}/HDF5/Hdf5Function.h)
#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(Activation Activation.cc ${sources} ${headers})
target_link_libraries(Activation ${Geant4_LIBRARIES} -lstdc++fs ${HDF5_CXX_LIBRARIES} ${HDF5_LIBRARYES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build Hadr06. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(Activation_SCRIPTS
    loop.mac
    detector.mac
    envHadronic.csh
    envHadronic.sh
    run.mac
    vis.mac
    init_vis.mac
  )

foreach(_script ${Activation_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS Activation DESTINATION bin)

