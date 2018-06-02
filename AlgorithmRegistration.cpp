#include "AlgorithmRegistration.h"

#include <functional>
#include <memory>

#include "TournamentManager.h"

AlgorithmRegistration::AlgorithmRegistration(std::string id,
                                             std::function<std::unique_ptr<PlayerAlgorithm>()> algorithm)
{
    TournamentManager::getInstance().onPlayerRegistration(id, algorithm);
}