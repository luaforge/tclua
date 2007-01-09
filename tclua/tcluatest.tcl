# $Id: tcluatest.tcl,v 1.1 2007-01-09 14:06:31 tclua Exp $
package require tcltest
namespace import tcltest::*

set setup {
  lappend auto_path .
  package require lua
}
eval $setup

test lua-1.1 {�P���Ȑ���P�[�X} -body {
  lua::lua {
    foo = 1
  }
} -result {}

test lua-2.1 {����������Ȃ�} -body {
  lua::lua
} -returnCodes error -match glob -result {wrong # args: should be *}

test lua-2.2 {��������������} -body {
  lua::lua {} {}
} -returnCodes error -match glob -result {wrong # args: should be *}

test lua-2.3 {�s���S�ȃZ���e���X} -body {
  lua::lua {
    function
  }
} -returnCodes error -match glob -result *

test lua-2.4 {���s���G���[} -body {
  lua::lua {
    a=nil
    a()
  }
} -returnCodes error -match glob -result *

test call-1.1 {��`�ς݊֐����Ă�} -body {
  lua::call tonumber 15
} -result 15

test call-1.2 {�֐����`���ČĂ�} -body {
  lua::lua {
    function f(x)
      return x * 2
    end
  }
  lua::call f 15
} -result 30

test call-1.3 {���W���[�����֐����Ă�} -body {
  lua::call math.abs -1
} -result 1

test call-1.4 {�����̖߂�l} -body {
  lua::lua {
    function f(x)
      return x + 2, x * 2
    end
  }
  lua::call -result {a b} f 3
  return "$a $b"
} -result {5 6}

test call-2.1 {����������Ȃ�} -body {
  lua::call tonumber
} -returnCodes error -match glob -result {*bad argument*}

test call-2.2 {���݂��Ȃ��֐�} -body {
  lua::lua {nonexist = nil}
  lua::call nonexist
} -returnCodes error -match glob -result {attempt to call a nil value}

test call-2.3 {���݂��Ȃ��֐��i���W���[�����j} -body {
  lua::lua {a = {}}
  lua::call a.foo
} -returnCodes error -match glob -result {attempt to call a nil value}

test call-2.4 {��e�[�u�������W���[���Ƃ��ĒH�낤�Ƃ���} -body {
  lua::lua {a = "hello"}
  lua::call a.foo
} -returnCodes error -match glob -result {attempt to index a non-table value:*}

test funexist-1.1 {���݂���֐�} -body {
  lua::funexist print
} -result 1

test funexist-1.2 {���݂���֐��i���W���[�����j} -body {
  lua::funexist math.abs
} -result 1

test funexist-1.3 {���݂��Ȃ��֐�} -body {
  lua::lua {nonexist = nil}
  lua::funexist nonexist
} -result 0

test funexist-1.4 {���݂��Ȃ��֐��i���W���[�����j} -body {
  lua::lua {a = {}}
  lua::funexist a.foo
} -result 0

test funexist-1.5 {��e�[�u�������W���[���Ƃ��ĒH�낤�Ƃ���} -body {
  lua::lua {a = "hello"}
  lua::funexist a.foo
} -result 0

test funexist-2.1 {����������Ȃ�} -body {
  lua::funexist
} -returnCodes error -match glob -result {wrong # args: should be *}

test funexist-2.2 {��������������} -body {
  lua::funexist {} {}
} -returnCodes error -match glob -result {wrong # args: should be *}
