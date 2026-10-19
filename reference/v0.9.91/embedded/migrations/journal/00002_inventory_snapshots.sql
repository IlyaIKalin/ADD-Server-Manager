-- +goose Up
-- Last inventory snapshots used to restore a player's gear. Kept outside the daily
-- journal purge: each player retains only the ten newest rows.
CREATE TABLE inventory_snapshots (
    id INTEGER PRIMARY KEY,
    ts INTEGER NOT NULL,
    steam_id TEXT NOT NULL,
    name TEXT NOT NULL DEFAULT '',
    source TEXT NOT NULL,
    data TEXT NOT NULL
);
CREATE INDEX inventory_snapshots_steam ON inventory_snapshots(steam_id, ts);

-- +goose Down
DROP TABLE inventory_snapshots;
