cmake_minimum_required(VERSION 3.0)
project(numinspector)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

########################################
# Boost

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.49.0 REQUIRED program_options)

include_directories(${Boost_INCLUDE_DIR})
set(LINK_LIBS ${LINK_LIBS} ${Boost_LIBRARIES})

########################################
set(SOURCE_FILES num_inspect.cc)
add_executable(numinspector ${SOURCE_FILES})
target_link_libraries(numinspector PUBLIC ${LINK_LIBS})