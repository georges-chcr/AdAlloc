#include "knapsack.hh"

#include <algorithm>
#include <vector>

std::vector<AllocationResult> solve(const std::vector<Channel>& channels, int budget_eur) {
    const int n = static_cast<int>(channels.size());

    // dp[i][w] = max weighted ROI using first i items with budget w
    std::vector<std::vector<double>> dp(n + 1, std::vector<double>(budget_eur + 1, 0.0));

    for (int i = 1; i <= n; ++i) {
        const Channel& ch = channels[i - 1];
        double value = static_cast<double>(ch.roi_index) * ch.reach_thousands;

        for (int w = 0; w <= budget_eur; ++w) {
            dp[i][w] = dp[i - 1][w];
            if (ch.cost_eur <= w) {
                double with_item = dp[i - 1][w - ch.cost_eur] + value;
                if (with_item > dp[i][w]) {
                    dp[i][w] = with_item;
                }
            }
        }
    }

    // Traceback to find selected items
    std::vector<bool> selected(n, false);
    int w = budget_eur;
    for (int i = n; i >= 1; --i) {
        if (dp[i][w] != dp[i - 1][w]) {
            selected[i - 1] = true;
            w -= channels[i - 1].cost_eur;
        }
    }

    // Build results
    std::vector<AllocationResult> results;
    results.reserve(n);
    for (int i = 0; i < n; ++i) {
        const Channel& ch = channels[i];
        AllocationResult r;
        r.channel_id = ch.id;
        r.selected = selected[i];
        r.roi_obtained = selected[i] ? ch.roi_index * ch.reach_thousands : 0.0f;
        r.budget_used_eur = selected[i] ? ch.cost_eur : 0;
        results.push_back(r);
    }

    return results;
}
