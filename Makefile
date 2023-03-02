################################################################################
################################ COMPILER & FLAGS ##############################
################################################################################

CC =		g++

CFLAGS	=	-Wall -Wextra -Werror -Wshadow -std=c++98

DEBUG =		-Wall -Wextra -Werror -g -std=c++98 

VAL =		valgrind -s --leak-check=full --show-leak-kinds=all

################################################################################
################################### LIBRARIES ##################################
################################################################################

FT_INC	= 		-I ./include/
				
INCLUDES = 		$(FT_INC)

SRCDIR =		./src/

################################################################################
################################## SRCS & SRCS #################################
################################################################################

SRCS	=	$(SRCDIR)Config.cpp			\
			$(SRCDIR)ConfigParser.cpp	\
			$(SRCDIR)Location.cpp		\
			$(SRCDIR)CGI.cpp			\
			$(SRCDIR)httpHeader.cpp		\
			$(SRCDIR)Server.cpp			\
			$(SRCDIR)simple_server.cpp	\
			$(SRCDIR)minilib.cpp	\

################################################################################
#################################### PROGRAM ###################################
################################################################################

EXEC =		webserv

RUN =		./webserv

################################################################################
#################################### RULES #####################################
################################################################################

all:		$(EXEC)

$(EXEC):	$(SRCS)
		$(CC) $(SRCS) $(INCLUDES) -o $(EXEC) $(CFLAGS)

debug:	$(SRCS)
		$(CC) $(SRCS) $(INCLUDES) -o $(EXEC) $(DEBUG)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -g $(SRCS)

clean:
		rm -f $(EXEC)

fclean:		clean
		rm -f $(EXEC)

re:			fclean all

test: debug
	$(VAL) $(RUN)

.PHONY: all debug clean fclean re test