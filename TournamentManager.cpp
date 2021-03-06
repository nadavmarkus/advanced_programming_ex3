#include <memory>
#include <string>
#include <iostream>
#include <condition_variable>
#include <random>
#include <cassert>
#include <mutex>
#include <iostream>
#include <limits>
#include <algorithm>
#include <iterator>

/* Note: I don't use the filesystem header because it exists only from c++17 onwards. */
#include <dirent.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <sys/types.h>

#include "TournamentManager.h"
#include "Game.h"
#include "PlayerAlgorithm.h"

void TournamentManager::loadAllPlayers()
{
    DIR *raw_so_dir = opendir(so_directory.c_str());
    
    if (nullptr == raw_so_dir) {
        std::cerr << "Failed to retrieve dir entries from the so dir." << std::endl;
        return;
    }
    
    struct dirent *dir_entry;
    
    while (nullptr != (dir_entry = readdir(raw_so_dir))) {
        if (DT_REG == dir_entry->d_type) {
            /* Let's attempt to load the potential file. */
            std::string name(dir_entry->d_name);
            std::string prefix("RPSPlayer_");
            
            if (0 != name.compare(0, prefix.size(), prefix)
                /* 
                 * Note that these reads are safe, due to the fact that we are guaranteed
                 * in this stage that the file name has at least the substring 'RPSPlayer_'
                 */
                || 'o' != name[name.size() - 1]
                || 's' != name[name.size() - 2]) {
                /* This is not another player's lib. */
                continue;
            }
            void *algorithm_so = dlopen((so_directory + name).c_str(), RTLD_NOW);
            
            if (nullptr == algorithm_so) {
                std::cerr << "Failed to load player from " << name << std::endl;
                std::cout << dlerror() << std::endl;
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
    std::vector<std::string> all_ids;
    
    for (const auto &pair: id_to_algorithm) {
        all_ids.push_back(pair.first);
    }
    
    assert(all_ids.size() > 1);
    
    /*
     * This vector contains an entry for each possible player.
     * The entry consists of a vector of play counts. The j'th entry in this vector
     * contains the amount of scheduled matches against the j'th opponent.
     * We use this vector to retrieve each time the opponent with which the current player
     * had the least amount of plays.
     */
    std::vector<std::vector<size_t>> planned_games_count;
    
    for (size_t i = 0; i < all_ids.size(); ++i) {
        planned_games_count.push_back(std::vector<size_t>(all_ids.size()));
        /* 
         * The diagonal corresponds to games with ourselves. We explicitly set this to the maximum possible count,
         * so it will never get chosen.
         */
        planned_games_count[i][i] = std::numeric_limits<size_t>::max();
    }
    
    /* Generate the matches - we try to spread evenly as much as possible. */
    for (size_t i = 0; i < planned_games_count.size(); ++i) {
        std::string current = all_ids[i];
        while (scheduled_matches[current] < TournamentManager::REQUIRED_GAMES) {
            std::vector<size_t> &current_matches = planned_games_count[i];
            
            size_t min_pos = std::distance(current_matches.begin(),
                                           std::min_element(current_matches.begin(), current_matches.end()));
            assert(0 <= min_pos && min_pos < all_ids.size());
            
            std::string opponent = all_ids[min_pos];
            
            WorkItem item(current, opponent);
            work_vector.push_back(item);
            
            scheduled_matches[current]++;
            scheduled_matches[opponent]++;
            
            planned_games_count[i][min_pos]++;
            planned_games_count[min_pos][i]++;
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
    
    std::cout << "Generated " << work_vector.size() << " jobs" << std::endl;
    
    /* We will push all the jobs, and additionally, we will create the termination jobs afterwards. */
    work_queue.push(work_vector);
    
    WorkItem termination_item(true);
    for (size_t i = 0; i < threads.size(); ++i) {
        work_queue.push(termination_item);
    }
    
    /* Welp, time to wait for all the threads to terminate. */
    for (auto &thread: threads) {
        thread.join();
    }
    
    /* There should be no work items left to process. The termination jobs should the last jobs executed. */
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
        
        Game game;
        int winner = game.run(*player1, *player2, message);
        updateWithItemResults(work_item, winner);
    }
}

void TournamentManager::runMatches()
{
    std::cout << "Going to run.. " << std::endl;
    if (thread_count > 1) {
        std::cout << "Running asynchronously.. " << std::endl;
        runMatchesAsynchronously();
    
    } else {
        std::cout << "Running synchronously.. " << std::endl;
        runMatchesSynchronously();
    }
    
    /* Let's print the results. */
    std::cout << "Printing results.. " << std::endl;
                
    std::vector<std::pair<std::string, size_t>> sorted;
    
    for (const auto &element: id_to_points) {
        sorted.push_back(element);
    }
    
    auto comparer = [](std::pair<std::string, size_t> a,
                       std::pair<std::string, size_t> b) -> bool 
                {
                    return a.second > b.second;
                };
    
    std::sort(sorted.begin(), sorted.end(), comparer);  
    
    for (const auto &pair: sorted) {
        std::cout << pair.first << " " << pair.second << std::endl;
    }
}

void TournamentManager::run()
{
    loadAllPlayers();
    
    if (player_count < 2) {
        std::cerr << "Please supply at least 2 players in the so directory." << std::endl;
        return;
    }
    
    runMatches();
}



