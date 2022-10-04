cmake_minimum_required(VERSION 3.21)
project(irc)

set(CMAKE_CXX_STANDARD 98)

add_executable(irc main.cpp Server.cpp Server.hpp User.cpp User.hpp Message.cpp Message.hpp Responses.hpp utils/utils.cpp utils/utils.h ServerMessagesHanling.cpp utils/sendAll.cpp ServerUsersHandle.cpp Channel.cpp Channel.hpp UserCommands.cpp)
