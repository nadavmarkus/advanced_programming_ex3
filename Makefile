COMP = g++-5
OBJS = main.o TournamentManager.o
EXEC = ex3
CPP_COMP_FLAG = -std=gnu++14 -g -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG -g
LINKING_LIBS = -ldl -lpthread
DEPS = *.h

$(EXEC): $(OBJS)
	$(COMP) -o $@ $(OBJS) $(LINKING_LIBS) 
    
%.o: %.cpp $(DEPS)
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp

clean:
	rm -f $(OBJS) $(EXEC)