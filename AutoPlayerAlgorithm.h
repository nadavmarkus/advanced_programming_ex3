/*
 * Author: Nadav Markus
 * This file contains the main implementation of the file auto algorithm.
 * The algorithm is simple - it attempts to eat opponent pieces if it knows for sure
 * that it can, afterwards it attempts to run out of danger if possible, and finally
 * it attempts to search the enmie's flag.
 */

#ifndef __AUTO_PLAYER_ALGORITHM_H_
#define __AUTO_PLAYER_ALGORITHM_H_

#include "PlayerAlgorithm.h"
#include "Board.h"
#include "FightInfo.h"
#include "Move.h"
#include "ConcreteBoard.h"
#include "Globals.h"
#include "ConcretePiecePosition.h"
#include "PiecePosition.h"
#include "ConcretePoint.h"
#include "ConcreteMove.h"
#include "GameUtils.h"
#include "ConcreteFightInfo.h"

#include <memory>
#include <vector>
#include <map>
#include <ctime>
#include <cstdlib>
#include <set>
#include <random>
#include <assert.h>
#include <stdlib.h>
#include <chrono>
#include <thread>

using piece_set_iterator = std::set<ConcretePoint>::iterator;

class AutoPlayerAlgorithm : public PlayerAlgorithm
{
private:
    ConcreteBoard my_board_view;
    int my_player_number;
    int other_player;
    std::vector<unique_ptr<PiecePosition>> *vector_to_fill;
    std::default_random_engine gen;
    std::uniform_int_distribution<int> bool_generator;
    std::uniform_int_distribution<int> x_generator;
    std::uniform_int_distribution<int> y_generator;
    std::set<ConcretePoint> possible_opponent_flag_locations;
    /*
     * Used to track who is executing the current move.
     */
    bool my_move;
    std::unique_ptr<Move> last_move;
    std::unique_ptr<FightInfo> last_fight_result;
    
    void fillVectorAndUpdateBoard(int x, int y, char type, char joker_type='#')
    {
        assert(nullptr != vector_to_fill);
        ConcretePiecePosition position(my_player_number, x, y, type, joker_type);
        vector_to_fill->push_back(std::make_unique<ConcretePiecePosition>(position));
        my_board_view.addPosition(position);
    }
    
    void placeOnePieceAtRandomPosition(char type)
    {
        for (;;) {
            int x = x_generator(gen);
            int y = y_generator(gen);
            
            assert(1 <= x && x <= static_cast<int>(Globals::M));
            assert(1 <= y && y <= static_cast<int>(Globals::N));
            
            /* We count on the fact that eventually we will hit an empty spot. */
            if (my_player_number == my_board_view.getPlayerAt(x , y)) {
                continue;
            }
            
            /* All right, we are good to go. */
            fillVectorAndUpdateBoard(x, y, type);
            break;
        }
    }
    
    void placePiecesAtRandomPosition(char type)
    {
        for (size_t i = 0; i < Globals::ALLOWED_PIECES_COUNT[type]; ++i) {
            placeOnePieceAtRandomPosition(type);
        }
    }
    
    void placeMovablePieces()
    {
        placePiecesAtRandomPosition('P');
        placePiecesAtRandomPosition('R');
        placePiecesAtRandomPosition('S');
    }
    
    /*
     * Updates our view with known other player unit types.
     * This method is only called for the initial fights.
     */
    void updateWithInitialFightResult(const FightInfo &info)
    {
        const Point &where = info.getPosition();
        if (other_player == info.getWinner()) {
            const ConcretePiecePosition pos(other_player, where, info.getPiece(other_player));
            my_board_view.addPosition(pos);
            
            /* This also means that the position doesn't contain a flag - a flag can't win. */
            if (possible_opponent_flag_locations.count(where)) {
                possible_opponent_flag_locations.erase(where);
            }
            
        } else if(my_player_number == info.getWinner()) {
            /* Nothing to do really - they just lost a piece. */
            if (possible_opponent_flag_locations.count(where)) {
                possible_opponent_flag_locations.erase(where);
            }
            
        /* Both units got annihilated. */
        } else if (0 == info.getWinner()) {
            my_board_view.invalidatePosition(where);
            
            if (possible_opponent_flag_locations.count(where)) {
                possible_opponent_flag_locations.erase(where);
            }
            
        } else {
            /* Should not happen. */
            assert(false);
        }
    }
    
