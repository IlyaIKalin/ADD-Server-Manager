-- +goose Up
-- One journal database per server. Every table has ts (Unix ms) so the daily purge
-- can delete one calendar date through the ts indexes.
CREATE TABLE events (
    id INTEGER PRIMARY KEY,
    ts INTEGER NOT NULL,
    cat TEXT NOT NULL,
    type TEXT NOT NULL DEFAULT '',
    steam_id TEXT NOT NULL DEFAULT '',
    name TEXT NOT NULL DEFAULT '' COLLATE NOCASE,
    other_id TEXT NOT NULL DEFAULT '',
    class TEXT NOT NULL DEFAULT '' COLLATE NOCASE,
    pid TEXT NOT NULL DEFAULT '',
    x REAL,
    z REAL,
    data TEXT NOT NULL DEFAULT '{}'
);
CREATE INDEX events_ts ON events(ts);
CREATE INDEX events_steam ON events(steam_id, ts);
CREATE INDEX events_steam_cat ON events(steam_id, cat, ts);
CREATE INDEX events_other ON events(other_id, ts);
CREATE INDEX events_cat ON events(cat, ts);
CREATE INDEX events_pid ON events(pid, ts);
CREATE INDEX events_class ON events(class, ts);
CREATE INDEX events_name ON events(name, ts);

CREATE TABLE positions (
    ts INTEGER NOT NULL,
    steam_id TEXT NOT NULL,
    x REAL NOT NULL,
    y REAL NOT NULL,
    z REAL NOT NULL
);
CREATE INDEX positions_ts ON positions(ts);
CREATE INDEX positions_steam ON positions(steam_id, ts);

-- data is flate-compressed JSON. eid is "part/parts" for timed snapshots and the
-- entity id for entity changes; keyframes use kind "kf:<entity kind>".
CREATE TABLE snapshots (
    id INTEGER PRIMARY KEY,
    ts INTEGER NOT NULL,
    kind TEXT NOT NULL,
    eid TEXT NOT NULL DEFAULT '',
    data BLOB NOT NULL
);
CREATE INDEX snapshots_ts ON snapshots(ts);
CREATE INDEX snapshots_kind ON snapshots(kind, ts);
CREATE INDEX snapshots_eid ON snapshots(kind, eid, ts);

CREATE TABLE metrics (
    ts INTEGER NOT NULL,
    cpu REAL,
    ram REAL,
    players INTEGER,
    fps REAL
);
CREATE INDEX metrics_ts ON metrics(ts);

CREATE TABLE reports (
    id TEXT PRIMARY KEY,
    ts INTEGER NOT NULL,
    reporter_steam_id TEXT NOT NULL,
    reporter_name TEXT NOT NULL DEFAULT '',
    target_steam_id TEXT NOT NULL DEFAULT '',
    target_name TEXT NOT NULL DEFAULT '',
    category TEXT NOT NULL,
    message TEXT NOT NULL DEFAULT '',
    from_death_screen INTEGER NOT NULL DEFAULT 0,
    x REAL, y REAL, z REAL,
    data TEXT NOT NULL DEFAULT '{}',
    status TEXT NOT NULL DEFAULT 'new',
    comment TEXT NOT NULL DEFAULT '',
    assignee TEXT NOT NULL DEFAULT ''
);
CREATE INDEX reports_ts ON reports(ts);

CREATE TABLE meta (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

-- +goose Down
SELECT 1;
