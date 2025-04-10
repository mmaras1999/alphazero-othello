#ifndef HUMAN_AGENT
#define HUMAN_AGENT

#include "agents/agent_base.hpp"
#include "utils.hpp"

#include <iostream>
#include <algorithm>

class HumanAgent : public AgentBase {
public:
    virtual std::pair<move, std::vector <std::pair <move, int>>> select_move(GameState& state) override {
        std::cout << "Waiting for move..." << std::endl;

        int x, y;
        std::cin >> x >> y;

        auto valid_moves = state.get_valid_moves();

        while (std::find(valid_moves.begin(), valid_moves.end(), move{x, y}) == valid_moves.end()) {
            std::cout << "Invalid move!" << std::endl << std::endl;

            std::cin >> x >> y;
        }

        return {{x, y}, {}};
    }

    virtual void make_move(const move& move) override {};
};

#endif
