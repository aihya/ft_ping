
NAME = ft_ping

SRC_PATH = ./src
OBJ_PATH = ./obj

SRC_FILES = main.c

OBJ_FILES = $(SRC_FILES:.c=.o)

SRC = $(addprefix $(SRC_PATH)/,$(SRC_FILES))
OBJ = $(addprefix $(OBJ_PATH)/,$(OBJ_FILES))

DEPS = ./inc/ft_ping.h

INC = -Iinc -Ilibft/include

CC = gcc

CFLAGS = #-Wall -Werror -Wextra

all : libft_all $(NAME)

$(OBJ_PATH)/%.o : $(SRC_PATH)/%.c $(DEPS)
	@mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) $(INC) -Llibft -lft -o $@ -c $<

libft_all :
	make -C libft

$(NAME) : $(OBJ)
	gcc $(OBJ) -Llibft -lft -o $@

clean :
	make -C libft clean
	/bin/rm -rf $(OBJ_PATH) 2> /dev/null

fclean : clean
	make -C libft fclean
	/bin/rm -f $(NAME)

re : fclean all

