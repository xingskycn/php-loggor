php-loggor
==========

Intercepts errors and encodes them using JSON. Can be configured to send out 
errors over UDP. Can intercept and handle fatal errors and parse errors, unlike 
set_error_handler(). Also overloads the error_log function such that the message
is also run through the encoder.

Requirements
--------------------------------------------------------------------------------

[Jansson](https://github.com/akheron/jansson) - JSON library

Installation
--------------------------------------------------------------------------------

```bash
apt-get install git-core php5-dev
```

```bash
git clone git://github.com/akheron/jansson.git
cd jansson
autoreconf -i
./configure
make
make install
```

```bash
git clone git://github.com/jbboehr/php-loggor.git
cd php-loggor
phpize
./configure
make
make install
```

Configuring
--------------------------------------------------------------------------------

```ini
loggor.enabled = 1        ; 0 to disable all functionality
loggor.type_format = 3    ; Changes format of error type, one of LOGGOR_TYPE_INT, 
                          ; LOGGOR_TYPE_CONST, LOGGOR_TYPE_SIMPLE, LOGGOR_TYPE_STRING
loggor.php.enabled = 1    ; 0 to disable logging to PHP default error handler
loggor.udp.enabled = 0    ; 1 to enable logging to UDP
loggor.udp.host =         ; UDP host to which to send data
loggor.udp.port =         ; UDP port to which to send data
```

Usage
--------------------------------------------------------------------------------

Example:

```php
<?php
trigger_error("Some error", E_USER_NOTICE);
```

Produces:

```text
{"file": "some-file.php", "hostname": "some-hostname", "line": 1337, "message": "Some error", "time": 1350097238.695085, "type": "Notice"}
```

Todo
--------------------------------------------------------------------------------

* Resolve segfaults when default error handler is not called so a silence option can be added
* Add getrusage() support and other analytics?
