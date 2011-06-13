Introduction
=========
php-memoize is a PHP extension which transparently caches your PHP functions, much like [Perl's Memoize module](http://perldoc.perl.org/Memoize.html).

It uses APC or memcached for the caching, so values persist between requests.

Usage
=====
````php
<?php
memoize('my_expensive_function');

// now calls to my_expensive_function() are cached by their arguments
my_expensive_function('foo'); // returns in 0.8s
my_expensive_function('foo'); // returns in 0.00001s
````
