<?php # vim:ft=php
if (!extension_loaded('memoize')) die('skip memoize not loaded');
if (!(extension_loaded('memoize_memory') || extension_loaded('memoize_apc') || extension_loaded('memoize_memcached'))) die('skip storage module not loaded');
?>
