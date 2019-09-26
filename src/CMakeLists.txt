if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Source and build directories cannot be the same. Go use the /build directory.")
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.70 REQUIRED COMPONENTS filesystem)
message(STATUS "Boost version: ${Boost_VERSION}")


find_package(OpenCV 4.1 REQUIRED)
message(STATUS "OpenCV Version: ${OpenCV_VERSION}")

find_path( AVCODEC_INCLUDE_DIR libavcodec/avcodec.h )
find_library( AVCODEC_LIBRARY avcodec )

find_path(AVFORMAT_INCLUDE_DIR libavformat/avformat.h)
find_library(AVFORMAT_LIBRARY avformat)

find_path(AVUTIL_INCLUDE_DIR libavutil/avutil.h)
find_library(AVUTIL_LIBRARY avutil)

find_path(AVDEVICE_INCLUDE_DIR libavdevice/avdevice.h)
find_library(AVDEVICE_LIBRARY avdevice)

find_path( SWSCALE_INCLUDE_DIR libswscale/swscale.h )
find_library( SWSCALE_LIBRARY libswscale.a )

SET(SRCS

    main.cpp
    VideoInfo.h
    VideoInfo.cpp
)

add_executable(videoinfo ${SRCS})

set_property(TARGET videoinfo PROPERTY CXX_STANDARD 17)
set_property(TARGET videoinfo PROPERTY CXX_STANDARD_REQUIRED TRUE)

target_include_directories(videoinfo PRIVATE ${Boost_INCLUDE_DIR} )
target_link_libraries(videoinfo PRIVATE Boost::filesystem)

target_link_libraries(videoinfo PRIVATE ${OpenCV_LIBS})

target_include_directories(videoinfo PRIVATE ${AVCODEC_INCLUDE_DIR} ${AVFORMAT_INCLUDE_DIR} ${AVUTIL_INCLUDE_DIR} ${AVDEVICE_INCLUDE_DIR} ${SWSCALE_INCLUDE_DIR})
target_link_libraries(videoinfo PRIVATE ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${AVDEVICE_LIBRARY} ${SWSCALE_LIBRARY})
