--TEST--
loggor - with parse error
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
as(I$#@*)
?>
--EXPECTREGEX--
\{"message"\:\s+".*",\s+"type"\:\s+\d+,\s+"file"\:\s+".+",\s+"line"\:\s+\d+\}.*