PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS channels (
    id               INTEGER PRIMARY KEY,
    name             TEXT    NOT NULL UNIQUE,
    cost_eur         INTEGER NOT NULL,
    reach_thousands  INTEGER NOT NULL,
    roi_index        REAL    NOT NULL,
    category         TEXT    NOT NULL
);

CREATE TABLE IF NOT EXISTS scenarios (
    id                INTEGER PRIMARY KEY AUTOINCREMENT,
    label             TEXT    NOT NULL,
    total_budget_eur  INTEGER NOT NULL,
    created_at        TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ', 'now'))
);

CREATE TABLE IF NOT EXISTS allocation_results (
    id              INTEGER PRIMARY KEY AUTOINCREMENT,
    scenario_id     INTEGER NOT NULL REFERENCES scenarios(id) ON DELETE CASCADE,
    channel_id      INTEGER NOT NULL REFERENCES channels(id)  ON DELETE RESTRICT,
    selected        INTEGER NOT NULL CHECK (selected IN (0, 1)),
    roi_obtained    REAL    NOT NULL,
    budget_used_eur INTEGER NOT NULL
);

CREATE INDEX IF NOT EXISTS idx_allocation_results_scenario_id
    ON allocation_results (scenario_id);
