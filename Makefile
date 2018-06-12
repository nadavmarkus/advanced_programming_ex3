COMP = g++-5.3.0
OBJS = main.o TournamentManager.o AlgorithmRegistration.o
ALGORITHM_OBJS = Globals.o
EXEC = ex3
CPP_COMP_FLAG = -std=gnu++14 -g -Wall -Wextra \
-Werror -pedantic-errors -DNDEBUG -g
LINKING_LIBS = -ldl -lpthread
DEPS = *.h
OUTPUT_LIB = RPSPlayer_305261901.so
SHARED_OBJECT_FLAGS = -fPIC -shared

rps_tournament: $(OBJS) $(ALGORITHM_OBJS)
	$(COMP) -rdynamic -o $(EXEC) $(OBJS) $(ALGORITHM_OBJS) $(LINKING_LIBS) 
    
%.o: %.cpp $(DEPS)
	$(COMP) $(CPP_COMP_FLAG) -fPIC -c $*.cpp
    
rps_lib: SharedObjectAlgorithm.cpp $(DEPS) $(ALGORITHM_OBJS)
	$(COMP) $(CPP_COMP_FLAG) SharedObjectAlgorithm.cpp $(SHARED_OBJECT_FLAGS) $(ALGORITHM_OBJS) -o $(OUTPUT_LIB)
    
OPPONENTS := $(patsubst DummyOpponent%.cpp,RPSPlayer_%.so,$(wildcard DummyOpponent?*.cpp))

dummy_opponents: $(OPPONENTS)

RPSPlayer_%.so: DummyOpponent%.cpp $(ALGORITHM_OBJS)
	$(COMP) $(CPP_COMP_FLAG) $< $(SHARED_OBJECT_FLAGS) $(ALGORITHM_OBJS) -o $@

clean:
	rm -f $(OBJS) $(EXEC) $(OUTPUT_LIB) $(ALGORITHM_OBJS) $(DUMMY_OPPONENT)
	rm -f $(wildcard RPSPlayer_*.so)
	rm -f $(wildcard DummyOpponent?*.h)
	rm -f $(wildcard DummyOpponent?*.cpp)