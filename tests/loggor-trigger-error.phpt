--TEST--
loggor test
--SKIPIF--
<?php 

if(!extension_loaded('loggor')) die('skip ');
 ?>
--FILE--
<?php
trigger_error('test', E_USER_ERROR);
?>
--EXPECT--
testing