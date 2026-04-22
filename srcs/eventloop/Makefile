NAME = server
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic
SRCS = main.cpp Server.cpp
OBJS = $(SRCS:.cpp=.o)

$(NAME):$(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

all: $(NAME)

clean: 
	rm -f $(OBJS)

fclean:	clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
