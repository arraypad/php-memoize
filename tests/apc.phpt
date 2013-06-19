--TEST--
memoize - apc
--SKIPIF--
<?php if (!extension_loaded("memoize") || !extension_loaded("apc") || !memoize_has_storage("apc")) { die("skip"); } ?>
--INI--
memoize.storage_module=apc
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

function expensive_func($s, $t = "foo") {
  echo "expensive_func() called\n";
  return $s . $t;
}

function expensive_func2($s, $t = "foo") {
  echo "expensive_func2() called\n";
  return $s . $t;
}

echo "*** Testing memoize(): apc\n";

memoize('expensive_func');

test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
*** Testing memoize(): apc
expensive_func() called
expensive_func() returned 'haifoo' in %fs
expensive_func() returned 'haifoo' in %fs
expensive_func() called
expensive_func() returned 'haiagain' in %fs
expensive_func() returned 'haiagain' in %fs
===DONE===
