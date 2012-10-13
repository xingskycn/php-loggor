php-loggor
==========

Intercepts errors and encodes them using JSON. Can be configured to send out 
errors over UDP. Can intercept and handle fatal errors and parse errors, unlike 
set_error_handler().

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
* Overload error_log() function
* Add getrusage() support and other analytics?
