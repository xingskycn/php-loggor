--TEST--
loggor - with error_log
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
error_log('test');
?>
--EXPECTREGEX--
\{"file"\: ".+?", "hostname"\: ".+?", "line"\: \d+?, "message"\: ".+?", "time"\: [\d.]+?, "type"\: ".+?"?\}.*
--XFAIL--
error_log doesn't get handled by the callback