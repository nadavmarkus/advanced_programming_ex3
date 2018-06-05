#include <memory>
#include <string>
#include <iostream>
#include <condition_variable>
#include <random>
#include <cassert>
#include <dlfcn.h>
#include <mutex>

/* Note: I don't use the filesystem header because it exists only from c++17 onwards. */
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "TournamentManager.h"
#include "Game.h"
#include "PlayerAlgorithm.h"

void TournamentManager::loadAllPlayers()
{
    DIR *raw_so_dir = opendir(so_directory.c_str());
    
    if (nullptr == raw_so_dir) {
        //TODO: Handle error.
        return;
    }
    
    struct dirent *dir_entry;
    
    while (nullptr != (dir_entry = readdir(raw_so_dir))) {
        if (DT_REG == dir_entry->d_type) {
            /* Let's attempt to load the potential file. */
            void *algorithm_so = dlopen(dir_entry->d_name, RTLD_NOW);
            
            if (nullptr == algorithm_so) {
                // TODO: Handle error
            }
        }
    }
    
    (void) closedir(raw_so_dir);
}

/* Note: The caller is responsible for locking. */
void TournamentManager::incrementIfNeeded(const std::string &id, size_t how_much)
{
    if (id_to_play_count[id] < TournamentManager::REQUIRED_GAMES) {
        id_to_points[id] += how_much;
    }
}

/* Note: The caller is responsible for locking. */
void TournamentManager::updateWithItemResults(const WorkItem &work_item, int winner)
{
    switch(winner) {
        case 0:
            incrementIfNeeded(work_item.player1_id, 1);
            incrementIfNeeded(work_item.player2_id, 1);
            break;
        
        case 1:
            incrementIfNeeded(work_item.player1_id, 3);
            break;
            
        case 2:
            incrementIfNeeded(work_item.player2_id, 3);
            break;
            
        default:
            /* Should not happen. */
            assert(false);
    }
    
    id_to_play_count[work_item.player1_id]++;
    id_to_play_count[work_item.player2_id]++;
}

void TournamentManager::workerThread()
{
    for (;;) {
        const WorkItem &work_item = work_queue.pop();
        
        if (work_item.should_terminate) {
            break;
        }
        
        std::unique_ptr<PlayerAlgorithm> player1 = id_to_algorithm[work_item.player1_id]();
        std::unique_ptr<PlayerAlgorithm> player2 = id_to_algorithm[work_item.player2_id]();
        std::string message;
        
        int winner = Game().run(*player1, *player2, message);
        
        {
            std::lock_guard<std::mutex> lock(global_stats_mutex);
            updateWithItemResults(work_item, winner);
        }
    }
}

void TournamentManager::createMatchesWork(std::vector<WorkItem> &work_vector)
{
    std::map<std::string, size_t> scheduled_matches;
    std::default_random_engine generator;
    std::vector<std::string> all_ids;
    
    for (const auto &pair: id_to_play_count) {
        all_ids.push_back(pair.first);
    }
    
    assert(all_ids.size() > 1);
    
    std::uniform_int_distribution<size_t> distribution(0, all_ids.size() - 1);
    
    for (const auto &pair: id_to_play_count) {
        std::string current = pair.first;
        while (scheduled_matches[current] < TournamentManager::REQUIRED_GAMES) {
            for (;;) {
                
                size_t index = distribution(generator);
                std::string opponent = all_ids[index];
                
                /* We rely on the fact that eventually we will find a suitable opponent. */
                if (opponent == current) {
                    continue;
                }
                
                /* OK - we got a different player. Lets generate a match. */
                WorkItem item(current, opponent);
                work_vector.push_back(item);
                
                scheduled_matches[current]++;
                scheduled_matches[opponent]++;
                break;
            }
        }
    }
}

void TournamentManager::runMatchesAsynchronously()
{
    std::vector<std::thread> threads;
    
    for (size_t i = 0; i < thread_count - 1; ++i) {
        threads.push_back(std::thread(&TournamentManager::workerThread, this));
    }
    
    std::vector<WorkItem> work_vector;
    createMatchesWork(work_vector);
    
    /* We will push all the jobs, and additionally, we will create the termination jobs afterwards. */
    work_queue.push(work_vector);
    
    WorkItem termination_item(false);
    for (size_t i = 0; i < thread_count - 1; ++i) {
        work_queue.push(termination_item);
    }
    
    /* Welp, time to wait for all the threads to terminate. */
    for (auto &thread: threads) {
        thread.join();
    }
    
    assert(work_queue.empty());
}

void TournamentManager::runMatchesSynchronously()
{
    std::vector<WorkItem> work_vector;
    createMatchesWork(work_vector);
    
    for (const auto &work_item: work_vector) {
        std::unique_ptr<PlayerAlgorithm> player1 = id_to_algorithm[work_item.player1_id]();
        std::unique_ptr<PlayerAlgorithm> player2 = id_to_algorithm[work_item.player2_id]();
        std::string message;
        
        int winner = Game().run(*player1, *player2, message);
        updateWithItemResults(work_item, winner);
    }
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



