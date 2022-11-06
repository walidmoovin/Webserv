NAME= server
SRCS= srcs/webserv.cpp srcs/tools.cpp
OBJS= $(SRCS:.cpp=.o)
CXX=c++
CXXFLAGS= -Werror -Wextra -Wall -std=c++98


all : $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re

	
