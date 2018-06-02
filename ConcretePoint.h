/*
 * Author: Nadav Markus
 * A concrete implementation of a point. Contains utilities to initialize it from other
 * instances, and to compare it in order to insert it to a set.
 */


#ifndef __CONCRETE_POINT_H_
#define __CONCRETE_POINT_H_

#include "Point.h"

#include <stdlib.h>
#include <cmath>

class ConcretePoint : public Point
{
    private:
        int x, y;
        
    public:
        ConcretePoint() : x(0), y(0) {}
        ConcretePoint(int x, int y) : x(x), y(y) {}
        ConcretePoint(const Point &other): x(other.getX()), y(other.getY()) {}
        virtual int getX() const override { return x; }
        virtual int getY() const override { return y; }
        
        size_t getDistance(const Point &other) const
        {
            return static_cast<size_t>(abs(other.getX() - getX()) + abs(other.getY() - getY()));
        }
        
        const ConcretePoint& operator=(const Point &other)
        {
            if (this == &other) {
                return *this;
            }
            
            x = other.getX();
            y = other.getY();
            
            return *this;
        }
};

/* Used so we can insert points to a set. */
bool operator <(const Point &a, const Point &b)
{
    /* Perform lexicographical comparison */
    if (a.getX() < b.getX()) {
        return true;
    }
    
    if (a.getX() > b.getX()) {
        return false;
    }
    
    /* This means the X coordinates are equal. */
    if (a.getY() < b.getY()) {
        return true;
    }
    
    return false;
}

#endif