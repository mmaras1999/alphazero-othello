#ifndef RANDOM_AGENT
#define RANDOM_AGENT

#include "agents/agent_base.hpp"
#include "utils.hpp"

#include <random>

class RandomAgent : public AgentBase {
private:
    std::mt19937 mt;

public:
    RandomAgent(unsigned int seed = 1234) {
        std::mt19937 mt(seed);
    }

    virtual std::pair<move, std::vector<std::pair<move, int>>> select_move(GameState& state) override {
        auto valid_moves = state.get_valid_moves();

        std::uniform_int_distribution<int> dist(0, valid_moves.size() - 1);
        return {valid_moves[dist(mt)], {}};
    }

    virtual void make_move(const move& move) override {};
};

#endif
