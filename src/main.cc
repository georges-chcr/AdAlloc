#include <iostream>
#include <vector>
#include <string>
#include <sqlite3.h>
#include "knapsack.hh"
#include "db_writer.hh"

int main() {
    sqlite3* db = nullptr;
    if (sqlite3_open("adalloc.db", &db) != SQLITE_OK) {
        std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    if (!initDB(db)) {
        std::cerr << "Failed to initialize schema\n";
        sqlite3_close(db);
        return 1;
    }

    std::vector<Channel> channels;
    if (!loadChannels(db, "data/channels.csv", channels)) {
        std::cerr << "Failed to load channels\n";
        sqlite3_close(db);
        return 1;
    }

    struct Scenario { std::string label; int budget; };
    const Scenario scenarios[] = {
        {"Conservative",  50000},
        {"Standard",     100000},
        {"Aggressive",   150000},
    };

    for (const auto& s : scenarios) {
        int scenario_id = createScenario(db, s.label, s.budget);
        if (scenario_id < 0) {
            std::cerr << "Failed to create scenario '" << s.label << "'\n";
            sqlite3_close(db);
            return 1;
        }

        auto results = solve(channels, s.budget);

        if (!saveResults(db, scenario_id, results)) {
            std::cerr << "Failed to save results for scenario '" << s.label << "'\n";
            sqlite3_close(db);
            return 1;
        }

        float total_roi = 0.0f;
        int budget_used = 0;
        std::vector<std::string> selected;
        for (const auto& r : results) {
            if (r.selected) {
                total_roi += r.roi_obtained;
                budget_used += r.budget_used_eur;
                for (const auto& ch : channels) {
                    if (ch.id == r.channel_id) {
                        selected.push_back(ch.name);
                        break;
                    }
                }
            }
        }

        std::cout << "=== " << s.label << " (budget: " << s.budget << " EUR) ===\n";
        std::cout << "  Total ROI: " << total_roi << "\n";
        std::cout << "  Budget used: " << budget_used << " EUR\n";
        std::cout << "  Channels selected (" << selected.size() << "):\n";
        for (const auto& name : selected) {
            std::cout << "    - " << name << "\n";
        }
        std::cout << "\n";
    }

    sqlite3_close(db);
    return 0;
}
