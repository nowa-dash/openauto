cmake_minimum_required(VERSION 3.5.1)
project(openauto CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(base_directory ${CMAKE_CURRENT_SOURCE_DIR})
set(resources_directory ${base_directory}/assets)
set(sources_directory ${base_directory}/src)
set(include_directory ${base_directory}/include)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${base_directory}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${base_directory}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${base_directory}/bin)
set(EXECUTABLE_OUTPUT_PATH ${base_directory}/bin)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")
SET(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_INIT} -Wall -pedantic -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g -O3")

add_definitions(-DBOOST_ALL_DYN_LINK)

find_package(Boost REQUIRED COMPONENTS system log OPTIONAL_COMPONENTS unit_test_framework)
find_package(libusb-1.0 REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia REQUIRED)
find_package(rtaudio REQUIRED)
find_package(absl REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-app-1.0)
find_package(aasdk REQUIRED)

if(WIN32)
    set(WINSOCK2_LIBRARIES "ws2_32")
endif(WIN32)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${GST_INCLUDE_DIRS}
                    ${Boost_INCLUDE_DIRS}
                    ${LIBUSB_1_INCLUDE_DIRS}
                    ${RTAUDIO_INCLUDE_DIRS}
                    ${include_directory})
								
link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

set(common_include_directory ${include_directory}/f1x/openauto/Common)

set(autoapp_sources_directory ${sources_directory}/autoapp)
set(autoapp_include_directory ${include_directory}/f1x/openauto/autoapp)
file(GLOB_RECURSE autoapp_source_files ${autoapp_sources_directory}/*.ui ${autoapp_sources_directory}/*.cpp ${autoapp_include_directory}/*.hpp ${common_include_directory}/*.hpp ${resources_directory}/*.qrc)

add_executable(autoapp ${autoapp_source_files})

target_link_libraries(autoapp
                        ${Boost_LIBRARIES}
                        Qt6::Core
                        Qt6::Gui
                        Qt6::Widgets
                        Qt6::Multimedia
                        ${GST_LIBRARIES}
                        ${LIBUSB_1_LIBRARIES}
                        ${WINSOCK2_LIBRARIES}
                        ${RTAUDIO_LIBRARIES}
                        aasdk::aasdk
                        absl::log_internal_check_op
                        absl::status
                        absl::strings)

install(TARGETS autoapp RUNTIME DESTINATION bin)
