NAME= server
SRCS= srcs/webserv.cpp srcs/tools.cpp srcs/load/Env.cpp srcs/load/Server.cpp \
	  srcs/load/Socket.cpp srcs/load/Route.cpp   \
	  srcs/json/Nodes.cpp srcs/json/Token.cpp srcs/json/Parser.cpp
OBJS= $(SRCS:.cpp=.o)
CXX=c++
CXXFLAGS= -g -I includes -Werror -Wextra -Wall -std=c++98


all : $(NAME)

$(NAME): $(OBJS)
	$(CXX) -g  $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

	
