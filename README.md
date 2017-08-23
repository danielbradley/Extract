# Extract

'Extract' is a command line tool for extracting SQL and other code
from marked up text files that are able to represent pre-formated text sections.

Currently, two such text formats are explicitly supported - Markdown and MaxText.
MaxText is a plain text markup system that is similar conceptually to Markdown.

Markdown uses a triplet of back-ticks ("```") in the left-most column to indicate the start and end of a pre-formatted text section, while Maxtext uses a tilde ('~') character.
While, technically, both text systems ignore any remaining text on the line,
some systems such as Github, allow remaining text to hint at a programming language,
and others such as Pandoc may not recognise the delimiter if there are spaces in any remaining text.

'Extract' uses these superfluous characters to indicate a pattern that will be matched against a pattern passed
as a common-line argument.
For example,
when run,
the following command line matches pre-formatted sections in the specified files that contain the pattern 'tables'.

```
extract -p "tables" source/mt/file.txt > sql/tables.sql
```

When using Markdown, such a pre-formatted block would be delimited as such:

```
 ```tables
 Pre-formatted text here
```

Unfortunately, it seems like there is no pattern that can be used with both Pandoc and Github to indicate both a pattern and a file type.
For example, Github will accept a delimiter such as that below and appropriately mark it up as SQL:

```
 ```tables sql
```

However, Pandoc does not seem to accept such a delimiter, but rather will accept the following,
which is not accepted by Github:

```
 ```tables.sql
```

Therefore, if you will only be using Github you can use a pattern that has a space then 'sql',
however, if you will be using Pandoc, use a pattern with no spaces in it.

Hopefully, in the future one of these two systems will be modified so that they can support similar patterns.


## Implementation

The following sections describe the current implementation of 'Extract'.
This implementation now uses the "quasi-literate programming" style as described at:
[https://www.quasi-literateprogramming.org].
To facilitate this implementation that now supports Markdown,
the tool 'quasi' was also extended to support Markdown.

Source code extracted from this file may be viewed under the '_gen' directory.

```!main.c
/*
 *  !!! This document has been auto-generated using quasi !!!
 */
