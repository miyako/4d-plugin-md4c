//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// Basic conversion; options may be omitted
$result:=MD4C Convert("# Hello")
ASSERT($result.success=True)
ASSERT($result.status=0)
ASSERT($result.result="<h1>Hello</h1>\n")
ASSERT($result.error=Null)

// Non-ASCII text, including a character outside the BMP (surrogate pair)
$result:=MD4C Convert("*日本語* 🎉"; {})
ASSERT($result.success)
ASSERT($result.result="<p><em>日本語</em> 🎉</p>\n")

// Empty source
$result:=MD4C Convert("")
ASSERT($result.success)
ASSERT($result.result="")

// Entities are translated to characters by default
$result:=MD4C Convert("&copy;")
ASSERT($result.result="<p>©</p>\n")
