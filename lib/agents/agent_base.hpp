#ifndef AGENT_BASE
#define AGENT_BASE

#include "othello.hpp"
#include "utils.hpp"

class AgentBase {
public:
    virtual std::pair<move, std::vector <std::pair <move, int>>> select_move(const GameState& state) = 0;
    virtual void make_move(const move& move) = 0;
};

#endif
