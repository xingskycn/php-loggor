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
\{"file"\: ".+?", "hostname"\: ".+?", "line"\: \d+?, "message"\: ".+?", "time"\: [\d.]+?, "type"\: ".+?"?\}