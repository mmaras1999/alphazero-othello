#ifndef RANDOM_AGENT
#define RANDOM_AGENT

#include "agents/agent_base.hpp"
#include <random>
#include <iostream>

class RandomAgent : public AgentBase {
private:
    std::mt19937 mt;

public:
    RandomAgent(int player_id, unsigned int seed = 1234)
        :   AgentBase(player_id) {
        std::mt19937 mt(seed);
    }

    virtual std::pair <int, int> select_move(const GameState& state) override {
        auto valid_moves = state.get_valid_moves();

        for (auto& move : valid_moves) {
            std::cout << move.first << " " << move.second << std::endl;
        }

        std::uniform_int_distribution<int> dist(0, valid_moves.size() - 1);

        return valid_moves[dist(mt)];
    }

    virtual void make_move(const std::pair<int, int>& move) override {};
};

#endif
