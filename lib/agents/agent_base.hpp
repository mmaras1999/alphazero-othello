#ifndef AGENT_BASE
#define AGENT_BASE

#include "othello.hpp"

class AgentBase {
public:
    int player_id;

    AgentBase(int player_id) : player_id(player_id) {}
    virtual std::pair <int, int> select_move(const GameState& state) = 0;
    virtual void make_move(const std::pair<int, int>& move) = 0;
};

#endif

