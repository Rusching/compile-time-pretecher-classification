#ifndef PATHFINDER_H
#define PATHFINDER_H

#include "../snnpp/include/network.hpp"
#include "prefetcher.h"
#include <memory>
#include <opencv2/opencv.hpp>
#include <unordered_map>
#include <vector>

using namespace std;

typedef struct lru_pc
{
    unordered_map<uint64_t, unique_ptr<training_table_info_t>> page;
    uint64_t evict;
} lru_pc_t;

typedef struct training_table_info
{
    int fired_neuron;
    int last_offset;
    unique_ptr<int[]> delta_pattern; // Use smart pointers for arrays
    uint64_t evict;
} training_table_info_t;

typedef struct prediction_table_info
{
    int label;
    int confidence;
    bool valid;
} prediction_table_info_t;

class PathfinderPrefetcher : public Prefetcher
{
  private:
    Network net;

    /* Training tables. */
    unordered_map<uint64_t,
                  unordered_map<uint64_t, unique_ptr<training_table_info_t>>>
        training_table;
    unique_ptr<prediction_table_info_t[]> prediction_table;

    cv::Mat mat;
    unique_ptr<int[]> offsets;

    int page_bits;
    int cache_block_bits;

    uint64_t block_mask;
    uint64_t page_mask;

    uint64_t last_addr;

    uint64_t iteration;
    const int iter_freq = 50000;

    /* Training table variables. */
    uint64_t LRU_evict = 0;
    int training_table_PC_len;
    int training_table_page_len;

  private:
    void init_knobs();
    void init_stats();
    void update_pixel_matrix(uint64_t address, bool page_change,
                             int offset_idx);
    vector<int> custom_train(const int epochs);
    vector<int> custom_train_on_potential(vector<vector<float>> &potential);
    vector<vector<float>> poisson_encode(vector<vector<float>> &potential);
    void custom_update_weights(vector<vector<float>> &spike_train, int t);
    float custom_stdp_update(float w, float delta_w);
    float custom_reinforcement_learning(int time);

  public:
    PathfinderPrefetcher(string type);
    ~PathfinderPrefetcher();
    void invoke_prefetcher(uint64_t pc, uint64_t address, uint8_t cache_hit,
                           uint8_t type, vector<uint64_t> &pref_addr);
    void dump_stats();
    void print_config();
};

#endif /* PATHFINDER_H */
