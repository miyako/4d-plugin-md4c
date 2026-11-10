//%attributes = {"invisible":true}
If (Application info.headless)
	
	test_convert
	test_dialect
	test_flags
	test_renderer
	test_errors
	
	LOG EVENT(Into system standard outputs; "PASS"; Information message)
	
End if 
