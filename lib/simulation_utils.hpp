#ifndef SIMULATION_UTILS
#define SIMULATION_UTILS

#include "agents/agent_base.hpp"

#include <iostream>
#include <memory>


struct GameMove {
    std::vector <bool> board;
    std::vector <std::pair<move, int>> policy;
    int player;
    int value;
};

struct GameHistory {
    std::vector <GameMove> history;
    int result = 0;
};


GameHistory play_game(
    std::unique_ptr<AgentBase> agent1,
    std::unique_ptr<AgentBase> agent2,
    bool verbose = false)
{
    GameHistory game_history;
    auto state = GameState();

    while (!state.is_terminal()) {
        if (verbose) {
            std::cout << state.draw() << std::endl;
            std::cout << "Current player: " << (state.current_player == 1 ? "WHITE" : "BLACK") << std::endl;
        }

        if (state.current_player == 1) {
            auto[move, policy] = agent1->select_move(state);
            game_history.history.push_back(
                GameMove{state.get_tensor_representation(), policy, 1, 0}
            );

            state.make_move(move);
            agent1->make_move(move);
            agent2->make_move(move);
        }
        else {
            auto[move, policy] = agent2->select_move(state);
            game_history.history.push_back(
                GameMove{state.get_tensor_representation(), policy, 2, 0}
            );

            state.make_move(move);
            agent1->make_move(move);
            agent2->make_move(move);
        }
    }    

    auto scores = state.get_scores();

    if (verbose) {
        std::cout << "Finish!" << std::endl;
        std::cout << state.draw() << std::endl;
        std::cout << scores.first << " " << scores.second << std::endl;
    }

    if (scores.first < scores.second) {
        if (verbose) {
            std::cout << "Player 2 wins!" << std::endl;
        }

        game_history.result = 2;
        for (auto& mv : game_history.history) {
            if (mv.player == 2) {
                mv.value = 1;
            } else {
                mv.value = -1;
            }
        }
    }
    else if (scores.first > scores.second) {
        if (verbose) {
            std::cout << "Player 1 wins!" << std::endl;
        }

        game_history.result = 1;
        for (auto& mv : game_history.history) {
            if (mv.player == 1) {
                mv.value = 1;
            } else {
                mv.value = -1;
            }
        }
    }

    if (verbose) {
        std::cout << "Tie!" << std::endl;
    }

    return game_history;
}

#endif
