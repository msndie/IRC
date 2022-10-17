cmake_minimum_required(VERSION 3.21)
project(irc)

set(CMAKE_CXX_STANDARD 98)

add_executable(irc srcs/main.cpp srcs/Server.cpp includes/Server.hpp srcs/User.cpp includes/User.hpp srcs/Message.cpp includes/Message.hpp includes/Responses.hpp srcs/utils/utils.cpp includes/utils.h srcs/ServerMessagesHandling.cpp srcs/utils/sendAll.cpp srcs/ServerUsersHandle.cpp srcs/Channel.cpp includes/Channel.hpp srcs/UserCommands.cpp srcs/ChannelCommands.cpp srcs/Configuration.cpp includes/Configuration.hpp srcs/OtherCommands.cpp srcs/OperatorCommands.cpp srcs/ServerUtils.cpp srcs/ServerInitUtils.cpp bot/srcs/Bot.cpp bot/includes/Bot.hpp)
