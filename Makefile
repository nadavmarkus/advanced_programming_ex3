COMP = g++-5
OBJS = main.o TournamentManager.o AlgorithmRegistration.o
ALGORITHM_OBJS = Globals.o
EXEC = ex3
CPP_COMP_FLAG = -std=gnu++14 -g -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG -g
LINKING_LIBS = -ldl -lpthread
DEPS = *.h
OUTPUT_LIB = RPSPlayer_305261901.so
DUMMY_OPPONENT = RPSPlayer_123456789.so
SHARED_OBJECT_FLAGS = -fPIC -shared

rps_tournamet: $(OBJS) $(ALGORITHM_OBJS)
	$(COMP) -rdynamic -o $(EXEC) $(OBJS) $(ALGORITHM_OBJS) $(LINKING_LIBS) 
    
%.o: %.cpp $(DEPS)
	$(COMP) $(CPP_COMP_FLAG) -fPIC -c $*.cpp
    
rps_lib: SharedObjectAlgorithm.cpp $(DEPS) 
	$(COMP) $(CPP_COMP_FLAG) SharedObjectAlgorithm.cpp $(SHARED_OBJECT_FLAGS) $(ALGORITHM_OBJS) -o $(OUTPUT_LIB)
    
dummy_opponent: DummyOpponent.cpp $(DEPS) 
	$(COMP) $(CPP_COMP_FLAG) DummyOpponent.cpp $(SHARED_OBJECT_FLAGS) $(ALGORITHM_OBJS) -o $(DUMMY_OPPONENT)

clean:
	rm -f $(OBJS) $(EXEC) $(OUTPUT_LIB) $(ALGORITHM_OBJS) $(DUMMY_OPPONENT)