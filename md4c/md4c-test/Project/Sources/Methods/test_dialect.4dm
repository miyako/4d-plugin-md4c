//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// CommonMark is the default: no strikethrough
$result:=MD4C Convert("~~x~~")
ASSERT($result.result="<p>~~x~~</p>\n")

$result:=MD4C Convert("~~x~~"; {dialect: MD4C dialect CommonMark})
ASSERT($result.result="<p>~~x~~</p>\n")

// GitHub dialect: strikethrough, permissive autolinks, task lists
$result:=MD4C Convert("~~x~~"; {dialect: MD4C dialect GitHub})
ASSERT($result.result="<p><del>x</del></p>\n")

$result:=MD4C Convert("www.example.com"; {dialect: "github"})
ASSERT($result.result="<p><a href=\"http://www.example.com\">www.example.com</a></p>\n")

$result:=MD4C Convert("- [x] done"; {dialect: "github"})
ASSERT(Position("type=\"checkbox\""; $result.result)>0)
ASSERT(Position("checked"; $result.result)>0)

// A flag set to false switches off part of the dialect
$result:=MD4C Convert("~~x~~"; {dialect: "github"; strikethrough: False})
ASSERT($result.result="<p>~~x~~</p>\n")

$result:=MD4C Convert("www.example.com"; {dialect: "github"; permissiveAutolinks: False})
ASSERT($result.result="<p>www.example.com</p>\n")
