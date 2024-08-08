cmake_minimum_required(VERSION 3.14)

if (WIN32)
    project(TestCoin LANGUAGES CXX)
elseif(UNIX)
    project(TestCoin)
endif()

set(CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo" CACHE STRING "" FORCE)

#======================= INCLUSION OF Qt =======================#
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH $ENV{QTDIR} "I:/dev/coin/libs/Coin3D/lib/cmake")
find_package(Qt6Core REQUIRED)
find_package(Qt6Widgets REQUIRED)
find_package(Qt6OpenGL REQUIRED)
find_package(Qt6OpenGLWidgets REQUIRED)
#find_package(coin PATHS REQUIRED)
find_package(quarter PATHS REQUIRED)

#=================== INCLUSION OF Project Files ====================#
set(FORMS_DIR "${CMAKE_SOURCE_DIR}/forms")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include" "${COINDIR}/include")
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/src")

#include_directories(${FORMS_DIR})
include_directories(${INCLUDE_DIR})
include_directories(${SOURCE_DIR})
link_directories("${COINDIR}/lib")

file(GLOB_RECURSE SOURCES
    "${FORMS_DIR}/*.ui"
    "${FORMS_DIR}/*.qrc"
    "${INCLUDE_DIR}/*.h"
    "${SOURCE_DIR}/*.cpp"
)

#=================== SETUP EXECTUABLE ====================#
# Enable debug logging on RELWITHDEBINFO configuration
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS
    $<$<CONFIG:RELWITHDEBINFO>:QT_MESSAGELOGCONTEXT>
)

# Add the forms directory to the AUTOUIC search paths
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_AUTOUIC_SEARCH_PATHS} ${FORMS_DIR})

# Add the executable
if (WIN32) 
    add_executable(TestCoin WIN32 ${SOURCES})
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
elseif(UNIX)
    add_executable(TestCoin ${SOURCES})
endif()

# Add preprocessor definition
add_compile_definitions(QUARTER_DLL)

# Add the target includes for TestCoin 
#target_include_directories(TestCoin PRIVATE ${FORMS_DIR})
target_include_directories(TestCoin PRIVATE ${INCLUDE_DIR})
target_include_directories(TestCoin PRIVATE ${SOURCE_DIR})

#===================== LINKING LIBRARIES =======================#
target_link_libraries(TestCoin Qt6::Widgets Qt6::OpenGL Qt6::OpenGLWidgets Quarter::Quarter )
#target_link_libraries(TestCoin Qt6::OpenGL Coin::Coin Coin4d.lib)
