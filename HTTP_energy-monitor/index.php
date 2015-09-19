<html>
<head>
<!-- Load c3.css -->
<link href="c3.css" rel="stylesheet" type="text/css">
<!-- Load d3.js and c3.js -->
<script src="d3.min.js" charset="utf-8"></script>
<script src="c3.min.js"></script>
<meta charset="utf-8">
<title>Energie-Monitor</title>
</head>
<body onload="setInterval('auto_reload();', 3000)">
<center>
<?php
	//INIT DATABASE - Change Database path etc. here !
	$dbhandle = new SQLite3('../phpliteadmin/energy_monitor.db');
	//	
	$tbl_name = 'power_log';
	$Ct_kwh = 0.23;
	$time_interval_h_min = 0.1666; # 10/60 min
	$dbhandle->busyTimeout(3000);
	$get = $dbhandle->query("SELECT round(sum(real_power*".$time_interval_h_min."),2) FROM ".$tbl_name." where date(timestamp) = date('now') limit 1");
	$data = $get->fetchArray();
	print "Today: ".round($data[0]/1000,2)." kWh (".round($data[0]/1000*$Ct_kwh,1)." Euro)<br>";
	$get = $dbhandle->query("SELECT round(sum(real_power*".$time_interval_h_min."),2) FROM ".$tbl_name." limit 1");
	$data = $get->fetchArray();
	print "Overall: ".round($data[0]/1000,2)." kWh (".round($data[0]/1000*$Ct_kwh,1)." Euro)<br>";
?>
<h2>Today - Live !</h2>
<a href="live.html" style="color: #DCDCDD;">>>>Live<<<</a><br>
<p>
<h2>Today - hours</h2><br>
<div id="chart1"></div>
<h2>Week - Days</h2><br>
<div id="chart2"></div>
<h2>Month</h2><br>
<div id="chart3"></div>
<script>
<?php
	//HOURS TODAY FILTER
	$get = $dbhandle->query("select strftime('%H',timestamp), round(sum(real_power*".$time_interval_h_min."),2) from ".$tbl_name." where date(timestamp) = date('now') group by strftime('%H', timestamp)");
	$num_entries = 0;
	unset($real_power);
	unset($x_tick_label);
	while($data = $get->fetchArray())
	{
		$x_tick_label[] = $data[0];	
		$real_power[] = $data[1];
		$num_entries++;
	}
?>
var chart = c3.generate({
	size: {
        width: 750
    },
    bindto: '#chart1',
    data: {
		x: 'x',
		columns: [
		['x',<?php $i = 0; for ($i; $i < $num_entries; $i++){ if ($i == $num_entries-1) echo "'".$x_tick_label[$i]."'"; else echo "'".$x_tick_label[$i]."'".','; } ?>],
		['Wh', <?php $i = 0; for ($i; $i < $num_entries; $i++){ if ($i == $num_entries-1) echo $real_power[$i]; else echo $real_power[$i].','; } ?>]
		],
		types: { Wh: 'bar'},
		colors: {Wh: '#1965A1'}
    },
	bar: { width: 20 },
	legend: {
        show: false	
	},
	axis: {
        x: {
            label: {
                text: 'time',
                position: 'outer-center'
            },
        },
        y: {
            label: {
                text: 'Wh',
                position: 'outer-middle'
            }
        }
	}
});

<?php
	//DAYS FILTER 
	$get = $dbhandle->query("select strftime('%w',timestamp), round(sum(real_power*".$time_interval_h_min."),2) from ".$tbl_name." where timestamp >= date('now','-6 days') group by strftime('%d',timestamp) order by strftime('%Y-%m-%d',timestamp) asc ");
	$num_entries = 0;
	$days = array('Sun','Mon','Tue','Wed','Thu','Fri','Sat'); 
	unset($real_power);
	unset($x_tick_label);
	while($data = $get->fetchArray())
	{
		$x_tick_label[] = $days[$data[0]];	
		$real_power[] = $data[1];
		$num_entries++;
	}
?>
var chart = c3.generate({
	size: {
		width: 750
    },
    bindto: '#chart2',
    data: {
		columns: [
		['Wh', <?php $i = 0; for ($i; $i < $num_entries; $i++){ if ($i == $num_entries-1) echo $real_power[$i]; else echo $real_power[$i].','; } ?>]
		],
	  	types: { Wh: 'bar'},
		colors: {Wh: '#1965A1'}
    },
	bar: { width: 20 },
	legend: { show: false },
	axis: {
        x: {
			type: 'category',
            categories: [<?php $i = 0; for ($i; $i < $num_entries; $i++){ if ($i == $num_entries-1) echo "'".$x_tick_label[$i]."'"; else echo "'".$x_tick_label[$i]."'".','; } ?>],

		   label: {
                text: 'days',
                position: 'outer-center'
            },
			 tick: {
				fit: true,
				show: false
			}
        },
        y: {
            label: {
                text: 'Wh',
                position: 'outer-middle'
            }
        }
	}
});
<?php 
	$get = $dbhandle->query("SELECT strftime('%Y-%m-%d',timestamp), round(sum(real_power*".$time_interval_h_min."),2) FROM ".$tbl_name." where timestamp >= datetime('now','start of month') group by strftime('%d', timestamp)");
	$num_entries = 0;
	unset($real_power);
	unset($x_tick_label);
	while($data = $get->fetchArray())
	{
		$x_tick_label[] = $data[0];
		$real_power[] = $data[1];
		$num_entries++;
	}
?>
var chart = c3.generate({
	size: {
        width: 750
    },
    bindto: '#chart3',
    data: {
		x: 'x',
		columns: [
		['x',<?php $i = 0; for ($i; $i < $num_entries; $i++){ if ($i == $num_entries-1) echo "'".$x_tick_label[$i]."'"; else echo "'".$x_tick_label[$i]."'".','; } ?>],
		['Wh', <?php $i = 0; for ($i; $i < $num_entries; $i++){ if ($i == $num_entries-1) echo $real_power[$i]; else echo $real_power[$i].','; } ?>]
		],
		types: {Wh: 'bar'},
		colors: { Wh: '#1965A1'}
    },
	bar: { width: 20 },
	legend: { show: false},
	axis: {
        x: {
			type : 'timeseries',
            tick: {
                fit: true,
                format: "%d"
            },
            label: {
                text: 'time',
                position: 'outer-center'
            },
        },
        y: {
            label: {
                text: 'Wh',
                position: 'outer-middle'
            }
        }
	}
});
</script>
</center>
</body>
</html>