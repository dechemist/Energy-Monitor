#!/usr/bin/env python
import serial
import string
import time
import datetime as dt
from datetime import datetime
import sqlite3
import sys, os
path_to_db = '/var/www/html/phpliteadmin/energy_monitor.db'
tbl_name = 'current_power_log'
tbl_name_stat = 'power_log'

class Pysql:
	def __init__(self, db_file, tbl_name):
		self.tbl_name = tbl_name
		self.con = sqlite3.connect(db_file)
		self.cur = self.con.cursor()

	def SQLwriteData (self,tbl_name,date_now, v, i, rp, ap):
		self.cur.execute("insert into "+tbl_name+" (timestamp, vrms, irms, real_power, apparent_power) values ('"+date_now+"','"+v+"','"+i+"','"+rp+"','0')")	
		self.con.commit()
	
	def SQLTblexists (self, tbl_name):
		tbl_result = self.cur.execute("SELECT name FROM sqlite_master WHERE name = '"+tbl_name+"'")
		if tbl_result.fetchone() == None:
			return False
		else:
			return True
			
	def SQLquery(self,q):
		return self.cur.execute(q)
			
	def SQLcountRows (self, tbl_name):
		row_count = self.cur.execute("SELECT count(id) FROM "+tbl_name+"")	
		return self.cur.fetchone()
		
	def SQLaverageAndempty (self, tbl_name, tbl_name_stat):
		count = self.cur.execute("SELECT count(id) FROM "+tbl_name+"")
		row_count = self.cur.fetchone()
		if (row_count[0] >= 200):
			#average datapoints
			self.cur.execute("SELECT avg(vrms),avg(irms),avg(real_power),avg(apparent_power) FROM "+tbl_name+"")	
			avg_values = self.cur.fetchone()
			v = str(round(avg_values[0],1))
			i = str(round(avg_values[1],1))
			rp = str(round(avg_values[2],1))
			ap = str(round(avg_values[3],1))
			date_now = str(datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
			#write average
			self.cur.execute("insert into "+tbl_name_stat+" (timestamp, vrms, irms, real_power, apparent_power) values ('"+date_now+"','"+v+"','"+i+"','"+rp+"','"+ap+"')")
			self.con.commit()
			#empty table
			self.cur.execute("delete from "+tbl_name)
			self.cur.execute("VACUUM")
			self.con.commit()
			
	def quit(self):
		self.con.close()
	
try:
	query = Pysql(path_to_db,tbl_name)
	if len(sys.argv) > 1:
		if sys.argv[1] == "-log":
			while (1):
				ser = serial.Serial ("/dev/ttyAMA0")
				ser.baudrate = 9600
				query.SQLaverageAndempty(tbl_name, tbl_name_stat)
				data = ser.readline()
				values = data[:len(data)-4].split('|')
				query.SQLwriteData(tbl_name,str(datetime.now().strftime("%Y-%m-%d %H:%M:%S")), str(values[0]),str(values[1]),str(values[2]),"0")
		
except KeyboardInterrupt:
	ser.close()
	exit