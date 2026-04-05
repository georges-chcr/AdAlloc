#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>
#include "knapsack.hh"

bool initDB(sqlite3* db);
bool loadChannels(sqlite3* db, const std::string& csv_path, std::vector<Channel>& out);
int  createScenario(sqlite3* db, const std::string& label, int budget_eur);
bool saveResults(sqlite3* db, int scenario_id, const std::vector<AllocationResult>& results);
