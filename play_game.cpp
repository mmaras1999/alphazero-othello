#include "othello.hpp"
#include "simulation_utils.hpp"
#include "agents/human_agent.hpp"
#include "agents/random_agent.hpp"
#include "agents/mcts_agent.hpp"
#include "agents/alpha_zero_agent.hpp"

#include <iostream>
#include <memory>


int main() {
    int repeats = 10;
    float wins_1 = 0;
    float wins_2 = 0;

    for (int i = 0; i < repeats; ++i) {
        std::cout << "Starting game " << i + 1 << std::endl;

        auto agent1 = std::make_unique<MctsAgent>(1.4, 1600, 123);
        auto agent2 = std::make_unique<AlphaZeroAgent>("models/trained.onnx", 0.3, 1600);

        bool swap_agents = rand() % 2;

        if (not swap_agents) {
            auto game_history = play_game(std::move(agent1), std::move(agent2));
            
            if (game_history.result == 1) {
                wins_1 += 1;
            } else if (game_history.result == 2) {
                wins_2 += 1;
            }
            else {
                wins_1 += 0.5f;
                wins_2 += 0.5f;
            }
        }
        else {
            auto game_history = play_game(std::move(agent1), std::move(agent2));
            
            if (game_history.result == 1) {
                wins_1 += 1;
            } else if (game_history.result == 2) {
                wins_2 += 1;
            }
            else {
                wins_1 += 0.5f;
                wins_2 += 0.5f;
            }
        }
    }

    std::cout << "Agent 1 win rate: " << wins_1 / repeats << std::endl;
    std::cout << "Agent 2 win rate: " << wins_2 / repeats << std::endl;

    return 0;
}