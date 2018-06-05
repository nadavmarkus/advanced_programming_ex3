
#include <mutex>
#include <iostream>
#include <map>

#include "Globals.h"

namespace Globals
{   
    unsigned int getAllowedPieceCount(char type)
    {
        switch(type) {
            case 'R':
                return 2;
            
            case 'P':
                return 5;
                
            case 'S':
                return 1;
                
            case 'B':
                return 2;
                
            case 'J':
                return 2;
                
            case 'F':
                return 1;
        
            default:
                /* Should not happen */
                return 0;
        }
    }
}
