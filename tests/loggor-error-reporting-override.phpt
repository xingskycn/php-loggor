--TEST--
loggor - error reporting override
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
error_reporting(0);
ini_set('loggor.error_reporting', E_ALL | E_NOTICE);
trigger_error('test', E_NOTICE);
?>
--EXPECTREGEX--
