# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kizuna <kizuna@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/12/19 00:00:00 by student           #+#    #+#              #
#    Updated: 2025/05/24 17:27:55 by kizuna           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = philo

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread
RM = rm -f

SRCDIR = src
INCDIR = include
OBJDIR = obj

SOURCES = main.c \
          input.c \
          utils.c \
          init.c \
          threads.c \
          actions.c \
          monitor.c \
          utils2.c

SRCS = $(addprefix $(SRCDIR)/, $(SOURCES))
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

# Colors for output
GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
NC = \033[0m # No Color

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJECTS)
	@echo "$(YELLOW)Linking $(NAME)...$(NC)"
	@$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)
	@echo "$(GREEN)$(NAME) created successfully!$(NC)"

$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "$(YELLOW)Compiling $<...$(NC)"
	@$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@echo "$(RED)Removing object files...$(NC)"
	@$(RM) -r $(OBJDIR)

fclean: clean
	@echo "$(RED)Removing $(NAME)...$(NC)"
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re
