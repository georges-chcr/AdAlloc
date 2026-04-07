#include "db_writer.hh"

#include <fstream>
#include <sstream>

bool initDB(sqlite3* db) {
    std::ifstream f("sql/schema.sql");
    if (!f.is_open()) return false;

    std::ostringstream ss;
    ss << f.rdbuf();
    const std::string sql = ss.str();

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        sqlite3_free(errmsg);
        return false;
    }
    return true;
}

bool loadChannels(sqlite3* db, const std::string& csv_path, std::vector<Channel>& out) {
    std::ifstream f(csv_path);
    if (!f.is_open()) return false;

    const char* sql =
        "INSERT OR IGNORE INTO channels (id, name, cost_eur, reach_thousands, roi_index, category) "
        "VALUES (?, ?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    std::string line;
    std::getline(f, line); // skip header

    while (std::getline(f, line)) {
        if (line.empty()) continue;

        std::istringstream row(line);
        std::string tok;
        Channel ch;

        std::getline(row, tok, ',');  ch.id              = std::stoi(tok);
        std::getline(row, ch.name, ',');
        std::getline(row, tok, ',');  ch.cost_eur        = std::stoi(tok);
        std::getline(row, tok, ',');  ch.reach_thousands = std::stoi(tok);
        std::getline(row, tok, ',');  ch.roi_index       = std::stof(tok);
        std::getline(row, ch.category);

        sqlite3_reset(stmt);
        sqlite3_bind_int   (stmt, 1, ch.id);
        sqlite3_bind_text  (stmt, 2, ch.name.c_str(),     -1, SQLITE_TRANSIENT);
        sqlite3_bind_int   (stmt, 3, ch.cost_eur);
        sqlite3_bind_int   (stmt, 4, ch.reach_thousands);
        sqlite3_bind_double(stmt, 5, ch.roi_index);
        sqlite3_bind_text  (stmt, 6, ch.category.c_str(), -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return false;
        }

        out.push_back(ch);
    }

    sqlite3_finalize(stmt);
    return true;
}

int createScenario(sqlite3* db, const std::string& label, int budget_eur) {
    const char* sql =
        "INSERT INTO scenarios (label, total_budget_eur) VALUES (?, ?)";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, label.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int (stmt, 2, budget_eur);

    const int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE) return -1;
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

bool saveResults(sqlite3* db, int scenario_id, const std::vector<AllocationResult>& results) {
    if (sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr) != SQLITE_OK)
        return false;

    const char* sql =
        "INSERT INTO allocation_results (scenario_id, channel_id, selected, roi_obtained, budget_used_eur) "
        "VALUES (?, ?, ?, ?, ?)";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        return false;
    }

    for (const auto& r : results) {
        sqlite3_reset(stmt);
        sqlite3_bind_int   (stmt, 1, scenario_id);
        sqlite3_bind_int   (stmt, 2, r.channel_id);
        sqlite3_bind_int   (stmt, 3, r.selected ? 1 : 0);
        sqlite3_bind_double(stmt, 4, r.roi_obtained);
        sqlite3_bind_int   (stmt, 5, r.budget_used_eur);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
            return false;
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    return true;
}
