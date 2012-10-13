--TEST--
loggor - with trigger_error
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
ini_set('loggor.type_format', LOGGOR_TYPE_INT);
trigger_error('test', E_USER_ERROR);
?>
--EXPECTREGEX--
\{"file"\: ".+?", "hostname"\: ".+?", "line"\: \d+?, "message"\: ".+?", "time"\: [\d.]+?, "type"\: \d+?\}.*