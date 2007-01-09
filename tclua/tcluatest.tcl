# $Id: tcluatest.tcl,v 1.1 2007-01-09 14:06:31 tclua Exp $
package require tcltest
namespace import tcltest::*

set setup {
  lappend auto_path .
  package require lua
}
eval $setup

test lua-1.1 {単純な正常ケース} -body {
  lua::lua {
    foo = 1
  }
} -result {}

test lua-2.1 {引数が足りない} -body {
  lua::lua
} -returnCodes error -match glob -result {wrong # args: should be *}

test lua-2.2 {引数が多すぎる} -body {
  lua::lua {} {}
} -returnCodes error -match glob -result {wrong # args: should be *}

test lua-2.3 {不完全なセンテンス} -body {
  lua::lua {
    function
  }
} -returnCodes error -match glob -result *

test lua-2.4 {実行時エラー} -body {
  lua::lua {
    a=nil
    a()
  }
} -returnCodes error -match glob -result *

test call-1.1 {定義済み関数を呼ぶ} -body {
  lua::call tonumber 15
} -result 15

test call-1.2 {関数を定義して呼ぶ} -body {
  lua::lua {
    function f(x)
      return x * 2
    end
  }
  lua::call f 15
} -result 30

test call-1.3 {モジュール内関数を呼ぶ} -body {
  lua::call math.abs -1
} -result 1

test call-1.4 {複数の戻り値} -body {
  lua::lua {
    function f(x)
      return x + 2, x * 2
    end
  }
  lua::call -result {a b} f 3
  return "$a $b"
} -result {5 6}

test call-2.1 {引数が足りない} -body {
  lua::call tonumber
} -returnCodes error -match glob -result {*bad argument*}

test call-2.2 {存在しない関数} -body {
  lua::lua {nonexist = nil}
  lua::call nonexist
} -returnCodes error -match glob -result {attempt to call a nil value}

test call-2.3 {存在しない関数（モジュール内）} -body {
  lua::lua {a = {}}
  lua::call a.foo
} -returnCodes error -match glob -result {attempt to call a nil value}

test call-2.4 {非テーブルをモジュールとして辿ろうとする} -body {
  lua::lua {a = "hello"}
  lua::call a.foo
} -returnCodes error -match glob -result {attempt to index a non-table value:*}

test funexist-1.1 {存在する関数} -body {
  lua::funexist print
} -result 1

test funexist-1.2 {存在する関数（モジュール内）} -body {
  lua::funexist math.abs
} -result 1

test funexist-1.3 {存在しない関数} -body {
  lua::lua {nonexist = nil}
  lua::funexist nonexist
} -result 0

test funexist-1.4 {存在しない関数（モジュール内）} -body {
  lua::lua {a = {}}
  lua::funexist a.foo
} -result 0

test funexist-1.5 {非テーブルをモジュールとして辿ろうとする} -body {
  lua::lua {a = "hello"}
  lua::funexist a.foo
} -result 0

test funexist-2.1 {引数が足りない} -body {
  lua::funexist
} -returnCodes error -match glob -result {wrong # args: should be *}

test funexist-2.2 {引数が多すぎる} -body {
  lua::funexist {} {}
} -returnCodes error -match glob -result {wrong # args: should be *}
