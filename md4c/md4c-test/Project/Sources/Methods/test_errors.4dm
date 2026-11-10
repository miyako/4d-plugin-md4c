//%attributes = {"invisible":true,"preemptive":"capable"}
var $result : Object

// An unknown dialect is an error, not a silent fallback
$result:=MD4C Convert("# x"; {dialect: "markdown"})
ASSERT($result.success=False)
ASSERT($result.status=2)
ASSERT($result.result="")
ASSERT(Position("markdown"; $result.error)>0)
