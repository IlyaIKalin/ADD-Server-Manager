-- +goose Up
CREATE TABLE players (
    steam_id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    first_seen INTEGER NOT NULL,
    last_seen INTEGER NOT NULL,
    total_joins INTEGER NOT NULL DEFAULT 0,
    total_seconds INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE player_identities (
    be_guid TEXT PRIMARY KEY,
    steam_id TEXT NOT NULL DEFAULT '',
    bis_id TEXT NOT NULL DEFAULT '',
    name TEXT NOT NULL DEFAULT '',
    updated_at INTEGER NOT NULL
);
CREATE INDEX player_identities_steam ON player_identities(steam_id);

CREATE TABLE bans_global (
    id INTEGER PRIMARY KEY,
    guid TEXT NOT NULL DEFAULT '',
    ip TEXT NOT NULL DEFAULT '',
    steam_id TEXT NOT NULL DEFAULT '',
    reason TEXT NOT NULL DEFAULT '',
    expires_at INTEGER,
    created_by TEXT NOT NULL,
    created_at INTEGER NOT NULL
);

CREATE TABLE player_sessions (
    id TEXT PRIMARY KEY,
    instance_id TEXT NOT NULL,
    steam_id TEXT NOT NULL DEFAULT '',
    be_guid TEXT NOT NULL DEFAULT '',
    bis_id TEXT NOT NULL DEFAULT '',
    name TEXT NOT NULL,
    ip TEXT NOT NULL DEFAULT '',
    source TEXT NOT NULL,
    started_at INTEGER NOT NULL,
    ended_at INTEGER
);
CREATE INDEX player_sessions_i ON player_sessions(instance_id, started_at);
CREATE INDEX player_sessions_s ON player_sessions(steam_id, started_at);

CREATE TABLE player_names (
    steam_id TEXT NOT NULL,
    name TEXT NOT NULL,
    first_seen INTEGER NOT NULL,
    last_seen INTEGER NOT NULL,
    PRIMARY KEY (steam_id, name)
);

CREATE TABLE player_ips (
    steam_id TEXT NOT NULL,
    ip TEXT NOT NULL,
    country TEXT NOT NULL DEFAULT '',
    first_seen INTEGER NOT NULL,
    last_seen INTEGER NOT NULL,
    PRIMARY KEY (steam_id, ip)
);
