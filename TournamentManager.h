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
#include "BlockingQueue.h"

using playerAlgorithmPtr = std::function<std::unique_ptr<PlayerAlgorithm>()>;

/* We define WorkItem here although it is not part of the actual interface since it is needed for BlockingQueue */
struct WorkItem
{
public:
    bool should_terminate;
    const std::string player1_id;
    const std::string player2_id;
    
    WorkItem(const std::string &id1, const std::string &id2) :should_terminate(false), player1_id(id1), player2_id(id2) {}
    WorkItem(): should_terminate(false), player1_id(), player2_id() {}
    WorkItem(bool should_terminate, const std::string &id1, const std::string &id2): should_terminate(should_terminate),
                                                                         player1_id(id1),
                                                                         player2_id(id2) {}
    WorkItem(bool should_terminate): WorkItem(should_terminate, std::string(), std::string()) {}
};

class TournamentManager
{
private:
    static constexpr size_t REQUIRED_GAMES = 30;
    
    std::map<std::string, playerAlgorithmPtr> id_to_algorithm;
    std::string so_directory;
    size_t thread_count;
    
    std::mutex global_stats_mutex;
    std::map<std::string, size_t> id_to_play_count;
    std::map<std::string, size_t> id_to_points;
    
    size_t player_count;
    BlockingQueue<WorkItem> work_queue;
    /* 
     * The tournament manager will be a singleton. Therefore, we forbid
     * direct instantiation of it. We don't want to use only static variables due to static
     * variables instantiation fiascos, so we still use a single instance that has its access
     * serialized via the public getInstance method.
     */
    //TODO: Complete constructor
    TournamentManager(): id_to_algorithm(),
                         so_directory("./"),
                         thread_count(4),
                         id_to_play_count(),
                         player_count(0) {}
    
    void loadAllPlayers();
    void createMatchesWork(std::vector<WorkItem> &work_vector);
    void runOneMatch();
    void runMatchesAsynchronously();
    void runMatchesSynchronously();
    void runMatches();
    void workerThread();
    void incrementIfNeeded(const std::string &id, size_t how_much);

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