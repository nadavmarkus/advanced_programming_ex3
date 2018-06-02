/*
 * Author: Nadav Markus
 * A concrete implementation of a fileplayeralgorithm.
 * Contains our old implementation, as in ex1, of reading a file for moves and positions.
 * Note that we can't throw erros, and therefore we return invalid moves/positions if we need
 * to report an error.
 */


#ifndef __FILE_PLAYER_ALGORITHM_H_
#define __FILE_PLAYER_ALGORITHM_H_

#include "PlayerAlgorithm.h"
#include "PiecePosition.h"
#include "Globals.h"
#include "ConcretePiecePosition.h"
#include "BadFilePathError.h"
#include "Move.h"
#include "ConcreteMove.h"
#include "JokerChange.h"
#include "ConcreteJokerChange.h"

#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <algorithm>
#include <sstream>
#include <fstream>

class FilePlayerAlgorithm : public PlayerAlgorithm
{
private:
    /* The player this algorithms handles */
    int player;
    /* The coordinates for a joker move, if one is supplied. */
    int joker_x, joker_y;
    char new_joker_type;
    bool joker_parsing_failed;
    std::ifstream player_move_file;
    
    void parseJokerParameters(std::stringstream &formatted_line)
    {
        char expected_colon, expected_j;
        formatted_line >> expected_j >> expected_colon;
        
        /* No joker in this line. */
        if (formatted_line.fail()) {
            return;
        }
        
        if ('J' != expected_j || ':' != expected_colon) {
            joker_parsing_failed = true;
            return;
        }
        
        formatted_line >> joker_x >> joker_y >> new_joker_type;
        
        if (formatted_line.fail()) {
            joker_parsing_failed = true;
            return;
        }
    }

    void parseBoardFile(std::ifstream &player_file,
                        std::vector<unique_ptr<PiecePosition>> &vectorToFill) const
    {
        std::string line;
        char type, masquerade_type;
        unsigned int x, y;
    
        for (;;)
        {
            std::getline(player_file, line, '\n');
            
            if (player_file.fail() && player_file.eof()) {
                break;
            }
            
            if (player_file.fail()) {
                vectorToFill.push_back(std::make_unique<ConcretePiecePosition>(-1, -1, -1,  '#', '#'));
            }
            
            /* Skip empty lines. */
            if (std::all_of(line.begin(), line.end(),isspace)) {
                continue;
            }
            
            std::stringstream line_formmater(line);
            
            line_formmater >> type >> x >> y;
            
            if (line_formmater.fail()) {
                vectorToFill.push_back(std::make_unique<ConcretePiecePosition>(-1, -1, -1,  '#', '#'));
            }
            
            masquerade_type = '#';
            
            /* Special handling for joker pieces. */
            if ('J' == type) {
                line_formmater >> masquerade_type;
                
                if (line_formmater.fail()) {
                    vectorToFill.push_back(std::make_unique<ConcretePiecePosition>(-1, -1, -1,  '#', '#'));
                }
            }
            
            vectorToFill.push_back(std::make_unique<ConcretePiecePosition>(player, x, y, type, masquerade_type));
        }
    }

public:
    virtual void getInitialPositions(int player, std::vector<unique_ptr<PiecePosition>>& vectorToFill) override
    {
        this->player = player;
        std::stringstream file_path;
        std::stringstream error;
        std::ifstream player_board_file;
        
        file_path << "./player" << player << ".rps_board";
        player_board_file.open(file_path.str());
        
        if (player_board_file.fail()) {
            error << "File " << file_path.str() << " does not exist";
            throw BadFilePathError(error.str());
        }
        
        file_path.str("");
        file_path << "./player" << player << ".rps_moves";
        player_move_file.open(file_path.str());
        
        if (player_move_file.fail()) {
            error << "File " << file_path.str() << " does not exist";
            throw BadFilePathError(error.str());
        }
        
        /* Let RAII take care of the file descriptor for us in case of exceptions. */
        parseBoardFile(player_board_file, vectorToFill);
    }
    
    /* We cast to void in these methods to avoid the unreferenced parameter warning. */
    virtual void notifyOnInitialBoard(const Board& b, const std::vector<unique_ptr<FightInfo>>& fights) override
    {
        (void) b;
        (void) fights;
    }
    
    virtual void notifyOnOpponentMove(const Move& move) override
    {
        (void) move;
    }
    
    virtual void notifyFightResult(const FightInfo& fightInfo) override
    {
        (void) fightInfo;
    }
    
    virtual unique_ptr<Move> getMove() override
    {
        std::string line;
        std::getline(player_move_file, line);
        
        joker_x = 0;
        joker_y = 0;
        new_joker_type = '#';
        joker_parsing_failed = false;
        
        if (player_move_file.fail()) {
            return std::make_unique<ConcreteMove>(-1, -1, -1, -1);
        }
        
        std::stringstream formatted_line(line);
        int source_x, source_y, dest_x, dest_y;
        
        formatted_line >> source_x >> source_y >> dest_x >> dest_y;
        
        if (formatted_line.fail()) {
            return std::make_unique<ConcreteMove>(-1, -1, -1, -1);
        }
        
        parseJokerParameters(formatted_line);
        return std::make_unique<ConcreteMove>(source_x, source_y, dest_x, dest_y);

    }
    
    virtual unique_ptr<JokerChange> getJokerChange() override
    {
        if (joker_parsing_failed) {
            return std::make_unique<ConcreteJokerChange>(-1, -1, '#');
        }
        
        if (0 == joker_x && 0 == joker_y && '#' == new_joker_type) {
            return nullptr;
        }
        
        return std::make_unique<ConcreteJokerChange>(joker_x, joker_y, new_joker_type);
    }
};

#endif 