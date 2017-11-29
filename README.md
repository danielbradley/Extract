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

### Dependencies

'Extract' depends on functions declared in the following header files.


```main.c
#include <stdlib.h>
```

This header is included to provide access to the 'calloc' and 'free' functions:
'calloc' is used to allocate memory that has been zeroed,
while 'free' is used to deallocate allocated memory.

```main.c
#include <stdio.h>
```

This header is included to provide access to the 'fopen', 'fclose', and 'fprintf' functions:
'fopen' is used to open the files indicated by the command-line arguments,
'fclose' is used to close those files, and
'fprintf' is used to write pre-formatted text blocks to the standard output stream.


```main.c
#include <string.h>
```

This header is included to provide access to the 'strdup', 'strcmp', 'strncmp', 'strlen', 'strpcpy', and functions:
'strdup' is used to duplicate the passed string,
'strcmp' and 'strncmp' are used to compare whether two strings are equal ('strcmp'), or whether one is a prefix of the other ('strncmp').
'strlen' is used to find the length of a string, and
'strpcpy' is used to ???

### Defined types and values

The 'int' type is typecast to a 'bool' for readability.

```main.c
typedef int bool;
```

### Global state

The implementation has two global variables:
if 'DEBUG' is set to 'TRUE', extra debugging lines are output to the standard error stream.
if 'STRIP' is set to 'TRUE', extract will strip lines beginning with 'DELIMITER' from the ouput.

```main.c
static bool DEBUG = 0; // FALSE;
static bool STRIP = 0; // FALSE;
```

### High-level function structure

'Extract' has a simple call-graph, which is shown below:

```
main
 |-- argumentsContains
 |-- argumentsGetValue
 |-- usage
 |-- generateDelimiter
 |-- tryToProcess
      |-- process
           |-- readline
           |-- processPreformatted
```

A summary of these functions is given, then their implementation is discussed in detail.

```main.c
int main( int argc, char** argv );
```

The 'main' function is responsible for checking whether appropriate arguments have been passed and:
if so, calling tryToProcess on each file and;
if not, calling the 'usage' function.

```main.c
bool argumentsContains( int argc, char** argv, char* flag );
```

The 'argumentsContains' function check each of the arguments passed to main to see if it matches the specified flag.

```main.c
char* argumentsGetValue( int n, char** files, char* flag );
```

The 'argumentsGetValue' function retrieves (if appropriate) the value following a specific flag.

```main.c
int usage();
```

The 'usage' function simply prints out the usage string and always returns -1.

```main.c
char* generateDelimiter( const char* prefix, const char* pattern, const char* suffix );
```

The 'generate' pattern function is used to generate the line delimiter to be searched for in the input files.

```main.c
void tryToProcess( char* file, const char* pattern );
```

The 'tryToProcess' function tries to open the passed file, and if successful passes the opened stream to
'process' along with the passed line pattern.

```main.c
void process( FILE*, const char* pattern );
```

The 'process' function reads each line of the passed stream,
and if the passed line pattern is found outputs any further lines to standard output
until an end delimiter is encountered.

```main.c
char* readline( FILE* );
```

The 'readline' function reimplements the UNIX readline function for portability.

```main.c
void processPreformatted( const char* line, FILE*, const char* pattern );
```

The 'processPreformatted' function is used to strip out lines starting with specific keywords such as 'DELIMITER', or 'DROP',
which (in the case of SQL) it is often desirous to remove for installation scripts.

```main.c
int stringEquals( const char* one, const char* two );
```

The 'stringEquals' function compares whether the two passed strings contain the same characters.

```main.c
int stringHasPrefix( const char* string, const char* prefix );
```

The 'stringHasPrefix' function determins whether the passed string has the passed prefix.

### Low-level implemention function descriptions

#### Function: main

The main function first processes the passed arguments to retrieve the pattern,
and to determine whether the *strip* flag has been passed, e.g., ('-s').

If a pattern has not been passed, or the number of arguments passed are less than expected,
the usage() function is called and exit is called.

Otherwise, each file passed in the arguments is passed to the 'tryToProcess' function
along with the *line delimiter*.

