<?php
	//LIVE
	$dbhandle = new SQLite3('../phpliteadmin/energy_monitor.db');
	$dbhandle->busyTimeout(3000);
	$tbl_name = 'current_power_log';
	$time_interval_h_min = 1;
	$num_entries = 0;
	$get = $dbhandle->query("select round((real_power*".$time_interval_h_min."),2) from ".$tbl_name);
	$wh = array();
	while($data = $get->fetchArray())
	{
		$num_entries++;
		$wh[] = $data[0];
	}
	echo "{"."\r\n";
	echo "\"Wh\": [";
	for ($i = 0;$i < $num_entries;$i++)
	{
		if ($i == $num_entries-1) { echo $wh[$i]; } else { echo $wh[$i].','; }
	}
	echo "]}";
?>