--TEST--
memoize - cache_namespace
--SKIPIF--
<?php if (!extension_loaded("memoize") || !memoize_has_storage("memory")) { die("skip"); } ?>
--FILE--
<?php
require 'init.inc';

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

echo "*** Testing memoize(): cache_namespace - no namespace\n";

test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

echo "*** Testing memoize(): cache_namespace - namespace A\n";

ini_set('memoize.cache_namespace', $currentNamespace = 'namespaceA');

test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

echo "*** Testing memoize(): cache_namespace - namespace B\n";

ini_set('memoize.cache_namespace', $currentNamespace = 'namespaceB');

test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

echo "*** Testing memoize(): cache_namespace - namespace A again\n";

ini_set('memoize.cache_namespace', $currentNamespace = 'namespaceA');

test_func('expensive_func', 'hai', 'again');

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
*** Testing memoize(): cache_namespace - no namespace
expensive_func() called
expensive_func() returned '::haiagain' in %fs
expensive_func() returned '::haiagain' in %fs
*** Testing memoize(): cache_namespace - namespace A
expensive_func() called
expensive_func() returned 'namespaceA::haiagain' in %fs
expensive_func() returned 'namespaceA::haiagain' in %fs
*** Testing memoize(): cache_namespace - namespace B
expensive_func() called
expensive_func() returned 'namespaceB::haiagain' in %fs
expensive_func() returned 'namespaceB::haiagain' in %fs
*** Testing memoize(): cache_namespace - namespace A again
expensive_func() returned 'namespaceA::haiagain' in %fs
===DONE===
