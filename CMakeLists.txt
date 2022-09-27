cmake_minimum_required(VERSION 3.21)
project(irc)

set(CMAKE_CXX_STANDARD 98)

add_executable(irc main.cpp utils.cpp Server.cpp Server.hpp)
