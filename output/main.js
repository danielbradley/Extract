Ronald.McDonald
=
function( responseText )
{
	var json = JSON.parse( responseText )
	
	if ( "OK" == json.status )
	{
		location.reload()
	}
}

