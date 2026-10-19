-- +goose Up
CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    username TEXT NOT NULL UNIQUE COLLATE NOCASE,
    password_hash TEXT NOT NULL,
    role TEXT NOT NULL,
    instance_scope TEXT NOT NULL DEFAULT '[]',
    locale TEXT NOT NULL DEFAULT '',
    totp_secret TEXT NOT NULL DEFAULT '',
    disabled INTEGER NOT NULL DEFAULT 0,
    created_at INTEGER NOT NULL,
    last_login_at INTEGER
);

CREATE TABLE sessions (
    id TEXT PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    csrf TEXT NOT NULL,
    created_at INTEGER NOT NULL,
    expires_at INTEGER NOT NULL,
    last_seen_at INTEGER NOT NULL,
    ip TEXT NOT NULL DEFAULT '',
    user_agent TEXT NOT NULL DEFAULT '',
    totp_pending INTEGER NOT NULL DEFAULT 0
);
CREATE INDEX sessions_user ON sessions(user_id);

CREATE TABLE api_tokens (
    id INTEGER PRIMARY KEY,
    name TEXT NOT NULL,
    token_hash TEXT NOT NULL UNIQUE,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    permissions TEXT NOT NULL DEFAULT '[]',
    created_at INTEGER NOT NULL,
    last_used_at INTEGER
);

CREATE TABLE audit_log (
    id INTEGER PRIMARY KEY,
    ts INTEGER NOT NULL,
    user_id INTEGER,
    username TEXT NOT NULL,
    ip TEXT NOT NULL DEFAULT '',
    action TEXT NOT NULL,
    instance_id TEXT NOT NULL DEFAULT '',
    params TEXT NOT NULL DEFAULT '{}',
    result TEXT NOT NULL DEFAULT 'ok'
);
CREATE INDEX audit_ts ON audit_log(ts);
CREATE INDEX audit_instance ON audit_log(instance_id, ts);

CREATE TABLE settings (
    section TEXT PRIMARY KEY,
    data TEXT NOT NULL
);

CREATE TABLE secrets (
    key TEXT PRIMARY KEY,
    blob BLOB NOT NULL,
    updated_at INTEGER NOT NULL
);

CREATE TABLE instances (
    id TEXT PRIMARY KEY,
    data TEXT NOT NULL,
    created_at INTEGER NOT NULL,
    updated_at INTEGER NOT NULL
);

CREATE TABLE instance_settings (
    instance_id TEXT NOT NULL REFERENCES instances(id) ON DELETE CASCADE,
    section TEXT NOT NULL,
    data TEXT NOT NULL,
    PRIMARY KEY (instance_id, section)
);

CREATE TABLE instance_schedules (
    instance_id TEXT PRIMARY KEY REFERENCES instances(id) ON DELETE CASCADE,
    data TEXT NOT NULL
);

CREATE TABLE instance_webhooks (
    instance_id TEXT PRIMARY KEY REFERENCES instances(id) ON DELETE CASCADE,
    data TEXT NOT NULL
);

CREATE TABLE instance_firewall_rules (
    instance_id TEXT NOT NULL REFERENCES instances(id) ON DELETE CASCADE,
    kind TEXT NOT NULL,
    port INTEGER NOT NULL,
    rule_name TEXT NOT NULL,
    status TEXT NOT NULL,
    updated_at INTEGER NOT NULL,
    PRIMARY KEY (instance_id, kind)
);

CREATE TABLE mods (
    id TEXT PRIMARY KEY,
    workshop_id INTEGER,
    name TEXT NOT NULL,
    folder_name TEXT NOT NULL DEFAULT '',
    local_path TEXT NOT NULL DEFAULT '',
    is_local INTEGER NOT NULL DEFAULT 0,
    size INTEGER NOT NULL DEFAULT 0,
    hash TEXT NOT NULL DEFAULT '',
    manifest_id TEXT NOT NULL DEFAULT '',
    remote_updated_at INTEGER NOT NULL DEFAULT 0,
    local_updated_at INTEGER NOT NULL DEFAULT 0,
    status TEXT NOT NULL DEFAULT 'pending',
    error TEXT NOT NULL DEFAULT '',
    added_at INTEGER NOT NULL
);

CREATE TABLE instance_mods (
    instance_id TEXT NOT NULL REFERENCES instances(id) ON DELETE CASCADE,
    mod_id TEXT NOT NULL REFERENCES mods(id),
    position INTEGER NOT NULL,
    enabled INTEGER NOT NULL DEFAULT 1,
    server_mod INTEGER NOT NULL DEFAULT 0,
    PRIMARY KEY (instance_id, mod_id)
);

CREATE TABLE instance_mod_sync (
    instance_id TEXT NOT NULL REFERENCES instances(id) ON DELETE CASCADE,
    mod_id TEXT NOT NULL,
    folder TEXT NOT NULL,
    hash TEXT NOT NULL,
    synced_at INTEGER NOT NULL,
    PRIMARY KEY (instance_id, mod_id)
);

CREATE TABLE jobs (
    id TEXT PRIMARY KEY,
    type TEXT NOT NULL,
    instance_id TEXT NOT NULL DEFAULT '',
    status TEXT NOT NULL,
    progress REAL NOT NULL DEFAULT 0,
    message TEXT NOT NULL DEFAULT '',
    params TEXT NOT NULL DEFAULT '{}',
    error TEXT NOT NULL DEFAULT '',
    created_by TEXT NOT NULL DEFAULT '',
    created_at INTEGER NOT NULL,
    started_at INTEGER,
    finished_at INTEGER
);
CREATE INDEX jobs_created ON jobs(created_at);
CREATE INDEX jobs_status ON jobs(status);

CREATE TABLE job_logs (
    id INTEGER PRIMARY KEY,
    job_id TEXT NOT NULL REFERENCES jobs(id) ON DELETE CASCADE,
    ts INTEGER NOT NULL,
    line TEXT NOT NULL
);
CREATE INDEX job_logs_job ON job_logs(job_id);

CREATE TABLE game_versions (
    build INTEGER PRIMARY KEY,
    version TEXT NOT NULL,
    prev_build INTEGER,
    files INTEGER NOT NULL,
    delta_files INTEGER NOT NULL DEFAULT 0,
    deleted_files INTEGER NOT NULL DEFAULT 0,
    has_delta INTEGER NOT NULL DEFAULT 0,
    created_at INTEGER NOT NULL
);

CREATE TABLE kv (
    instance_id TEXT NOT NULL,
    key TEXT NOT NULL,
    value TEXT NOT NULL,
    expires_at INTEGER,
    PRIMARY KEY (instance_id, key)
);

CREATE TABLE host_metrics (
    ts INTEGER NOT NULL,
    cpu REAL,
    ram REAL
);
CREATE INDEX host_metrics_ts ON host_metrics(ts);

CREATE TABLE deliveries (
    id INTEGER PRIMARY KEY,
    ts INTEGER NOT NULL,
    instance_id TEXT NOT NULL DEFAULT '',
    event TEXT NOT NULL DEFAULT '',
    data TEXT NOT NULL
);
CREATE INDEX deliveries_ts ON deliveries(ts);
CREATE INDEX deliveries_inst ON deliveries(instance_id, ts);

-- +goose Down
SELECT 1;