    /* If the piece type is '?', it is considered to be a wild card. */
    bool matchingPiece(int x, int y, char type, int player) const
    {
        const ConcretePiecePosition &pos = my_board_view.getPiece(x, y);
        if (type != '?') {
            return (type == pos.effectivePieceType() && player == pos.getPlayer());
        }
        
        return player == pos.getPlayer();
    }
    
    /* 
     * This function can accept '?' as the type, and than every possible piece type will be matched.
     * Note that there is no wild card for the player.
     */
    bool hasAdjacentPieceOfType(int x,
                                int y,
                                char type,
                                int player,
                                int &result_x,
                                int &result_y) const
    {
        if (x < static_cast<int>(Globals::M)) {
            if (matchingPiece(x + 1, y, type, player)) {
                result_x = x + 1;
                result_y = y;
                return true;
            }
        }
        
        if (x > 1) {
            if (matchingPiece(x - 1, y, type, player)) {
                result_x = x - 1;
                result_y = y;
                return true;
            }
        }
            
        if (y > 1) {
            if (matchingPiece(x, y - 1, type, player)) {
                result_x = x;
                result_y = y - 1;
                return true;
            }
        }
            
        if (y < static_cast<int>(Globals::N)) {
            if (matchingPiece(x, y + 1, type, player)) {
                result_x = x;
                result_y = y + 1;
                return true;
            }
        }
        
        return false;
    }
    
    /* 
     * This method checks whether we have any piece in danger, and if so, attempts
     * to find an escape path.
     */
    unique_ptr<Move> attemptToFlee() const
    {
        for (size_t y = 1; y <= static_cast<int>(Globals::N); ++y) {
            for (size_t x = 1; x <= static_cast<int>(Globals::M); ++x) {
                const ConcretePiecePosition &pos = my_board_view.getPiece(x, y);
                
                if (my_player_number == pos.getPlayer() &&
                    ('R' ==  pos.effectivePieceType() || 
                     'P' == pos.effectivePieceType()  || 
                     'S' == pos.effectivePieceType())) {
                     
                    char my_type = pos.effectivePieceType();
                    char stronger_piece = GameUtils::getStrongerPiece(my_type);
                    
                    int dummy_x, dummy_y;
                    if (hasAdjacentPieceOfType(x, y, stronger_piece, other_player, dummy_x, dummy_y)) {
                        /* First, we will attempt to flee to an empty square. */
                        int to_x, to_y;
                        if (hasAdjacentPieceOfType(x, y, '#', 0, to_x, to_y)) {
                            return std::make_unique<ConcreteMove>(x, y, to_x, to_y);
                        }
                        
                        /* Perhaps we can attempt suicide? */
                        if (hasAdjacentPieceOfType(x, y, my_type, other_player, to_x, to_y)) {
                            return std::make_unique<ConcreteMove>(x, y, to_x, to_y);
                        }
                        
                        /* No? too bad.. lets continue to search. */
                        continue;
                    }
                }
            }
        }
        
        /* Couldn't find a way to flee. */
        return nullptr;
    }
    
    /* This method checks whether we know we have a stronger piece than the opponent, and if so, attempt to eat it. */
    unique_ptr<Move> attemptToEatOpponentPiece() const
    {
        for (size_t y = 1; y <= static_cast<int>(Globals::N); ++y) {
            for (size_t x = 1; x <= static_cast<int>(Globals::M); ++x) {
                const ConcretePiecePosition &pos = my_board_view.getPiece(x, y);
                
                /* We only attempt to eat in case we KNOW we are stronger. */
                if (other_player == pos.getPlayer() &&
                    '#' != pos.effectivePieceType()) {
                    char opponent_type = pos.effectivePieceType();
                    assert('R' == opponent_type || 'S' == opponent_type || 'P' == opponent_type);
                    
                    char stronger_piece = GameUtils::getStrongerPiece(opponent_type);
                    
                    int x_from, y_from;
                    if (hasAdjacentPieceOfType(x, y, stronger_piece, my_player_number, x_from, y_from)) {
                        return std::make_unique<ConcreteMove>(x_from, y_from, x, y);
                    }
                }
            }
        }
        
        return nullptr;
    }
    
