--TEST--
memoize - methods
--SKIPIF--
<?php include('skipif.inc'); ?>
--INI--
apc.enabled=1
apc.enable_cli=1
apc.file_update_protection=0
--FILE--
<?php
error_reporting(E_ALL|E_STRICT);
function test_func($f) {
  $args = func_get_args();
  $f = array_shift($args);
  $t = microtime(true);
  $ret = call_user_func_array($f, $args);
  printf("%s() returned %s in %.5fs\n", join('::', $f), var_export($ret, true), microtime(true) - $t);;
}

/* user class */

class Foo {
  public $id;

  static function expensive_method($s, $t = "foo") {
    echo "expensive_method() called\n";
    return $s . $t;
  }

  function dynamic_method() {
    echo "dynamic_method() called\n";
    return $this->id;
  }

  function dynamic_method2() {
    echo "dynamic_method2() called\n";
    return $this->id;
  }

  function __toString() {
    return 'Foo instance[' . $this->id . ']';
  }
}

$cb = array('Foo', 'expensive_method');
memoize($cb);

test_func($cb, 'hai');
test_func($cb, 'hai');
test_func($cb, 'hai', 'again');
test_func($cb, 'hai', 'again');

/* instances */

$foo = new Foo;
$foo->id = "obj1";
$cb = array($foo, 'dynamic_method');

memoize($cb);

test_func($cb);
test_func($cb);

$foo2 = new Foo;
$foo2->id = "obj2";
$cb = array($foo2, 'dynamic_method');

test_func($cb);
test_func($cb);

/* instance method, memoized on class */

$cb = array('Foo', 'dynamic_method2');
memoize($cb);
/*
test_func(array($foo, 'dynamic_method2'));
test_func(array($foo, 'dynamic_method2'));
test_func(array($foo2, 'dynamic_method2'));
test_func(array($foo2, 'dynamic_method2'));
*/
/* internal class */

$cb = array('DateTime', 'createFromFormat');
memoize($cb);

date_default_timezone_set('Europe/London');
test_func($cb, 'j-M-Y', '15-Feb-2009');
test_func($cb, 'j-M-Y', '15-Feb-2009');
test_func($cb, 'j-M-Y', '01-Apr-1945');
test_func($cb, 'j-M-Y', '01-Apr-1945');
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
expensive_method() called
Foo::expensive_method() returned 'haifoo' in %fs
Foo::expensive_method() returned 'haifoo' in %fs
expensive_method() called
Foo::expensive_method() returned 'haiagain' in %fs
Foo::expensive_method() returned 'haiagain' in %fs
dynamic_method() called
Foo instance[obj1]::dynamic_method() returned 'obj1' in %fs
Foo instance[obj1]::dynamic_method() returned 'obj1' in %fs
dynamic_method() called
Foo instance[obj2]::dynamic_method() returned 'obj2' in %fs
Foo instance[obj2]::dynamic_method() returned 'obj2' in %fs
dynamic_method2() called
Foo instance[obj1]::dynamic_method2() returned 'obj1' in %fs
Foo instance[obj1]::dynamic_method2() returned 'obj1' in %fs
dynamic_method2() called
Foo instance[obj2]::dynamic_method2() returned 'obj2' in %fs
Foo instance[obj2]::dynamic_method2() returned 'obj2' in %fs
DateTime::createfromformat() returned DateTime::__set_state(array(
   'date' => '2009-02-15 %s',
   'timezone_type' => 3,
   'timezone' => 'Europe/London',
)) in %fs
DateTime::createfromformat() returned DateTime::__set_state(array(
   'date' => '2009-02-15 %s',
   'timezone_type' => 3,
   'timezone' => 'Europe/London',
)) in %fs
DateTime::createfromformat() returned DateTime::__set_state(array(
   'date' => '1945-04-01 %s',
   'timezone_type' => 3,
   'timezone' => 'Europe/London',
)) in %fs
DateTime::createfromformat() returned DateTime::__set_state(array(
   'date' => '1945-04-01 %s',
   'timezone_type' => 3,
   'timezone' => 'Europe/London',
)) in %fs
===DONE===
