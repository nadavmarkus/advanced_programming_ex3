/*
 * Author: Nadav Markus
 * A concrete implementation of a board. contains utilities to print it and to set
 * piece positions inside it.
 */

#ifndef __CONCRETE_BOARD_H_
#define __CONCRETE_BOARD_H_

#include "Board.h"
#include "Globals.h"
#include "ConcretePiecePosition.h"
#include "Move.h"
#include <stdlib.h>
#include <utility>
#include <sstream>
#include <string>
#include <cctype>


class ConcreteBoard : public Board
{
private:
    ConcretePiecePosition board[Globals::N][Globals::M];
    
public:
    ConcreteBoard(): board()
    {
        for (size_t i = 0; i < Globals::N; ++i) {
            for (size_t j = 0; j < Globals::M; ++j) {
                board[i][j].setPoint(j + 1, i + 1);
            }
        }
    }

    virtual int getPlayer(const Point& pos) const override
    {
        return board[pos.getY() - 1][pos.getX() - 1].getPlayer();
    }
    
    /* We don't to overload a virtual function.. */
    int getPlayerAt(int x, int y) const
    {
        return board[y - 1][x - 1].getPlayer();
    }
    
    /* Make sure that points stay consistent. */
    void addPosition(const ConcretePiecePosition &position)
    {
        board[position.getPosition().getY() - 1][position.getPosition().getX() - 1] = position;
    }
    
    const ConcretePiecePosition& getPiece(const Point &point) const
    {
        return board[point.getY() - 1][point.getX() - 1];
    }
    
    const ConcretePiecePosition& getPiece(int x, int y) const
    {
        return board[y - 1][x - 1];
    }
    
    void movePiece(const Point &from, const Point &to)
    {
        board[to.getY() - 1][to.getX() - 1] = std::move(board[from.getY() - 1][from.getX() - 1]);
        board[to.getY() - 1][to.getX() - 1].setPoint(to.getX(), to.getY());
    }
    
    void movePiece(const Move &move)
    {
        movePiece(move.getFrom(), move.getTo());
    }
    
    void invalidatePosition(const Point &where)
    {
        board[where.getY() - 1][where.getX() - 1].reset();
    }
    
    void updateJokerPiece(const Point &where, char new_joker_type)
    {
        board[where.getY() - 1][where.getX() - 1].setJokerRep(new_joker_type);
    }
    
    std::string printBoard() const
    {
        std::stringstream result;
        for (size_t i = 0; i < Globals::N; ++i) {
            for (size_t j = 0; j < Globals::M; ++j) {
                char c = board[i][j].getPiece();
                if (board[i][j].getPlayer() == 2) {
                    c = tolower(c);
                }
                result << c;
            }
            result << std::endl;
        }
        
        result << "-------------------------" << std::endl;
        return result.str();
    }
};

#endif