    /* In case of search and destroy, this function finds at least one viable path for a piece to pursue the flag. */
    unique_ptr<Move> findViablemove(const ConcretePoint &to_destroy, const ConcretePoint &chosen_piece_location) const
    {
        /* OK - let's construct the move. */
        if (to_destroy.getX() > chosen_piece_location.getX() &&
            my_player_number != my_board_view.getPlayerAt(chosen_piece_location.getX() + 1, 
                                                          chosen_piece_location.getY())) {
            return std::make_unique<ConcreteMove>(chosen_piece_location,
                                                  chosen_piece_location.getX() + 1,
                                                  chosen_piece_location.getY());
                                                  
        } else if (to_destroy.getX() < chosen_piece_location.getX() &&
                   my_player_number != my_board_view.getPlayerAt(chosen_piece_location.getX() - 1, 
                                                                 chosen_piece_location.getY())) {
            
            return std::make_unique<ConcreteMove>(chosen_piece_location,
                                                  chosen_piece_location.getX() - 1,
                                                  chosen_piece_location.getY());
                                                  
        } else if (to_destroy.getY() > chosen_piece_location.getY() && 
                   my_player_number != my_board_view.getPlayerAt(chosen_piece_location.getX(),
                                                                 chosen_piece_location.getY() + 1)) {
            return std::make_unique<ConcreteMove>(chosen_piece_location,
                                                  chosen_piece_location.getX(),
                                                  chosen_piece_location.getY() + 1);
                                                  
        } else if (to_destroy.getY() < chosen_piece_location.getY() && 
                   my_player_number != my_board_view.getPlayerAt(chosen_piece_location.getX(), 
                                                                 chosen_piece_location.getY() - 1)) {
            return std::make_unique<ConcreteMove>(chosen_piece_location,
                                                  chosen_piece_location.getX(),
                                                  chosen_piece_location.getY() - 1);
        } else {
            /* Should not happen. */
            assert(false);
            return nullptr;
        }
    }
    
    /* This method attempts to seek out and eat the enemy's flag. */
    unique_ptr<Move> searchAndDestroy() const
    {
        piece_set_iterator it = possible_opponent_flag_locations.begin();
        
        assert(possible_opponent_flag_locations.size() > 0);
        assert(it != possible_opponent_flag_locations.end());
        
        size_t best_distance = SIZE_MAX;
        
        const ConcretePoint &to_destroy = *it;
        ConcretePoint chosen_piece_location;
        
        /* Let's try a movable piece with the closet coordinates. */
        for (size_t y = 1; y <= static_cast<int>(Globals::N); ++y) {
            for (size_t x = 1; x <= static_cast<int>(Globals::M); ++x) {
                const ConcretePiecePosition &pos = my_board_view.getPiece(x, y);
                
                if (my_player_number != pos.getPlayer()) {
                    continue;
                }
                
                if (!GameUtils::isMovablePiece(pos.effectivePieceType())) {
                    continue;
                }
                
                /* All right, we got a new candidate. */
                size_t cur_distance = to_destroy.getDistance(pos.getPosition());
                int dummy_x, dummy_y;
                
                /* This check makes sure we can find a viable path for the current piece - if it is surrounded by our pieces
                 * for example we can't move, so we need to skip it.
                 */
                if ((cur_distance < best_distance) &&
                    (hasAdjacentPieceOfType(x, y, '?', 0, dummy_x, dummy_y) ||
                     hasAdjacentPieceOfType(x, y, '?', other_player, dummy_x, dummy_y))) {
                    
                    best_distance = cur_distance;
                    chosen_piece_location = pos.getPosition();
                }
            }
        }
        
        /* This is bad - we are out of movable pieces. */
        if (0 == chosen_piece_location.getX() || 0 == chosen_piece_location.getY()) {
            return nullptr;
        }
        
        assert(best_distance > 0);
        
        return findViablemove(to_destroy, chosen_piece_location);
    }
    