```


```main.c
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
```

```!main.js
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
```

```
static int DEBUG = 0;
```

```
int    argumentsContains();
char*  argumentsGetValue();
int                usage();
char*    generatePattern( const char* );
void        tryToProcess( char*, const char* );
void             process( FILE*, const char* );
char*           readline( FILE* );
void processPreformatted( char* line, FILE*, const char* pattern );
```

```
int STRIP;
```

```
int main( int argc, char** files )
{
	STRIP     = argumentsContains( argc, files, "-s" ) ? 1 : 0;
	char* pat = argumentsGetValue( argc, files, "-p" );

	int expected_arguments = 4 + STRIP; // extract -p "some pattern" files [-s]

	if ( ! pat )
	{
		return usage();
	}
	else
	if ( argc < expected_arguments )
	{
		return usage();
	}
	else
	{
		char* pattern = generatePattern( pat );
		int i;
		for ( i=expected_arguments; i < argc; i++ )
		{
			tryToProcess( files[i], pattern );
		}

		free( pattern );
	}
	free( pat );

	return 0;
}
```

```
int argumentsContains( int n, char** files, char* flag )
{
	int b = 0;
	int i;

	for ( i=0; i < n; i++ )
	{
		if ( stringEquals( flag, files[i] ) )
		{
			b = 1;
			i = n;
		}
	}
	return b;
}
```

```
char* argumentsGetValue( int n, char** files, char* flag )
{
	char* ret = NULL;
	int i;

	for ( i=0; i < n; i++ )
	{
		if ( stringEquals( flag, files[i] ) )
		{
			i++;
			if ( i < n) ret = strdup( files[i] );
			i = n;
		}
	}
	return ret;
}
```

```
int stringEquals( const char* one, const char* two )
{
	return (0 == strcmp( one, two ));
}
```

```
int usage()
{
	fprintf( stderr, "Usage:\n\textract [-s] -p <pattern> <file> [more files] [ > output file ]\n" );
	return -1;
}
```

```
char* generatePattern( const char* pat )
{
	int len = strlen( pat );
	char* pattern = calloc( len + 3, sizeof(char) );
	char* tmp = pattern;
	tmp = stpcpy( tmp, "~" );
	tmp = stpcpy( tmp, pat );
	tmp = stpcpy( tmp, "~" );
	return pattern;
}
```

```
void tryToProcess( char* file, const char* pattern )
{
	FILE* stream;
	if ( (stream = fopen( file, "r" )) )
	{
		if ( DEBUG ) fprintf( stderr, "Processing: %s\n", file );
		process( stream, pattern );
		fclose( stream );
	}
}
```

```
void process( FILE* stream, const char* pattern )
{
	char* line;
	do
	{
		line = readline( stream );
		if ( line )
		{
			switch ( line[0] )
			{
			case '~':
				if ( DEBUG ) fprintf( stderr, "@%s", line );
				processPreformatted( line, stream, pattern );
				break;
			default:
				if ( DEBUG ) fprintf( stderr, "@%s", line );
			}
			free( line );
		}
	} while ( line );
}
```

```
void processPreformatted( char* line, FILE* stream, const char* pattern )
{
	int   loop = 1;
	char* pre;

	FILE* out  = stderr;
	char* c    = "@";

	if ( 0 == strncmp( pattern, line, strlen(pattern) ) )
	{
		out = stdout;
		c   = "";
	}

	do
	{
		pre = readline( stream );
		if ( pre )
		{
			if ( STRIP )
			{
				switch ( pre[0] )
				{
				case '~':
					if ( DEBUG ) fprintf( stderr, "@%s", pre );
					loop = 0;
					break;
				case '/':
					if ( '/' == pre[1] ) fprintf( out, ";\n" );
					break;
				case 'D':
				case 'd':
					if ( 0 == strncmp( "DROP",      pre, strlen( "DROP"      ) ) ) break;
					if ( 0 == strncmp( "drop",      pre, strlen( "drop"      ) ) ) break;
					if ( 0 == strncmp( "DELIMITER", pre, strlen( "DELIMITER" ) ) ) break;
					if ( 0 == strncmp( "delimiter", pre, strlen( "delimiter" ) ) ) break;
				default:
					if ( 0 != strncmp( "@", c, 1 ) ) fprintf( out, "%s%s", c, pre );
				}
			}
			else
			{
				switch ( pre[0] )
				{
				case '~':
					if ( DEBUG ) fprintf( stderr, "@%s", pre );
					loop = 0;
					break;
				case 'D':
				case 'd':
					if ( 0 == strncmp( "DROP",      pre, strlen( "DROP"      ) ) ) break;
					if ( 0 == strncmp( "drop",      pre, strlen( "drop"      ) ) ) break;
					if ( 0 == strncmp( "delimiter", pre, strlen( "delimiter" ) ) ) break;
				default:
					if ( 0 != strncmp( "@", c, 1 ) ) fprintf( out, "%s%s", c, pre );
				}
			}
			free( pre );
		} else {
			loop = 0;
		}
	}
	while ( loop );
}
```

```
char* readline( FILE* stream )
{
	int   n = 0;
	char* line = calloc( 1024, sizeof(char) );
	char* ptr  = calloc( 10,  sizeof(char) );
	int read;
	do
	{
		read = fread( ptr, sizeof(char), 1, stream );
		if ( read )
		{
			switch( *ptr )
			{
			case '\n':
				line[n++] = *ptr;
				read = 0;
				break;
			default:
				line[n++] = *ptr;
			}
		}
	} while ( 0 != read );
	free( ptr );

	if ( n )
	{
		return line;
	}
	else
	{
		free( line );
		return NULL;
	}
}
```
