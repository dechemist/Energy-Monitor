----
-- phpLiteAdmin database dump (http://phpliteadmin.googlecode.com)
-- phpLiteAdmin version: 1.9.5
-- Exported: 9:58pm on September 7, 2015 (CEST)
-- database file: ./energy_monitor.db
----
BEGIN TRANSACTION;

----
-- Table structure for power_log
----
CREATE TABLE 'power_log' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, 'timestamp' DATETIME DEFAULT CURRENT_TIMESTAMP,'vrms' TEXT,'irms' TEXT,'real_power' TEXT,'apparent_power' TEXT);

----
-- Table structure for current_power_log
----
CREATE TABLE 'current_power_log' ('id' INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, 'timestamp' DATETIME DEFAULT CURRENT_TIMESTAMP,'vrms' TEXT,'irms' TEXT,'real_power' TEXT,'apparent_power' TEXT);
COMMIT;
