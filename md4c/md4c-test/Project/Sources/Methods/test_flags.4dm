//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// Individual extensions on top of CommonMark
$result:=MD4C Convert("| a |\n|---|\n| 1 |")
ASSERT(Position("<table>"; $result.result)=0)

$result:=MD4C Convert("| a |\n|---|\n| 1 |"; {tables: True})
ASSERT(Position("<table>"; $result.result)=1)
ASSERT(Position("<td>1</td>"; $result.result)>0)

$result:=MD4C Convert("~~x~~"; {strikethrough: True})
ASSERT($result.result="<p><del>x</del></p>\n")

$result:=MD4C Convert("==m=="; {highlight: True})
ASSERT($result.result="<p><mark>m</mark></p>\n")

// Raw HTML is passed through unless disabled
$result:=MD4C Convert("<b>x</b>")
ASSERT($result.result="<p><b>x</b></p>\n")

$result:=MD4C Convert("<b>x</b>"; {noHTML: True})
ASSERT($result.result="<p>&lt;b&gt;x&lt;/b&gt;</p>\n")

// Soft breaks as hard breaks
$result:=MD4C Convert("a\nb"; {hardSoftBreaks: True})
ASSERT($result.result="<p>a<br>\nb</p>\n")

// A non-Boolean flag value is ignored
$result:=MD4C Convert("~~x~~"; {strikethrough: "yes"})
ASSERT($result.result="<p>~~x~~</p>\n")
