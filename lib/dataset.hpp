#ifndef DATASET
#define DATASET

#include "utils.hpp"

#include <vector>
#include <string>
#include <algorithm>
#include <fstream>

class Dataset {
private:
    struct Sample {
        std::vector <float> board;
        std::vector <float> policy;
        float value;
    };

    int max_size;
    int last_sample_ptr = 0;

public:
    std::vector <Sample> samples;
    Dataset(int max_size)
        : max_size(max_size),
          last_sample_ptr(0) {
        samples.reserve(max_size);
    }

    void add(std::vector<float> board_tensor, std::vector<std::pair <move, int>> policy, int value) {
        if (last_sample_ptr == samples.size()) {
            samples.push_back(Sample{{}, {}, 0.0f});
        }
        samples[last_sample_ptr].board = board_tensor;
        samples[last_sample_ptr].value = value;
        samples[last_sample_ptr].policy.resize(65, 0.0f);
        
        int sum = 0;

        std::for_each(policy.begin(), policy.end(), [&] (const std::pair <move, int>& x) {
            sum += x.second;
        });

        for (auto& mv : policy) {
            int id = move_to_id(mv.first);

            samples[last_sample_ptr].policy[id] = mv.second / (float)sum;
        }
        
        last_sample_ptr = (last_sample_ptr + 1) % max_size;
    }

    void dump(std::string path) {
        std::ofstream board_dump_file((path + "board.bin").c_str(), std::ios::binary);
        for (auto& sample : samples) {
            board_dump_file.write(reinterpret_cast<char*>(sample.board.data()), sample.board.size() * sizeof(float));
        }
        board_dump_file.close();

        std::ofstream policy_dump_file((path + "policy.bin").c_str(), std::ios::binary);
        for (auto& sample : samples) {
            policy_dump_file.write(reinterpret_cast<char*>(sample.policy.data()), sample.policy.size() * sizeof(float));
        }
        policy_dump_file.close();
        
        std::ofstream value_dump_file((path + "value.bin").c_str(), std::ios::binary);
        for (auto& sample : samples) {
            value_dump_file.write(reinterpret_cast<char*>(&sample.value), sizeof(float));
        }
        value_dump_file.close();
    }
};

#endif
