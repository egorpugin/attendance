/*
Navicat SQLite Data Transfer

Source Server         : db_copy
Source Server Version : 31300
Source Host           : :0

Target Server Type    : SQLite
Target Server Version : 31300
File Encoding         : 65001

Date: 2018-03-14 19:59:18
*/

PRAGMA foreign_keys = OFF;

-- ----------------------------
-- Table structure for classes
-- ----------------------------
DROP TABLE IF EXISTS "main"."classes";
CREATE TABLE classes (
    lesson_id INTEGER PRIMARY KEY,
    teacher_id INTEGER REFERENCES person (person_id) ON DELETE CASCADE ON UPDATE CASCADE,
    subject_id INTEGER REFERENCES subject (subject_id) ON DELETE CASCADE ON UPDATE CASCADE
);

-- ----------------------------
-- Table structure for groups
-- ----------------------------
DROP TABLE IF EXISTS "main"."groups";
CREATE TABLE groups (
    group_id INTEGER PRIMARY KEY AUTOINCREMENT,
    group_name TEXT NOT NULL,
    type_id INTEGER DEFAULT (1) NOT NULL,
    UNIQUE ("group_name")
);

-- ----------------------------
-- Table structure for journal
-- ----------------------------
DROP TABLE IF EXISTS "main"."journal";
CREATE TABLE journal (
    record_id INTEGER PRIMARY KEY,
    person_id INTEGER REFERENCES person (person_id) ON DELETE CASCADE ON UPDATE CASCADE,
    timestamp INTEGER NOT NULL DEFAULT (0)
);

-- ----------------------------
-- Table structure for person
-- ----------------------------
DROP TABLE IF EXISTS "main"."person";
CREATE TABLE person (
    person_id  INTEGER,
    login  TEXT NOT NULL,
    password  TEXT NOT NULL,
    salt  TEXT NOT NULL,
    cookie  TEXT,
    cookie_expires  TEXT,
    surname  TEXT NOT NULL,
    first_name  TEXT NOT NULL,
    middle_name  TEXT NOT NULL,
    type_id  INTEGER NOT NULL DEFAULT (1),
    PRIMARY KEY ("person_id"),
    UNIQUE ("login"),
    UNIQUE ("cookie")
);

-- ----------------------------
-- Table structure for person_group
-- ----------------------------
DROP TABLE IF EXISTS "main"."person_group";
CREATE TABLE person_group (
    person_group_id INTEGER PRIMARY KEY,
    person_id INTEGER REFERENCES person (person_id) ON DELETE CASCADE ON UPDATE CASCADE,
    group_id INTEGER REFERENCES "group" (group_id) ON DELETE CASCADE ON UPDATE CASCADE
);


-- ----------------------------
-- Table structure for subject
-- ----------------------------
DROP TABLE IF EXISTS "main"."subject";
CREATE TABLE subject (
    subject_id INTEGER PRIMARY KEY,
    subject_name TEXT NOT NULL
);
PRAGMA foreign_keys = ON;

CREATE TABLE course (
    course_id INTEGER PRIMARY KEY,
    subject_id INTEGER REFERENCES subject (subject_id) NOT NULL,
    teacher_id INTEGER REFERENCES person (person_id) NOT NULL,
    group_id INTEGER REFERENCES groups (group_id) NOT NULL,
    semester INTEGER NOT NULL
);
