<?php

require("phpMQTT.php");

if ($_GET['time'] == "t") {
        $time=gmdate("y/m/d,H:i:s+00\n", time());
        echo $time;
}

if ($_GET['topic'] == "") {
	die("please set topic");
} else {
$topic = $_GET['topic'];
}

if ($_GET['payload'] == "") {
        die("please set payload");
} else {
$payload = $_GET['payload'];
}

if ($_GET['user'] == "") {
	echo "user not set\n";
	$user=NULL;
} else {
$user = $_GET['user'];
}

if ($_GET['password'] == "") {
	echo "password not set\n";
	$password=NULL;
} else {
$password = $_GET['password'];
}

	
$mqtt = new phpMQTT("localhost", 1883, "PHP MQTT Client");


if ($mqtt->connect(true,NULL,$user,$password)) {
	$mqtt->publish($topic,$payload,0);
	$mqtt->close();
	echo "okay";
} else {
	die("not connected");
}

?>