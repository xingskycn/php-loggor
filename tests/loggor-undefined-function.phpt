--TEST--
loggor - with undefined function
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
thisFunctionShouldNotExist();
?>
--EXPECTREGEX--
\{"message"\:\s+".*",\s+"type"\:\s+\d+,\s+"file"\:\s+".+",\s+"line"\:\s+\d+\}.*