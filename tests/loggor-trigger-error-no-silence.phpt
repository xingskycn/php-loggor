--TEST--
loggor - with trigger_error, with no silence
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
ini_set('loggor.silence', 0);
trigger_error('test', E_USER_ERROR);
?>
--EXPECTREGEX--
\{"file"\: ".+?", "hostname"\: ".+?", "line"\: \d+?, "message"\: ".+?", "time"\: [\d.]+?, "type"\: ".+?"?\}\s+Fatal error: test.*