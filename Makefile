NAME= webserv
SRCS= srcs/webserv.cpp srcs/tools.cpp srcs/debug.cpp \
	  srcs/load/Env.cpp srcs/load/Server.cpp \
	  srcs/load/Route.cpp   srcs/sock/Master.cpp srcs/sock/Client.cpp \
	  srcs/json/Nodes.cpp srcs/json/Token.cpp srcs/json/Parser.cpp
OBJS= $(SRCS:.cpp=.o)
CXX=c++
CXXFLAGS= -g -I includes -Werror -Wextra -Wall -std=c++98


all : $(NAME)

$(NAME): $(OBJS)
	$(CXX) -g -fsanitize=address $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

	