    /* This method updates our view of the world. */
    void flushPreviousMovesData()
    {
        if (nullptr == last_move) {
            /* This can happen if we are player1 and this is the first turn. Nothing to do. */
            return;
        }
     
        /* 
         * This means we just got invoked before the next opponent's move.
         * This means that last_move is our move.
         */
        if (my_move) {
            /* Did a fight occur? */
            if (nullptr == last_fight_result) {
                /* No? this means our move was for sure successfully executed. */
                my_board_view.movePiece(*last_move);
                return;
            }
            
            /* 
             * No matter the result, a flag cannot be at the location where the fight took place.
             * The only possible situation is when we attacked a flag - but in that case, we already won,
             * so we can remove anyway this possible location for a flag.
             */
            if (possible_opponent_flag_locations.count(last_fight_result->getPosition())) {
                possible_opponent_flag_locations.erase(last_fight_result->getPosition());
            }
            
            /* OK - a fight occurred. We were the attacker, did we win? */
            if (last_fight_result->getWinner() == my_player_number) {
                /* We can carry on with just moving the piece. */
                my_board_view.movePiece(*last_move);
                
            /* It was a tie? */
            } else if (0 == last_fight_result->getWinner()) {
                my_board_view.invalidatePosition(last_fight_result->getPosition());
                my_board_view.invalidatePosition(last_move->getFrom());
                
            /* Other player won. */
            } else {
                assert(other_player == last_fight_result->getWinner());
                /* We lost for some reason (perhaps a joker change). Update accordingly. */
                my_board_view.invalidatePosition(last_move->getFrom());
                /* We can now update the position of the target with our new found information. */
                const ConcretePiecePosition pos(other_player,
                                                last_move->getTo(),
                                                last_fight_result->getPiece(other_player));
                my_board_view.addPosition(pos);
            }
        
        /* 
         * This means we just got invoked before our own turn.
         * This means that last_move is the opponent's move.
         */
        } else {
            
            /* A flag can't move, so we can remove the from point. */
            if (possible_opponent_flag_locations.count(last_move->getFrom())) {
                possible_opponent_flag_locations.erase(last_move->getFrom());
            }
            
            /* No fight? we can just update. */
            if (nullptr == last_fight_result) {
                my_board_view.movePiece(*last_move);
                return;
            }
            
            /* A fight did occur. Update accordingly. */
            /* Other player tried to attack us, but failed. */
            if (my_player_number == last_fight_result->getWinner()) {
                my_board_view.invalidatePosition(last_move->getFrom());
                
            /* A tie. */
            } else if (0 == last_fight_result->getWinner()) {
                my_board_view.invalidatePosition(last_move->getTo());
                my_board_view.invalidatePosition(last_move->getFrom());
                
            /* Other player tried to attack, and succeeded. */
            } else {
                assert(other_player == last_fight_result->getWinner());
                const ConcretePiecePosition pos(other_player,
                                                last_move->getTo(),
                                                last_fight_result->getPiece(other_player));
                my_board_view.addPosition(pos);
                my_board_view.invalidatePosition(last_move->getFrom());
            }
        }
        
        /* Make sure the previous data is invalidated. */
        last_fight_result = nullptr;
        last_move = nullptr;
    }
    
public:
    AutoPlayerAlgorithm() : my_board_view(),
                            my_player_number(0),
                            other_player(0),
                            vector_to_fill(nullptr),
                            gen(),
                            bool_generator(0, 1),
                            x_generator(1, Globals::M),
                            y_generator(1, Globals::N),
                            possible_opponent_flag_locations(),
                            my_move(false),
                            last_move(nullptr),
                            last_fight_result(nullptr)
    {
        /* Initialize RNG. */
        /* Just to make sure that different players get different random seeds, we sleep for 2 milliseconds. */
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        gen.seed(std::chrono::system_clock::now().time_since_epoch().count());
    }

