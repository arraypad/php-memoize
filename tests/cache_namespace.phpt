--TEST--
memoize - cache_namespace
--SKIPIF--
<?php include('skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
apc.file_update_protection=0
--FILE--
<?php

function test_func($f) {
  $args = func_get_args();
  $f = array_shift($args);
  $t = microtime(true);
  $ret = call_user_func_array($f, $args);
  printf("%s() returned %s in %.5fs\n", $f, var_export($ret, true), microtime(true) - $t);;
}

$currentNamespace = null;
function expensive_func($s, $t = "foo") {
  echo "expensive_func() called\n";
  return $GLOBALS['currentNamespace'] . '::' . $s . $t;
}

memoize('expensive_func');

/* no namespace */

test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

/* namespace a */

ini_set('memoize.cache_namespace', $currentNamespace = 'namespaceA');

test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

/* namespace b */

ini_set('memoize.cache_namespace', $currentNamespace = 'namespaceB');

test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

/* namespace b */

ini_set('memoize.cache_namespace', $currentNamespace = 'namespaceA');

test_func('expensive_func', 'hai', 'again');

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
expensive_func() called
expensive_func() returned '::haiagain' in %fs
expensive_func() returned '::haiagain' in %fs
expensive_func() called
expensive_func() returned 'namespaceA::haiagain' in %fs
expensive_func() returned 'namespaceA::haiagain' in %fs
expensive_func() called
expensive_func() returned 'namespaceB::haiagain' in %fs
expensive_func() returned 'namespaceB::haiagain' in %fs
expensive_func() returned 'namespaceA::haiagain' in %fs
===DONE===
