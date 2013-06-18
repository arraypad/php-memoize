Introduction
=========
php-memoize is a PHP extension which transparently caches PHP functions, much like [Perl's Memoize module](http://perldoc.perl.org/Memoize.html).

It comes with the following storage modules which can be enabled at compile time:

- **memory**: Simple per-request module with no dependencies
- **memcached**: Uses [libmemcached](http://libmemcached.org/libMemcached.html) or the [memcached PHP extension](http://pecl.php.net/package/memcached)
- **apc**: Uses the [APC PHP extension](http://pecl.php.net/package/apc)

It also provides a simple interface (`php_memoize_storage.h`) if you want to define your own storage modules as PHP extensions.

Usage
=====
````php
<?php

function my_expensive_function($x) {
	sleep(10);
	return $x . 'bar';
}

memoize('my_expensive_function');

// now calls to my_expensive_function() are cached by their arguments

echo my_expensive_function('foo'); // returns "foobar" in 10s
echo my_expensive_function('foo'); // returns "foobar" in 0.0001s
````

**N.B.** Do not memoize functions which have side-effects or depend on anything other than their arguments, such as global variables or the current time.

Functions
=========

### bool memoize(mixed $callback)

Registers a function to be memoized. Like the callbacks taken for example by ````call_user_func()````, $callback can be a string containing a function name or an array containing a class or object and a method name.

Unlike normal callbacks, it can refer to methods which aren't callable from this scope (e.g. private methods.) It can also refer to non-static methods as if they were static, so you don't need to have an instance available when you register the method.

Returns ````true```` if the function was successfully registered, or false and raises an E_WARNING error otherwise.

### bool memoize_memcached_set_connection(Memcached $m)

Sets an existing Memcached object to be be used for memoize storage, instead of creating a new connection using the servers defined in ````memoize.memcached.servers````. This function is only available when the memoize_memcached extension is present, and only applicable when ````memoize.storage_module```` is set to "memcached".

Settings
========

All of the below ini settings can be changed at any time (PHP_INI_ALL).

### General 
<table>
	<tr>
		<td>memoize.storage_module</td>
		<td>String</td>
		<td>The storage module to use ("memory", "apc" or "memcached")</td>
	</tr>
	<tr>
		<td>memoize.cache_namespace</td>
		<td>String</td>
		<td>A string to prepend to all cache keys, so that separate applications can use the same storage without conflicts.</td>
	</tr>
	<tr>
		<td>memoize.default_ttl</td>
		<td>Integer</td>
		<td>The number of seconds to store cache entries for (defaults to 3600, one hour.)</td>
	</tr>
</table>

### Memcached

<table>
	<tr>
		<td>memoize.memcached.servers</td>
		<td>String</td>
		<td>A [libmemcached configuration string](http://docs.libmemcached.org/libmemcached_configuration.html) (e.g. "--SERVER=127.0.0.1 --SERVER=10.0.2.2:22122"). This is only used if a connection hasn't been supplied by memoize_memcached_set_connection().</td>
	</tr>
</table>

Installation
============

See [Compiling shared PECL extensions with phpize](http://www.php.net/manual/en/install.pecl.phpize.php) for general instructions. The available configure options can be listed with `configure --help`.
