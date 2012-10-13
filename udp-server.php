<?php

$socket = socket_create(AF_INET, SOCK_DGRAM, getprotobyname('udp'));
if( !$socket ) {
  die('Unable to create AF_UNIX socket');
}

if( !socket_bind($socket, '127.0.0.1', 1337) ) {
  die("Unable to bind ");
}

ini_set('loggor.enabled', 0);

while( 1 ) {
  $buf = '';
  $bytes_received = @socket_recv($socket, $buf, 65536, MSG_DONTWAIT);
  if( $bytes_received > 0 ) {
    echo $buf;
    usleep(10000);
  }
}
