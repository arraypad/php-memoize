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

function expensive_func2($s, $t = "foo") {
  echo "expensive_func2() called\n";
  return $s . $t;
}

$memcached = new Memcached;
$memcached->addServer('127.0.0.1', 11211);

echo "*** Testing memoize(): libmemcached\n";
$memcached->delete("_memoizdaf87d399139a2fc36392016082176ccb");
$memcached->delete("_memoizd7c1fbf66478e8e5f96b4b824baa0a411");

memoize('expensive_func');

test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

echo "*** Testing memoize(): memcached\n";
$memcached->delete("_memoizdbb0c511502be1a7a0607074a438c18ae");
$memcached->delete("_memoizd6f94da3841e8463ddb3e6b9c5a9e0e8a");

var_dump(memoize_memcached_set_connection($memcached));

memoize('expensive_func2');

test_func('expensive_func2', 'hai');
test_func('expensive_func2', 'hai');
test_func('expensive_func2', 'hai', 'again');
test_func('expensive_func2', 'hai', 'again');

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
*** Testing memoize(): memcached
bool(true)
expensive_func2() called
expensive_func2() returned 'haifoo' in %fs
expensive_func2() returned 'haifoo' in %fs
expensive_func2() called
expensive_func2() returned 'haiagain' in %fs
expensive_func2() returned 'haiagain' in %fs
===DONE===
