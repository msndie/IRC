NAME			=	ircserv

BOT_NAME		=	joker

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

BOT_FILES		=	main.cpp \
					Bot.cpp \
					Configuration.cpp \
					Message.cpp \
					utils/sendAll.cpp \
					utils/utils.cpp

SOURCES_FOLDER	=	srcs/

BOT_FOLDER		=	bot/srcs/

OBJECTS_FOLDER	=	objs/

BOT_OBJS_FOLDER	=	bot/objs/

INCLUDES_FOLDER	=	includes/

BOT_INCLUDES	=	bot/includes/

SOURCES			=	$(addprefix $(SOURCES_FOLDER), $(SRC_FILES))

BOT_SOURCES		=	$(addprefix $(BOT_FOLDER), $(BOT_FILES))

OBJECTS			=	$(addprefix $(OBJECTS_FOLDER), $(SRC_FILES:.cpp=.o))

BOT_OBJECTS		=	$(addprefix $(BOT_OBJS_FOLDER), $(BOT_FILES:.cpp=.o))

all: $(NAME)

$(OBJECTS_FOLDER):
	mkdir -p objs/utils

$(OBJECTS_FOLDER)%.o: $(SOURCES_FOLDER)%.cpp
	c++ -Wall -Wextra -Werror -std=c++98 -c $< -o $@ -I $(INCLUDES_FOLDER)

$(NAME): $(OBJECTS_FOLDER) $(OBJECTS)
	c++ $(OBJECTS) -o $(NAME)

bonus: $(BOT_NAME)

$(BOT_OBJS_FOLDER):
	mkdir -p bot/objs/utils

$(BOT_OBJS_FOLDER)%.o: $(BOT_FOLDER)%.cpp
	c++ -Wall -Wextra -Werror -std=c++98 -c $< -o $@ -I $(BOT_INCLUDES)

$(BOT_NAME): $(BOT_OBJS_FOLDER) $(BOT_OBJECTS)
	c++ $(BOT_OBJECTS) -o $(BOT_NAME)

clean:
	rm -rf $(OBJECTS_FOLDER)
	rm -rf $(BOT_OBJS_FOLDER)

fclean: clean
	rm -f $(NAME)
	rm -f $(BOT_NAME)

re: fclean all

.PHONY: all clean fclean re