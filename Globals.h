/*
 * Author: Nadav Markus
 * This is file contains the global configuration for the game.
 * Note: the function initGlobals must be invoked for the ALLOWED_PIECES_COUNT map to be valid.
 */


#ifndef __GLOBALS_H_
#define __GLOBALS_H_

#include <map>

#include <mutex>

namespace Globals
{
    constexpr size_t M = 10;
    constexpr size_t N = 10;
    constexpr size_t MOVES_UNTIL_TIE = 50;

    extern std::map<char, unsigned int> ALLOWED_PIECES_COUNT;

    void initGlobals();
}

#endif
