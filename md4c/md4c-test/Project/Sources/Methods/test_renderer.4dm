//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// HTML vs XHTML line breaks
$result:=MD4C Convert("a\\\nb")
ASSERT($result.result="<p>a<br>\nb</p>\n")

$result:=MD4C Convert("a\\\nb"; {xhtml: True})
ASSERT($result.result="<p>a<br />\nb</p>\n")

// Keep entities as written
$result:=MD4C Convert("&copy;"; {verbatimEntities: True})
ASSERT($result.result="<p>&copy;</p>\n")
