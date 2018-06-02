#include <memory>
#include <string>
#include <iostream>
#include <dlfcn.h>

/* Note: I don't use the filesystem header because it exists only from c++17 onwards. */
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "TournamentManager.h"

void TournamentManager::loadAllPlayers()
{
    
}

void TournamentManager::run()
{
    loadAllPlayers();
    
    if (player_count < 2) {
        //TODO: Implement me.
    }
    
    DIR *raw_so_dir = opendir(so_directory.c_str());
    
    if (NULL == raw_so_dir) {
        //TODO: Handle error.
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
}



