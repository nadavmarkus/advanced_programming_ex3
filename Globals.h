/*
 * Author: Nadav Markus
 * This is file contains the global configuration for the game.
 * Note: the function initGlobals must be invoked for the ALLOWED_PIECES_COUNT map to be valid.
 */


#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <map>

namespace Globals
{
    constexpr size_t M = 10;
    constexpr size_t N = 10;
    constexpr size_t MOVES_UNTIL_TIE = 50;

    std::map<char, unsigned int> ALLOWED_PIECES_COUNT;

    void initGlobals()
    {
        static bool initialized = false;
        if (initialized) {
            return;
        }
        
        ALLOWED_PIECES_COUNT['R'] = 2;
        ALLOWED_PIECES_COUNT['P'] = 5;
        ALLOWED_PIECES_COUNT['S'] = 1;
        ALLOWED_PIECES_COUNT['B'] = 2;
        ALLOWED_PIECES_COUNT['J'] = 2;
        ALLOWED_PIECES_COUNT['F'] = 1;
        
        initialized = true;
    }
}

#endif
