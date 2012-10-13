--TEST--
loggor - udp support
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
ini_set('loggor.udp.enabled', 1);
ini_set('loggor.udp.host', '127.0.0.1');
ini_set('loggor.udp.port', 1337);
trigger_error('test', E_USER_ERROR);
?>
--EXPECTREGEX--
\{"file"\: ".+?", "hostname"\: ".+?", "line"\: \d+?, "message"\: ".+?", "time"\: [\d.]+?, "type"\: \d+?\}.*