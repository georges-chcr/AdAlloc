#pragma once

#include <string>
#include <vector>

struct Channel {
    int id;
    std::string name;
    int cost_eur;
    int reach_thousands;
    float roi_index;
    std::string category;
};

struct AllocationResult {
    int channel_id;
    bool selected;
    float roi_obtained;
    int budget_used_eur;
};

std::vector<AllocationResult> solve(const std::vector<Channel>& channels, int budget_eur);
