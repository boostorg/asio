cmake_minimum_required( VERSION 3.22 )
project( boost_asio_http_server )

set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_BUILD_TYPE Debug )

set(Boost_NO_WARN_NEW_VERSIONS 1)
find_package( Boost 1.82 )

#message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")

file(GLOB BOOST_ASIO_HTTP_SERVER_SOURCES "./*.cpp" "./*.hpp")
add_executable( ${PROJECT_NAME}
    ${BOOST_ASIO_HTTP_SERVER_SOURCES}
)

target_include_directories( ${PROJECT_NAME}
    PUBLIC
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries( ${PROJECT_NAME}
    ${Boost_LIBRARIES}
)

