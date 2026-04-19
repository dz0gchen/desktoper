cmake_minimum_required(VERSION 3.16)
project(App)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

set(app_SRC
  src/main.cpp
  src/desktoper.cpp
  src/desktoper.h
)

add_executable(desktoper ${app_SRC})
target_link_libraries(desktoper Qt6::Widgets)
