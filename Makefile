NAME = ft_ping
CC = cc
CFLAGS = -Wall -Wextra -Werror

SRC = ./srcs/
INCS_DIR = ./incs

FILES = $(SRC)main.c \
		$(SRC)resolve_hostname.c

OBJ_FILES = $(FILES:.c=.o)
DEP_FILES = $(OBJ_FILES:.o=.d)

all : $(NAME)

%.o: %.c
	$(CC) $(CFLAGS) -I $(INCS_DIR) -MMD -MP -c -o $@ $<

$(NAME) : $(OBJ_FILES)
	$(CC) $(CFLAGS) $^ -o $@ -I$(INCS_DIR)

clean :
	rm -f $(OBJ_FILES) $(DEP_FILES)

fclean : clean
	rm -f $(NAME)

re : fclean
	$(MAKE) all

-include $(DEP_FILES)

.PHONY : all clean fclean re
