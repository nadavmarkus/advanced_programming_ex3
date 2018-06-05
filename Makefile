COMP = g++-5
OBJS = main.o TournamentManager.o AlgorithmRegistration.o
EXEC = ex3
CPP_COMP_FLAG = -std=gnu++14 -g -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG -g
LINKING_LIBS = -ldl -lpthread
DEPS = *.h
OUTPUT_LIB = RPSPlayer_305261901.so
DUMMY_OPPONENT = RPSPlayer_123456789.so
SHARED_OBJECT_FLAGS = -fPIC -shared

rps_tournamet: $(OBJS)
	$(COMP) -rdynamic -o $(EXEC) $(OBJS) $(LINKING_LIBS) 
    
%.o: %.cpp $(DEPS)
	$(COMP) $(CPP_COMP_FLAG) -c $*.cpp
    
rps_lib: SharedObjectAlgorithm.cpp $(DEPS) 
	$(COMP) $(CPP_COMP_FLAG) SharedObjectAlgorithm.cpp $(SHARED_OBJECT_FLAGS) -o $(OUTPUT_LIB)
    
dummy_opponent: DummyOpponent.cpp $(DEPS) 
	$(COMP) $(CPP_COMP_FLAG) DummyOpponent.cpp $(SHARED_OBJECT_FLAGS) -o $(DUMMY_OPPONENT)
clean:
	rm -f $(OBJS) $(EXEC) $(OUTPUT_LIB)