```main.c
int main( int argc, char** argv )
{
    char* pat = argumentsGetValue( argc, argv, "-p" );
    STRIP     = argumentsContains( argc, argv, "-s" ) ? 1 : 0;

                                        // 1.       2.  3.  4.            5.
    int expected_arguments = 4 + STRIP; // extract [-s] -p "some pattern" file...

    if ( ! pat )
    {
        exit( usage() );
    }
    else
    if ( argc < expected_arguments )
    {
        exit( usage() );
    }
    else
    {
        //
        //  Must try to process each arg that does not start with '-', except:
        //  argv[0], which is process name, and
        //  argv[?], pattern which follows '-p'.
        //

        int i;
        for ( i = 1; i < argc; i++ )
        {
            if ( '-' != argv[i][0] )
            {
                tryToProcess( argv[i], pat );
            }
            else
            if ( 'p' == argv[i][1] )
            {
                i++;
            }
        }
    }
    free( pat );

    return 0;
}
```

#### Function: argumentContains

```main.c
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

#### Function: argumentGetValue

```main.c
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

#### Function: usage

```main.c
int usage()
{
    fprintf( stderr, "Usage:\n\textract [-s] -p <pattern> <file> [more files] [ > output file ]\n" );
    return -1;
}
```

#### Function: generateDelimiter

```main.c
char* generateDelimiter( const char* prefix, const char* pattern, const char* suffix )
{
    char* delimiter = calloc( strlen( prefix ) + strlen( pattern ) + strlen( suffix ) + 1, sizeof(char) );
    char* tmp       = delimiter;
          tmp       = stpcpy( tmp, prefix  );
          tmp       = stpcpy( tmp, pattern );
          tmp       = stpcpy( tmp, suffix  );

    return delimiter;
}
```

#### Function: tryToProcess

```main.c
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

#### Function: process

```main.c
void process( FILE* stream, const char* pattern )
{
    char* line_delimiter = "";
    char* line;

    do
    {
        line = readline( stream );
        if ( line )
        {
            switch ( line[0] )
            {
            case '~':
                line_delimiter = generateDelimiter( "~", pattern, "~" );

                if ( DEBUG ) fprintf( stderr, "@%s", line );
                processPreformatted( line, stream, line_delimiter );
                break;
            case '`':
                line_delimiter = generateDelimiter( "```", pattern, "" );

                if ( ('`' == line[1]) && ('`' == line[2]) )
                {
                    if ( DEBUG ) fprintf( stderr, "@%s", line );
                    processPreformatted( line, stream, line_delimiter );
                }
                break;
            default:
                if ( DEBUG ) fprintf( stderr, "@%s", line );
            }
            free( line );
        }
    } while ( line );

    free( line_delimiter );
}
```

#### Function: processPreformatted

```main.c
void processPreformatted( const char* line, FILE* stream, const char* line_delimiter )
{
    int   loop = 1;
    char* pre;

    FILE* dev_null = fopen( "/dev/null", "a" );
    FILE* out      = dev_null;
    char* c        = "@";

    if ( 0 == strncmp( line_delimiter, line, strlen(line_delimiter) ) )
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

    fclose( dev_null );
}
```

#### Function: readline

```main.c
char* readline( FILE* stream )
{
    int  n     = 0;
    int  sz    = 1024;
    char ch[2] = { 0, 0 };
    char* line = calloc( sz + 1, sizeof( char ) );

    int read;
    do
    {
        read = fread( ch, sizeof(char), 1, stream );
        if ( read )
        {
            switch ( *ch )
            {
            case '\n':
                line[n++] = *ch;
                line[n]   = '\0';
                read      = 0;
                break;
            default:
                line[n++] = *ch;
                line[n]   = '\0';
            }

            if ( n == sz )
            {
                sz  *= 2;
                line = realloc( line, sz );
            }
        }
        
    }
    while ( 0 != read );

    if ( 0 == n )
    {
        free( line );
        line = NULL;
    }

    return line;
}
```

#### Function: stringEquals

```main.c
int stringEquals( const char* one, const char* two )
{
    return (0 == strcmp( one, two ));
}
```

#### Function: stringHasPrefix

```main.c
int stringHasPrefix( const char* string, const char* prefix )
{
    int len = strlen( prefix );

    return (0 == strncmp( string, prefix, len ));
}
```
