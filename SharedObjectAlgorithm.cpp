
#include "AlgorithmRegistration.h"
#include "AutoPlayerAlgorithm.h"

#include <iostream>

__attribute__((constructor))
void constructor()
{
    volatile REGISTER_ALGORITHM(305261901);
}
