<?php # vim:ft=php

$ext = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN' ? '.dll' : '.so';

if (!extension_loaded('memoize') && @!dl('memoize' . $ext)) {
	die('Memoize not available');
}
