<?php # vim:ft=php

$ext = strtoupper(substr(PHP_OS, 0, 3)) === 'WIN' ? '.dll' : '.so';

if (!extension_loaded('memoize') && @!dl('memoize' . $ext)) {
	die('Memoize not available');
}

$storages = array('memory', 'apc', 'memcached');
foreach ($storages as $storage) {
	if (extension_loaded('memoize_' . $storage)) {
		$foundStorage = $storage;
		break;
	}
}

if (!isset($foundStorage)) {
	foreach ($storages as $storage) {
		if (@dl('memoize_' . $storage . $ext)) {
			$foundStorage = $storage;
			break;
		}
	}
}

if (!isset($foundStorage)) {
	die('No memoize storage modules available');
}

ini_set('memoize.storage_module', $foundStorage);

if ($foundStorage == 'apc') {
	ini_set('apc.enabled', 1);
	ini_set('apc.enable_cli', 1);
	ini_set('apc.file_update_protection', 0);
} else if ($foundStorage == 'memcached') {
	ini_set('memoize.memcached.servers', '127.0.0.1:11211');
}
