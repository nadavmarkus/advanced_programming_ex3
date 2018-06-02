#ifndef __TOURNAMENT_MANAGER_H_
#define __TOURNAMENT_MANAGER_H_

#include <memory>
#include <vector>
#include <functional>
#include <string>
#include <map>
#include <mutex>

#include <stdlib.h>

#include "PlayerAlgorithm.h"

using playerAlgorithmPtr = std::function<std::unique_ptr<PlayerAlgorithm>()>;

class TournamentManager
{
private:
    static constexpr size_t REQUIRED_GAMES = 30;
    
    std::map<std::string, playerAlgorithmPtr> id_to_algorithm;
    std::string so_directory;
    size_t thread_count;
    std::mutex id_to_play_count_mutex;
    std::map<std::string, size_t> id_to_play_count;
    size_t player_count;
    std::vector<std::thread> threads;
    /* 
     * The tournament manager will be a singleton. Therefore, we forbid
     * direct instantiation of it. We don't want to use only static variables due to static
     * variables instantiation fiascos, so we still use a single instance that has its access
     * serialized via the public getInstance method.
     */ 
    TournamentManager(): id_to_algorithm(),
                         so_directory("./"),
                         thread_count(4),
                         id_to_play_count(),
                         player_count(0) {}
    
    void loadAllPlayers();
    
    void runOneMatch();

public:
    static TournamentManager& getInstance()
    {
        static TournamentManager instance;
        
        return instance;
    }
    
    void onPlayerRegistration(std::string &id, playerAlgorithmPtr algorithm)
    {
        id_to_algorithm[id] = algorithm;
        player_count++;
    }
    
    void setSODirectory(const std::string &so_directory)
    { 
        this->so_directory = so_directory;
        if ('/' != this->so_directory[this->so_directory.size() - 1]) {
            this->so_directory += '/';
        }
    }
    
    void setThreadCount(size_t thread_count) { this->thread_count = thread_count; }
    
    void run();
};

#endif