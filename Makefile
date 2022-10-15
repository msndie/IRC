NAME			=	ircserv

SRC_FILES		=	main.cpp \
					Channel.cpp \
					ChannelCommands.cpp \
					Configuration.cpp \
					Message.cpp \
					OperatorCommands.cpp \
					OtherCommands.cpp \
					Server.cpp \
					ServerInitUtils.cpp \
					ServerMessagesHandling.cpp \
					ServerUsersHandle.cpp \
					ServerUtils.cpp \
					User.cpp \
					UserCommands.cpp \
					utils/sendAll.cpp \
					utils/utils.cpp

SOURCES_FOLDER	=	srcs/

OBJECTS_FOLDER	=	objs/

INCLUDES_FOLDER	=	include/

SOURCES			=	$(addprefix $(SOURCES_FOLDER), $(SRC_FILES))

OBJECTS			=	$(addprefix $(OBJECTS_FOLDER), $(SRC_FILES:.cpp=.o))

all: $(NAME)

$(OBJECTS_FOLDER):
	mkdir -p objs/utils

$(OBJECTS_FOLDER)%.o: $(SOURCES_FOLDER)%.cpp
	c++ -Wall -Wextra -Werror -std=c++98 -c $< -o $@ -I $(INCLUDES_FOLDER)

$(NAME): $(OBJECTS_FOLDER) $(OBJECTS)
	c++ $(OBJECTS) -o $(NAME)

clean:
	rm -rf $(OBJECTS_FOLDER)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re