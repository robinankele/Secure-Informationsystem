PRAGMA foreign_keys=ON;
BEGIN TRANSACTION;

CREATE TABLE DataTypes (
  type  TEXT PRIMARY KEY NOT NULL
);

INSERT INTO DataTypes(type) VALUES
  ('Int64'),
  ('Double'),
  ('String'),
  ('Blob');

CREATE TABLE KeyInfo (
  id        INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
  domain    TEXT,
  key       TEXT    NOT NULL,
  datatype  TEXT    NOT NULL,
  UNIQUE(domain, key),
  FOREIGN KEY(datatype) REFERENCES DataTypes(type)
);

CREATE TABLE ValueInt64 (
  id    INTEGER PRIMARY KEY NOT NULL,
  value INTEGER NOT NULL,
  FOREIGN KEY(id) REFERENCES KeyInfo(id)
);

CREATE TABLE ValueDouble (
  id    INTEGER PRIMARY KEY NOT NULL,
  value REAL NOT NULL,
  FOREIGN KEY(id) REFERENCES KeyInfo(id)
);

CREATE TABLE ValueString (
  id    INTEGER PRIMARY KEY NOT NULL,
  value TEXT NOT NULL,
  FOREIGN KEY(id) REFERENCES KeyInfo(id)
);

CREATE TABLE ValueBlob (
  id    INTEGER PRIMARY KEY NOT NULL,
  path TEXT NOT NULL,
  FOREIGN KEY(id) REFERENCES KeyInfo(id)
);

INSERT INTO KeyInfo(domain, key, datatype) VALUES(NULL, 'blob-path', 'String');
INSERT INTO ValueString(id, value) VALUES(last_insert_rowid(), '/tmp');

COMMIT;
