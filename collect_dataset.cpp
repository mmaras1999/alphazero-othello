#include "othello.hpp"
#include "simulation_utils.hpp"
#include "agents/human_agent.hpp"
#include "agents/random_agent.hpp"
#include "agents/mcts_agent.hpp"
#include "agents/alpha_zero_agent.hpp"
#include "dataset.hpp"

#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <random>


Dataset dataset(2000000);
std::mutex dataset_mutex;
std::mutex std_out_mutex;
int game_id = 1;


void collect_data_from_games(int num_games, uint_fast32_t seed) {
    std::mt19937 random_gen(seed);

    for (int r = 0; r < num_games; ++r) {
        std_out_mutex.lock();
        std::cout << "Starting game " << game_id << std::endl;
        game_id++;
        std_out_mutex.unlock();

        auto agent1 = std::make_unique<AlphaZeroAgent>("models/trained.onnx", 0.3f, 800);
        auto agent2 = std::make_unique<AlphaZeroAgent>("models/trained.onnx", 0.3f, 800);

        bool swap_agents = random_gen() % 2;

        GameHistory game_history;

        if (not swap_agents) {
            game_history = play_game(std::move(agent1), std::move(agent2));
        }
        else {
            game_history = play_game(std::move(agent2), std::move(agent1));
        }

        dataset_mutex.lock();
        for (auto& history_sample : game_history.history) {
            dataset.add(history_sample.board, history_sample.policy, history_sample.value);
        }
        dataset_mutex.unlock();
    }
}


int main() {
    int num_threads = 20;
    int repeats_per_thread = 10;
    std::srand(std::time(NULL));

    std::vector <std::thread> threads(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        threads[i] = std::thread(collect_data_from_games, repeats_per_thread, std::rand());
    }

    for (auto& thread : threads) {
        thread.join();
    }

    dataset.dump("datasets/iter_0/");

    return 0;
}