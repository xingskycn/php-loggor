--TEST--
loggor - with trigger_error
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
trigger_error('test', E_USER_ERROR);
?>
--EXPECTREGEX--
\{"message"\:\s+".*",\s+"type"\:\s+\d+,\s+"file"\:\s+".+",\s+"line"\:\s+\d+\}.*