--TEST--
memoize - memcached
--SKIPIF--
<?php include('skipif.inc'); ?>
<?php if (!extension_loaded("memcached") || !extension_loaded("memoize_memcached")) { die("skip"): } ?>
--INI--
memoize.storage_module=memcached
--FILE--
<?php

function test_func($f) {
  $args = func_get_args();
  $f = array_shift($args);
  $t = microtime(true);
  $ret = call_user_func_array($f, $args);
  printf("%s() returned %s in %.5fs\n", $f, var_export($ret, true), microtime(true) - $t);;
}

echo "*** Testing memoize(): memcached\n";

function expensive_func($s, $t = "foo") {
  echo "expensive_func() called\n";
  return $s . $t;
}

$memcached = new Memcached;
$memcached->addServer('127.0.0.1', 11211);
$memcached->delete("_memoizdaf87d399139a2fc36392016082176ccb");
$memcached->delete("_memoizd7c1fbf66478e8e5f96b4b824baa0a411");

var_dump(memoize_memcached_set_connection($memcached));

memoize('expensive_func');

test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
*** Testing memoize(): memcached
bool(true)
expensive_func() called
expensive_func() returned 'haifoo' in %fs
expensive_func() returned 'haifoo' in %fs
expensive_func() called
expensive_func() returned 'haiagain' in %fs
expensive_func() returned 'haiagain' in %fs
===DONE===
