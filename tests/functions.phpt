--TEST--
memoize - functions
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

echo "*** Testing memoize(): functions - user functions\n";

function expensive_func($s, $t = "foo") {
  echo "expensive_func() called\n";
  return $s . $t;
}
memoize('expensive_func');

test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai');
test_func('expensive_func', 'hai', 'again');
test_func('expensive_func', 'hai', 'again');

echo "*** Testing memoize(): functions - functions with no args\n";

function expensive_func2() {
  echo "expensive_func2() called\n";
}
memoize('expensive_func2');

test_func('expensive_func2');
test_func('expensive_func2');

echo "*** Testing memoize(): functions - internal functions\n";

memoize('sqrt');

test_func('sqrt', 65536);
test_func('sqrt', 65536);
test_func('sqrt', 64);
test_func('sqrt', 64);

echo "*** Testing memoize(): functions - return by reference\n";

function &function_returns_ref() {
  $foo = "bar";
  return $foo;
}

memoize('function_returns_ref');

echo "*** Testing memoize(): functions - sanity checks\n";

var_dump(memoize('memoize'));

var_dump(memoize_call());

var_dump(memoize('expensive_func'));
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
*** Testing memoize(): functions - user functions
expensive_func() called
expensive_func() returned 'haifoo' in %fs
expensive_func() returned 'haifoo' in %fs
expensive_func() called
expensive_func() returned 'haiagain' in %fs
expensive_func() returned 'haiagain' in %fs
*** Testing memoize(): functions - functions with no args
expensive_func2() called
expensive_func2() returned NULL in %fs
expensive_func2() returned NULL in %fs
*** Testing memoize(): functions - internal functions
sqrt() returned 256 in %fs
sqrt() returned 256 in %fs
sqrt() returned 8 in %fs
sqrt() returned 8 in %fs
*** Testing memoize(): functions - return by reference

Warning: memoize(): Cannot cache functions which return references in %s on line %d
*** Testing memoize(): functions - sanity checks

Warning: memoize(): Cannot memoize memoize()! in %s on line %d
bool(false)

Warning: memoize_call(): Cannot call memoize_call() directly in %s on line %d
bool(false)

Warning: memoize(): expensive_func() is already memoized in %s on line %d
bool(false)
===DONE===
