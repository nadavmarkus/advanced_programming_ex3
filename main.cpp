#include <getopt.h>
#include "TournamentManager.h"

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;
    TournamentManager &tournament_manager = TournamentManager::getInstance();
    tournament_manager.run();
    
    return 0;
}


