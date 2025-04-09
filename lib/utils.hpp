#ifndef UTILS
#define UTILS

#include <utility>

using move = std::pair <int, int>;

unsigned int move_to_id(move mv) {
    if (mv.first == -1 and mv.second == -1) {
        return 64;
    } 
    
    return 8 * mv.first + mv.second;
}

#endif
