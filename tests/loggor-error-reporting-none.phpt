--TEST--
loggor - error reporting none
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
error_reporting(0);
trigger_error('test', E_USER_NOTICE);
?>
--EXPECT--
