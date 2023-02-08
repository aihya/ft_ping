
NAME = ft_ping

SRC_DIR = ./src
OBJ_DIR = ./obj

SRC_FILES =	main_2.c \
			send.c \
			receive.c \
			print.c \
			utils.c \
			errors.c \
			info.c \
			socket.c \

OBJ_FILES = $(SRC_FILES:.c=.o)

SRC = $(addprefix $(SRC_DIR)/,$(SRC_FILES))
OBJ = $(addprefix $(OBJ_DIR)/,$(OBJ_FILES))

DEPS = ./inc/ft_ping.h

INC = ./inc

CC = gcc

CFLAGS = #-Wall \
# 	 -Werror \
# 	 -Wextra


$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c $(DEPS)
	mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC) -o $@ -c $<


$(NAME) : $(OBJ)
	$(CC) $(CFLAGS) $(INC) $< -o $@


all : $(NAME)


clean :
	rm -rf $(OBJ_DIR)


fclean : clean
	rm -rf $(NAME)


re : fclean all
