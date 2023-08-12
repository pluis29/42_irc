P_SRC = src/
P_INCLUDE = include/
P_OBJ = ./obj/

F_SRC = main.cpp server.cpp utils.cpp

OBJ = $(addprefix $(P_OBJ), $(F_SRC:%.cpp=$(OBJDIR)%.o))

NAME = ircserv

INC = -I $(P_INCLUDE)
CFLAGS =  -std=c++98 -g
# CFLAGS = -Wall -Wextra -Werror -std=c++98 -g
P_GUARD = mkdir -p $(@D)
RM = rm -rf
CC = c++

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(INC) $(OBJ) -o $(NAME)

$(P_OBJ)%.o: $(P_SRC)%.cpp
	$(P_GUARD)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

run: all
	./$(NAME) .default.conf

val: all
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --undef-value-errors=yes --errors-for-leak-kinds=all --show-reachable=yes ./$(NAME) .default.conf

clean:
	$(RM) $(P_OBJ)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean  re run
