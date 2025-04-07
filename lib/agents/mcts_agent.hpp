#ifndef MCTS_AGENT
#define MCTS_AGENT

#include "agents/agent_base.hpp"
#include "othello.hpp"
#include "utils.hpp"

#include <ctime>
#include <random>
#include <cmath>
#include <stdexcept>

class MctsAgent : public AgentBase {
private:
    const int DEFAULT_SIZE = 2000000;

    struct MctsTreeNode {
        std::vector <std::pair <move, int> > children;
        int visits;
        int score;

        MctsTreeNode(GameState& state) {
            auto valid_moves = state.get_valid_moves();
            children.reserve(64);

            for (auto& move : valid_moves) {
                children.emplace_back(move, -1);
            }

            visits = 0;
            score = 0;
        }
    };
    std::vector <MctsTreeNode> tree;

    GameState root;
    int root_id = 0;
    float uct_factor;
    int iters_per_move;
    std::mt19937 generator;

    int rollout(GameState& state, int player) {
        while (not state.is_terminal()) {
            auto valid_moves = state.get_valid_moves();
            int move_id = generator() % valid_moves.size();

            state.make_move(valid_moves[move_id]);
        }

        auto scores = state.get_scores();

        if (scores.first < scores.second) {
            return player == 1 ? -1 : 1;
        }

        if (scores.first > scores.second) {
            return player == 1 ? 1 : -1;
        }

        return 0;
    }

    int select(int parent) {
        int best_ch = -1;
        float best_uct_score = -1e9;
        
        float nominator = std::log(tree[parent].visits);

        for (int i = 0; i < tree[parent].children.size(); ++i) {
            auto& action = tree[parent].children[i];

            if (action.second == -1) {
                return i;
            }

            auto& child = tree[action.second];

            float uct_score = child.score / (float)child.visits + uct_factor * std::sqrtf(nominator / (float)child.visits);

            if (uct_score > best_uct_score) {
                best_ch = i;
                best_uct_score = uct_score;
            }
        }

        return best_ch;
    }

    int search_iter(GameState& state, int node_id) {
        tree[node_id].visits++;
        
        // terminal node
        if (not tree[node_id].children.size()) {
            int value = rollout(state, state.current_player ^ 3);
            tree[node_id].score += value;
            return -value;
        }

        // select and apply action
        int child = select(node_id);
        state.make_move(tree[node_id].children[child].first);

        if(tree[node_id].children[child].second == -1) {
            tree[node_id].children[child].second = tree.size();
            tree.emplace_back(state);

            int r = rollout(state, state.current_player ^ 3);
            tree.back().visits++;
            tree.back().score += r;

            tree[node_id].score -= r;

            return r;
        }

        int r = search_iter(state, tree[node_id].children[child].second);
        tree[node_id].score += r;
        
        return -r;
    }

public:
    MctsAgent(float uct_factor, int iters_per_move)
        : uct_factor(uct_factor),
          iters_per_move(iters_per_move)
    {
        tree.reserve(DEFAULT_SIZE);
        tree.emplace_back(root);
    }

    MctsAgent(float uct_factor, int iters_per_move, uint_fast32_t seed)
        : MctsAgent(uct_factor, iters_per_move)
    {
        generator.seed(seed);
    }

    virtual std::pair<move, std::vector<std::pair<move, int>>> select_move(GameState& state) override {
        for (int i = 0; i < iters_per_move; ++i) {
            GameState state_copy = root;
            search_iter(state_copy, root_id);
        }

        int best_move = 0;
        float best_value = -1e9;
        std::vector <std::pair<move, int>> policy(tree[root_id].children.size());

        for (int ch = 0; ch < tree[root_id].children.size(); ++ch) {
            auto mv = tree[root_id].children[ch];

            if (mv.second != -1) {
                policy[ch] = {mv.first, tree[mv.second].visits};
                float ch_value = tree[mv.second].score / (float)tree[mv.second].visits;

                if (best_value < ch_value) {
                    best_value = ch_value;
                    best_move = ch;
                }
            }
            else {
                policy[ch] = {mv.first, 0};
            }
        }

        return {
            tree[root_id].children[best_move].first,
            policy
        };
    }

    virtual void make_move(const move& move) override {
        root.make_move(move);

        for (auto& mv : tree[root_id].children) {
            if (mv.first == move) {
                if (mv.second == -1) {
                    mv.second = tree.size();
                    tree.emplace_back(root);
                }

                root_id = mv.second;
                return;
            }
        }

        throw std::runtime_error("Tried to apply move that is invalid!");
    }
};

#endif
