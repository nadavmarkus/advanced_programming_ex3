
#include "AlgorithmRegistration.h"
#include "DummyOpponent.h"

__attribute__((constructor))
void constructor()
{
    volatile REGISTER_ALGORITHM(123456789);
}
