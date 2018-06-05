#include <cassert>
#include <string>
#include <iostream>

/* We resort to raw getopt since we don't have boost :( */
#include <getopt.h>
#include <stdlib.h>
#include "TournamentManager.h"

int main(int argc, char *const argv[])
{
    struct option options[] {
        {"threads", required_argument, nullptr, 0},
        {"path", required_argument, nullptr, 0},
        {nullptr, 0, nullptr, 0}
    };

    TournamentManager &tournament_manager = TournamentManager::getInstance();
    
    int longindex;
    while (-1 != getopt_long_only(argc, argv, "", options, &longindex)) {
        switch(longindex) {
            
            case 0:
                /* Thread count. */
                try {
                    size_t count = static_cast<size_t>(std::stoi(std::string(optarg)));
                    
                    if (0 == count) {
                        std::cerr << "The count of threads should be at least 1." << std::endl;
                        return -1;
                    }
                    
                    tournament_manager.setThreadCount(count);
                } catch (const std::exception &error) {
                    std::cerr << "Failed to parse the number of threads: " << optarg << std::endl;
                    return -1;
                }
                
                break;
                
            case 1:
                /* Set the path. */
                tournament_manager.setSODirectory(std::string(optarg));
                break;
            
            default:
                /* Should not happen. */
                assert(false);
                break;
        }
    }
    
    tournament_manager.run();
    
    return 0;
}


