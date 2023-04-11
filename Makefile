################################################################################
################################ COMPILER & FLAGS ##############################
################################################################################

CC =		g++

CFLAGS	=	-Wall -Wextra -Werror -Wshadow -std=c++98 -O3

DEBUG =		-Wall -Wextra -Werror -g -std=c++98

VAL =		valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --track-fds=yes

################################################################################
################################### LIBRARIES ##################################
################################################################################

FT_INC	= 		-I ./include/
				
INCLUDES = 		$(FT_INC)

SRCDIR =		src/

OBJDIR =		obj/

################################################################################
################################## SRCS & SRCS #################################
################################################################################

SRCS	=	$(SRCDIR)Config.cpp			\
			$(SRCDIR)ConfigParser.cpp	\
			$(SRCDIR)Location.cpp		\
			$(SRCDIR)httpHeader.cpp		\
			$(SRCDIR)Server.cpp			\
			$(SRCDIR)ServerManager.cpp	\
			$(SRCDIR)Response.cpp		\
			$(SRCDIR)MIME.cpp			\
			$(SRCDIR)CGI.cpp			\
			$(SRCDIR)minilib.cpp		\
			$(SRCDIR)main.cpp			\

OBJS	=	$(SRCS:$(SRCDIR)%.cpp=$(OBJDIR)%.o)

################################################################################
#################################### PROGRAM ###################################
################################################################################

EXEC =		webserv

RUN =		./webserv

################################################################################
#################################### RULES #####################################
################################################################################

all:		$(EXEC)

$(EXEC):	$(OBJDIR) $(OBJS)
	$(CC) $(OBJS) $(INCLUDES) -o $(EXEC) $(CFLAGS)

debug:	$(OBJDIR) $(OBJS)
	$(CC) $(OBJS) $(INCLUDES) -o $(EXEC) $(DEBUG)

$(OBJDIR):
	mkdir -p $(@D)

$(OBJDIR)%.o: $(SRCDIR)%.cpp
	$(CC) $(CFLAGS) -c -g $< -o $@

clean:
	rm -fr $(OBJDIR)
fclean:		clean
	rm -f $(EXEC)

re:			fclean all

launch: fclean debug
	$(RUN)

test: debug
	$(VAL) $(RUN)

.PHONY: all debug clean fclean re launch test