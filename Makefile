NAME= server
SRCS= srcs/webserv.cpp srcs/tools.cpp
OBJS= $(SRCS:.cpp=.o)
CXX=c++
CXXFLAGS= -g -Werror -Wextra -Wall -std=c++98


all : $(NAME)

$(NAME): $(OBJS)
	$(CXX) -g -fsanitize=address $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

	
