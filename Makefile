P_SRC = src/
P_INCLUDE = includes/
P_OBJ = obj/

F_SRC = main.cpp server.cpp utils.cpp user.cpp command.cpp

OBJ = $(addprefix $(P_OBJ), $(F_SRC:%.cpp=%.o))

NAME = ircserv

INC = -I $(P_INCLUDE)
CFLAGS =  -std=c++98 -g
# CFLAGS = -Wall -Wextra -Werror -std=c++98 -g
RM = rm -rf
CC = c++

.PHONY: all clean fclean  re run

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(INC) $(OBJ) -o $(NAME)

$(P_OBJ)%.o: $(P_SRC)%.cpp
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

run: all
	./$(NAME) 6667 123

val: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --undef-value-errors=yes --errors-for-leak-kinds=all --show-reachable=yes ./$(NAME) 6667 123

clean:
	$(RM) $(P_OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all
