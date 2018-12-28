#cmake_minimum_required(VERSION 3.10)
#message(STATUS "cmake version: ${CMAKE_VERSION}")
if (NOT PROJECT_NAME)
	set(IS_SUBPROJECT false)
	project(boost_asio LANGUAGES CXX)
else()
	set(IS_SUBPROJECT true)
endif()

if(CMAKE_CXX_STANDARD LESS 14)
  # c++14 required
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag(-std=c++11 HAS_CXX11_FLAG)
  check_cxx_compiler_flag(-std=c++14 HAS_CXX14_FLAG)
  check_cxx_compiler_flag(-std=c++17 HAS_CXX17_FLAG)
  
  if(HAS_CXX17_FLAG)
    set(CMAKE_CXX_STANDARD 17)
  elseif(HAS_CXX14_FLAG)
    set(CMAKE_CXX_STANDARD 14)
  else()
    message(FATAL_ERROR "boost_asio requires c++14")
  endif()
endif()

if(NOT (DEFINED CMAKE_CXX_STANDARD) OR CMAKE_CXX_STANDARD STREQUAL "" OR CMAKE_CXX_STANDARD LESS 14)
    message(FATAL_ERROR "boost_asio requires c++14")
endif()

file( GLOB_RECURSE boost_asio_interface_srcs
	"${CMAKE_CURRENT_SOURCE_DIR}/include/*.*"
)

add_library( boost_asio INTERFACE )
target_include_directories( boost_asio INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" )
target_compile_definitions( boost_asio INTERFACE BOOST_USE_WINFIB )
target_link_libraries( boost_asio INTERFACE boost_config boost_assert boost_smart_ptr )
