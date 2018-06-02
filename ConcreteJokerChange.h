/*
 * Author: Nadav Markus
 * A concrete implementation of joker change.
 */


#ifndef __CONCRETE_JOKER_CHANGE_H_
#define __CONCRETE_JOKER_CHANGE_H_

#include "JokerChange.h"
#include "Point.h"
#include "ConcretePoint.h"

class ConcreteJokerChange : public JokerChange
{
private:
    ConcretePoint position;
    char new_type;
public:
    ConcreteJokerChange(int x, int y, char new_type): position(x, y), new_type(new_type) {}
    virtual const Point& getJokerChangePosition() const override { return position; }
    virtual char getJokerNewRep() const override { return new_type; }
};

#endif