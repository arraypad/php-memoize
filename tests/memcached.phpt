--TEST--
memoize - memcached
--SKIPIF--
<?php if (!extension_loaded("memoize") || !memoize_has_storage("memcached")) { die("skip"); } ?>
--INI--
memoize.storage_module=memcached
memoize.memcached.servers="--SERVER=127.0.0.1"
--FILE--
<?php

function test_func($f) {
  $args = func_get_args();
  $f = array_shift($args);
  $t = microtime(true);
  $ret = call_user_func_array($f, $args);
  printf("%s() returned %s in %.5fs\n", $f, var_export($ret, true), microtime(true) - $t);;
}

function expensive_func($s, $t = "foo") {
  echo "expensive_func() called\n";
  return $s . $t;
}

echo "*** Testing memoize(): libmemcached\n";

memoize('expensive_func', 1);

test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
*** Testing memoize(): libmemcached
expensive_func() called
expensive_func() returned 'haifoo' in %fs
expensive_func() returned 'haifoo' in %fs
expensive_func() called
expensive_func() returned 'haiagain' in %fs
expensive_func() returned 'haiagain' in %fs
===DONE===
