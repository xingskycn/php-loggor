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
\{"message"\:\s+".*",\s+"type"\:\s+\d+,\s+"file"\:\s+".+",\s+"line"\:\s+\d+\}.*
--XFAIL--
error_log doesn't get handled by the callback