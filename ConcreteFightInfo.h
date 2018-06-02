#ifndef __CONCRETE_FIGHT_INFO_H_
#define __CONCRETE_FIGHT_INFO_H_

/*
 * Author: Nadav Markus
 * A concrete implementation of fightinfo. contains utilities to print it and to set
 */


#include "Point.h"
#include "ConcretePoint.h"

#include <assert.h>

class ConcreteFightInfo : public FightInfo
{
private:
    int winner;
    char player1_piece;
    char player2_piece;
    const ConcretePoint pos;

public:
    ConcreteFightInfo(int winner,
                      char player1_piece,
                      char player2_piece,
                      int x,
                      int y): winner(winner),
                              player1_piece(player1_piece),
                              player2_piece(player2_piece),
                              pos(x, y) {}

    virtual const Point& getPosition() const override { return pos; }
    virtual char getPiece(int player) const override
    {
        if (1 == player) return player1_piece;
        if (2 == player) return player2_piece;
        /* Should not happen. */
        assert(false);
        return '#';
    }
    
    virtual int getWinner() const override { return winner; }
                      
};

#endif