    /* Note: This algorithm assumes that there is a single flag and two jokers. */
    virtual void getInitialPositions(int player, std::vector<unique_ptr<PiecePosition>> &vectorToFill) override
    {
        vector_to_fill = &vectorToFill;
        my_player_number = player;
        other_player = (2 == my_player_number) ? 1: 2;
        
        /*
         * We will position our flag in one of the corners, than start surrounding it
         * with guarding bombs and pieces.
         */
        bool flag_up = static_cast<bool>(bool_generator(gen));
        bool flag_left = static_cast<bool>(bool_generator(gen));
        
        int x, y;
        
        x = flag_left ? 1 : Globals::M;
        y = flag_up ? 1 : Globals::N;
        
        fillVectorAndUpdateBoard(x, y, 'F');
        
        /* Surround it with our two available bombs. */
        int same_row_bomb_x = (flag_left) ? 2 : Globals::M - 1;
        fillVectorAndUpdateBoard(same_row_bomb_x, y, 'B');
        
        int same_column_bomb_y = (flag_up) ? 2 : Globals::N - 1;
        fillVectorAndUpdateBoard(x, same_column_bomb_y, 'B');
        
        /* We will keep our jokers nearby as a second line of bombs. */
        int same_row_joker_x = (flag_left) ? 3 : Globals::M - 2;
        fillVectorAndUpdateBoard(same_row_joker_x, y, 'J', 'B');
        
        int same_column_joker_y = (flag_up) ? 3 : Globals::N - 2;
        fillVectorAndUpdateBoard(x, same_column_joker_y, 'J', 'B');
        
        placeMovablePieces();
        
        /* We don't copy the vector, so get rid of the pointer. */
        vector_to_fill = nullptr;
    }

    virtual void notifyOnInitialBoard(const Board& b, const std::vector<unique_ptr<FightInfo>>& fights) override
    {
        for (int y = 1; y <= static_cast<int>(Globals::N); ++y) {
            for (int x = 1; x <= static_cast<int>(Globals::M); ++x) {
                const ConcretePoint point(x, y);
                int player = b.getPlayer(point);
                
                if (0 == player) {
                    my_board_view.invalidatePosition(point);
                    
                } else if (player == my_player_number) {
                    /* Nothing to do in this case actually.. */
                
                } else if (player == other_player) {
                    /* We don't know yet the type of the opponent's piece. */
                    const ConcretePiecePosition pos(player, point, '#', '#');
                    my_board_view.addPosition(pos);
                    possible_opponent_flag_locations.insert(pos.getPosition());
                } else {
                    /* Should not happen. */
                    assert(false);
                }
            }
        }
        
        for (auto const &info: fights) {
            updateWithInitialFightResult(*info);
        }
    }
    
    virtual void notifyOnOpponentMove(const Move& move) override
    {
        flushPreviousMovesData();
        my_move = false;
        last_move = std::make_unique<ConcreteMove>(move);
    }
    
    virtual void notifyFightResult(const FightInfo &fightInfo) override
    {
        last_fight_result = std::make_unique<ConcreteFightInfo>(fightInfo.getWinner(),
                                                                fightInfo.getPiece(1),
                                                                fightInfo.getPiece(2),
                                                                fightInfo.getPosition().getX(),
                                                                fightInfo.getPosition().getY());
    }
    
    /*
     * We try the following steps, in order:
     * - If we can capture an opponent's piece (no suicide), we will try to.
     * - If we have a weaker piece near an opponent's piece, we try to run
     * - Otherwise, we attempt to search the opponent's flag.
     */
    virtual unique_ptr<Move> getMove() override
    {
        flushPreviousMovesData();
        my_move = true;
        unique_ptr<Move> result;
        
        result = attemptToEatOpponentPiece();
        
        if (nullptr != result) {
            last_move = std::move(std::make_unique<ConcreteMove>(*result));
            return result;
        }
        
        result = attemptToFlee();
        
        if (nullptr != result) {
            last_move = std::move(std::make_unique<ConcreteMove>(*result));
            return result;
        }
        
        /* OK, lets try to find the opponent's flag. */
        result = searchAndDestroy();
        
        /* 
         * This means we are out of movable pieces. Unfortunately, we can't even move the jokers
         * since they are bombs (joker changes take effect for next move), so we have to report an invalid move..
         */
        if (nullptr == result) {
            return std::make_unique<ConcreteMove>(-1, -1, -1, -1);
        }
        
        last_move = std::move(std::make_unique<ConcreteMove>(*result));
        
        assert(nullptr != result);
        return result;
    }
    
    virtual unique_ptr<JokerChange> getJokerChange() override
    {
        /* We just want to keep our jokers as bombs, no need to change them. */
        return nullptr;
    }
};

#endif