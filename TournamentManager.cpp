#include <memory>
#include <string>
#include <iostream>
#include <condition_variable>
#include <deque>
#include <random>
#include <dlfcn.h>

/* Note: I don't use the filesystem header because it exists only from c++17 onwards. */
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "TournamentManager.h"

struct WorkItem
{
public:
    bool should_terminate;
    std::string player1_id;
    std::string player2_id;
};

void TournamentManager::loadAllPlayers()
{
    DIR *raw_so_dir = opendir(so_directory.c_str());
    
    if (NULL == raw_so_dir) {
        //TODO: Handle error.
        return;
    }
    
    struct dirent *dir_entry;
    
    while (NULL != (dir_entry = readdir(raw_so_dir)) {
        if (DT_REG == dir_entry.d_type) {
            /* Let's attempt to load the potential file. */
            void *algorithm_so = dlopen(dir_entry.d_name);
            
            if (NULL == algorithm_so) {
                // TODO: Handle error
            }
        }
    }
    
    (void) closedir(raw_so_dir);
}

void TournamentManager::workerThread()
{
    for (;;) {
        
    }
}

void TournamentManager::runMatchesAsynchronously()
{
    
    for (size_t i = 0; i < thread_count - 1; ++i) {
        threads.push_back(std::thread(workerThread));
    }
    
    
}

void TournamentManager::runMatchesSynchronously()
{
    
}

void TournamentManager::runMatches()
{
    if (thread_count > 1) {
        runMatchesAsynchronously();
    
    } else {
        runMatchesSynchronously();
    }
}

void TournamentManager::run()
{
    loadAllPlayers();
    
    if (player_count < 2) {
        //TODO: Handle error
        return;
    }
    
    runMatches();
}



