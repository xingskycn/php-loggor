--TEST--
loggor - with uncaught exception
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
throw new Exception('testing');
?>
--EXPECTREGEX--
\{"message"\:\s+".*",\s+"type"\:\s+\d+,\s+"file"\:\s+".+",\s+"line"\:\s+\d+\}.*