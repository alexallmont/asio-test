cmake_minimum_required(VERSION 3.3)

project(asio_test)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)

find_package(
  Boost
  REQUIRED
    system
)

add_executable(
  asio_test
    main.cpp
)

target_link_libraries(
  asio_test
    ${Boost_LIBRARIES}
)
