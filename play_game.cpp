#include "othello.hpp"
#include "agents/human_agent.hpp"
#include "agents/random_agent.hpp"
#include "agents/mcts_agent.hpp"

#include <iostream>

int main() {

    auto state = GameState();
    auto agent1 = RandomAgent(1, 2);
    auto agent2 = MctsAgent(2, 1.41f, 1600);

    while (!state.is_terminal()) {
        std::cout << state.draw() << std::endl;
        std::cout << "Current player: " << (state.current_player == 1 ? "WHITE" : "BLACK") << std::endl;


        if (state.current_player == 1) {
            auto move = agent1.select_move(state);
            state.make_move(move);
            agent1.make_move(move);
            agent2.make_move(move);
        }
        else {
            auto move = agent2.select_move(state);
            state.make_move(move);
            agent1.make_move(move);
            agent2.make_move(move);
        }
    }

    std::cout << "Finish!" << std::endl;
    std::cout << state.draw() << std::endl;

    auto scores = state.get_scores();

    std::cout << scores.first << " " << scores.second << std::endl;

    if (scores.first < scores.second) {
        std::cout << "Player 2 wins!" << std::endl;
    }
    else if (scores.first > scores.second) {
        std::cout << "Player 1 wins!" << std::endl;
    }
    else {
        std::cout << "Tie!" << std::endl;
    }

    return 0;
}