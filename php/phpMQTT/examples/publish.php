<?php

require("../phpMQTT.php");

	
$mqtt = new phpMQTT("192.168.2.10", 1883, "PHP MQTT Client");

if ($mqtt->connect()) {
	$mqtt->publish("bluerhinos/phpMQTT/examples/publishtest","Hello World! at ".date("r"),0);
	$mqtt->close();
}

?>
