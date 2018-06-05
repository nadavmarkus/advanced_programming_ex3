
#include "AlgorithmRegistration.h"
#include "DummyOpponent.h"

__attribute__((constructor))
void constructor()
{
    REGISTER_ALGORITHM(123456789);
}
