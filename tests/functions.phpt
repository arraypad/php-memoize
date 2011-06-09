--TEST--
memoize - functions
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

/* user function */

function expensive_func($s, $t = "foo") {
  echo "expensive_func() called\n";
  return $s . $t;
}
memoize('expensive_func');

test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

/* user function with no args */

function expensive_func2() {
  echo "expensive_func2() called\n";
}
memoize('expensive_func2');

test_func('expensive_func2');
test_func('expensive_func2');

/* internal function */

memoize('sqrt');

test_func('sqrt', 65536);
test_func('sqrt', 65536);
test_func('sqrt', 64);
test_func('sqrt', 64);

/* return by ref */
function &function_returns_ref() {
  $foo = "bar";
  return $foo;
}

memoize('function_returns_ref');

/* sanity checks */
var_dump(memoize('memoize'));

var_dump(memoize_call());

var_dump(memoize('expensive_func'));
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
expensive_func() called
expensive_func() returned 'haifoo' in %fs
expensive_func() returned 'haifoo' in %fs
expensive_func() called
expensive_func() returned 'haiagain' in %fs
expensive_func() returned 'haiagain' in %fs
expensive_func2() called
expensive_func2() returned NULL in %fs
expensive_func2() returned NULL in %fs
sqrt() returned 256 in %fs
sqrt() returned 256 in %fs
sqrt() returned 8 in %fs
sqrt() returned 8 in %fs

Warning: memoize(): Cannot cache functions which return references in %s on line %d

Warning: memoize(): Cannot memoize memoize()! in %s on line %d
bool(false)

Warning: memoize_call(): Cannot call memoize_call() directly in %s on line %d
bool(false)

Warning: memoize(): expensive_func() is already memoized in %s on line %d
bool(false)
===DONE===
