#ifndef ALPHA_ZERO_AGENT
#define ALPHA_ZERO_AGENT

#include "agents/agent_base.hpp"
#include "onnx_model.hpp"
#include "othello.hpp"
#include "utils.hpp"

#include <string>
#include <random>
#include <onnxruntime/onnxruntime_cxx_api.h>

class AlphaZeroAgent : public AgentBase {
private:
    const int DEFAULT_SIZE = 2000000;
    const int BEST_MOVE_THRESH = 10;
    struct Child {
        move mv;
        int node_id;
        float policy_score;

        Child(move mv, int node_id, float policy_score)
            : mv(mv), node_id(node_id), policy_score(policy_score) {}
    };

    struct MctsTreeNode {
        std::vector <Child> children;
        int visits;
        float score;
        float evaluation_value;

        MctsTreeNode(GameState& state, OnnxModel& model) {
            visits = 0;
            score = 0;
            evaluation_value = 0.0;

            auto valid_moves = state.get_valid_moves();

            if (state.is_terminal()) {
                auto scores = state.get_scores();

                if (scores.first < scores.second) {
                    evaluation_value = (state.current_player == 1 ? -1.0 : 1.0);
                }

                if (scores.first > scores.second) {
                    evaluation_value = (state.current_player == 1 ? 1.0 : -1.0);
                }
            }
            else {
                auto board_tensor = state.get_tensor_representation();
                auto [value, policy] = model.run_inference(board_tensor);

                evaluation_value = value;

                children.reserve(64);
                float policy_sum = 0.0f;

                for (auto& move : valid_moves) {
                    int id = move_to_id(move);
                    policy_sum += policy[id];
                }

                for (auto& move : valid_moves) {
                    int id = move_to_id(move);
                    children.emplace_back(move, -1, policy[id] / policy_sum);
                }
            }
        }
    };

    std::vector <MctsTreeNode> tree;
    GameState root;
    int root_id = 0;
    float puct_factor;
    int iters_per_move;
    std::mt19937 generator;
    int move_cnt;

    OnnxModel model;

    void root_add_noise() {}

    int select(int parent) {
        int best_ch = -1;
        float best_puct_score = -1e9;
        
        float nominator = puct_factor * std::sqrtf(tree[parent].visits);

        for (int i = 0; i < tree[parent].children.size(); ++i) {
            auto& child = tree[parent].children[i];

            float exploit_score = 0.0;
            int visits = 0;

            if (child.node_id != -1) {
                visits = tree[child.node_id].visits; 
                exploit_score = tree[child.node_id].score / visits;
            }

            float puct_score = exploit_score + child.policy_score * nominator / (1 + visits);

            if (puct_score > best_puct_score) {
                best_ch = i;
                best_puct_score = puct_score;
            }
        }

        return best_ch;
    }

    float search_iter(GameState& state, int node_id) {
        tree[node_id].visits++;
        
        // terminal node
        if (not tree[node_id].children.size()) {
            float value = -tree[node_id].evaluation_value;
            tree[node_id].score += value;
            return -value;
        }

        // select and apply action
        int child = select(node_id);
        state.make_move(tree[node_id].children[child].mv);

        if(tree[node_id].children[child].node_id == -1) {
            tree[node_id].children[child].node_id = tree.size();
            tree.emplace_back(state, model);

            float r = -tree.back().evaluation_value;
            tree.back().visits++;
            tree.back().score += r;

            tree[node_id].score -= r;

            return r;
        }

        float r = search_iter(state, tree[node_id].children[child].node_id);
        tree[node_id].score += r;
        
        return -r;
    }

public:
    AlphaZeroAgent(std::string model_path, float puct_factor, int iters_per_move)
        : AgentBase(),
          puct_factor(puct_factor),
          iters_per_move(iters_per_move),
          move_cnt(0),
          model(model_path) {
        tree.reserve(DEFAULT_SIZE);
        tree.emplace_back(root, model);
    }

    AlphaZeroAgent(std::string model_path, float puct_factor, int iters_per_move, uint_fast32_t seed)
        : AlphaZeroAgent(model_path, puct_factor, iters_per_move) {
        generator.seed(seed);
    }

    virtual std::pair<move, std::vector<std::pair<move, int>>> select_move(GameState& state) override {
        for (int i = 0; i < iters_per_move; ++i) {
            GameState state_copy = root;
            search_iter(state_copy, root_id);
        }

        int best_move = 0;
        float best_visits = 0;
        std::vector <std::pair<move, int>> policy(tree[root_id].children.size());

        for (int ch = 0; ch < tree[root_id].children.size(); ++ch) {
            auto child = tree[root_id].children[ch];

            if (child.node_id != -1) {
                int visits = tree[child.node_id].visits;
                policy[ch] = {child.mv, visits};

                if (best_visits < visits) {
                    best_visits = visits;
                    best_move = ch;
                }
            }
            else {
                policy[ch] = {child.mv, 0};
            }
        }

        if (move_cnt > BEST_MOVE_THRESH) {
            return {
                tree[root_id].children[best_move].mv,
                policy
            };
        }
        else {
            std::vector <int> weights;
            for (auto& p : policy) {
                weights.push_back(p.second);
            }

            std::discrete_distribution<> random_dist(weights.begin(), weights.end());
            return {
                policy[random_dist(generator)].first,
                policy
            };
        }
    }

    virtual void make_move(const move& move) override {
        root.make_move(move);
        ++move_cnt;

        for (auto& ch : tree[root_id].children) {
            if (ch.mv == move) {
                if (ch.node_id == -1) {
                    ch.node_id = tree.size();
                    tree.emplace_back(root, model);
                }

                root_id = ch.node_id;
                return;
            }
        }

        throw std::runtime_error("Tried to apply move that is invalid!");
    }
};

#endif
