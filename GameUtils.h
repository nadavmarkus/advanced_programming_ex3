/*
 * Author: Nadav Markus
 * This file contains some useful utilities to let us know which types are movables,
 * which types can be set to the joker masquerade, etc.
 */


#ifndef __GAME_UTILS_H_
#define __GAME_UTILS_H_

namespace GameUtils
{
    bool isValidType(char type)
    {
        switch (type) {
            case 'R':
                /* Fallthrough */
            case 'P':
                /* Fallthrough */
            case 'S':
                /* Fallthrough */
            case 'B':
                /* Fallthrough */
            case 'F':
                /* Fallthrough */
            case 'J':
                return true;
                
            default:
                return false;
        }
    }
    
    bool isValidJokerMasqueradeType(char type)
    {
        switch (type) {
            case 'R':
                /* Fallthrough */
            case 'P':
                /* Fallthrough */
            case 'S':
                /* Fallthrough */
            case 'B':
                return true;
                
            default:
                return false;
        }
    }
    
    bool isMovablePiece(char type)
    {
        switch (type) {
            case 'R':
                /* Fallthrough */
            case 'P':
                /* Fallthrough */
            case 'S':
                return true;
                
            default:
                return false;
        }
    }
    
    char getStrongerPiece(char type)
    {
        switch(type) {
            case 'R':
                return 'P';
                
            case 'P':
                return 'S';
                
            case 'S':
                return 'R';
                
            default:
                /* Should not happen. */
                return '?';
        }
    }
}

#endif