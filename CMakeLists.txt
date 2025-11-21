cmake_minimum_required(VERSION 4.0)

project(webgrab LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL)

if (NOT CURL_FOUND)
    message(STATUS "libcurl not found, fetching and building it")

    include(FetchContent)

    FetchContent_Declare(
        curl
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_17_0
    )

    FetchContent_MakeAvailable(curl)

    set(CURL_LIBRARIES libcurl)
    set(CURL_INCLUDE_DIRS ${curl_SOURCE_DIR}/include)
endif()

set(SOURCES
    main.cpp
    DownloadService.cpp
)

set(HEADERS
    DownloadService.hpp
)

add_executable(webgrab ${SOURCES} ${HEADERS})

target_include_directories(webgrab PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(webgrab PRIVATE ${CURL_LIBRARIES})