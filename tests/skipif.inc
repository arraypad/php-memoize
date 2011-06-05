<?php # vim:ft=php
if (!extension_loaded('apc') || !extension_loaded('memoize')) print 'skip not loaded';
if (!ini_get('apc.enable_cli')) print 'skip apc.enable_cli=0';
?>
