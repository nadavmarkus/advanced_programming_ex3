/*
 * Author: Nadav Markus
 * A concrete implementation of pieceposition. Contains utilities to initialize
 * it from other instances and to assign to it. Can also normalize and get the effective
 * piece type at a location.
 */


#ifndef __CONCRETE_PIECE_POSITION_H_
#define __CONCRETE_PIECE_POSITION_H_

#include "PiecePosition.h"
#include "ConcretePoint.h"
#include "Point.h"

#include <memory>

class ConcretePiecePosition : public PiecePosition
{
private:
    int player;
    ConcretePoint point;
    char type, joker_type;
    
public:
    /* Copy assignment. Used inside ConcreteBoard when adding new pieces. */
    ConcretePiecePosition& operator=(const ConcretePiecePosition &other)
    {
        if (&other == this) {
            return *this;
        }
        
        player = other.getPlayer();
        /* Note: The point stays the same. */
        point = ConcretePoint(other.getPosition());
        type = other.getPiece();
        joker_type = other.getJokerRep();
        
        return *this;
    }
    
    ConcretePiecePosition(int player,
                          const Point &point,
                          char type,
                          char joker_type='#'):
                          ConcretePiecePosition(player, point.getX(), point.getY(), type, joker_type) {}
    
    ConcretePiecePosition(int player,
                          int x,
                          int y,
                          char type,
                          char joker_type='#') : player(player),
                                             point(x, y),
                                             type(type),
                                             joker_type(joker_type) {}
                                             
    ConcretePiecePosition(int player, const PiecePosition &other) :ConcretePiecePosition(player,
                                                                                         other.getPosition().getX(),
                                                                                         other.getPosition().getY(),
                                                                                         other.getPiece(),
                                                                                         other.getJokerRep()) {}

    ConcretePiecePosition() : player(0), point(), type('#'), joker_type('#') {}
    
    ConcretePiecePosition(const ConcretePiecePosition &other): ConcretePiecePosition(other.getPlayer(), other) {}

    const ConcretePiecePosition& operator=(ConcretePiecePosition &&other)
    {
        if (this == &other) {
            return *this;
        }
        
        type = other.getPiece();
        joker_type = other.getJokerRep();
        player = other.getPlayer();
        /* Note: The point stay the same. */
        point = ConcretePoint(other.getPosition());
        
        /* Invalid the other instance now. */
        other.reset();
        
        return *this;
    }
    
    void reset()
    {
        type = '#';
        joker_type = '#';
        player = 0;
    }
    
    virtual const Point& getPosition() const override { return point; }
    virtual char getPiece() const override { return type; }
    virtual char getJokerRep() const override { return joker_type; }
    int getPlayer() const { return player; }
    void setPoint(int x, int y) { point = ConcretePoint(x, y); }
    void setJokerRep(char joker_type) { this->joker_type = joker_type; }
    /* Used to calculate the winner of battles and the like. */
    char effectivePieceType() const
    {
        if ('J' == getPiece()) return getJokerRep();
        return getPiece();
    }
};


#endif