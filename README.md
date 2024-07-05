# Extract
 Daniel Bradley - daniel@crossadaptive.com

Extract is Copyright 2011-2024 Daniel Robert Bradley

Extract is distributed under the terms of the GNU General Public License Version 3,
as is provided in the file GNU_GPL_License_v3.txt

## Introduction

"Extract" is a command line tool for extracting SQL and other code
from marked up text files that are able to represent pre-formatted text sections.

Currently, two such text formats are explicitly supported - Markdown and MaxText.
MaxText is a plain text markup system that is similar conceptually to Markdown.



## Quick start

1) Download Quasi from here
2) Compile by typing make (binary is at bin/<arch>/extract)


## MaxText versus Markdown

Markdown uses a triplet of back-ticks ("```") in the left-most column to indicate the start and end of a pre-formatted text section, while Maxtext uses a tilde ('~') character.
While, technically, both text systems ignore any remaining text on the line,
some systems such as Github, allow remaining text to hint at a programming language for syntax highlighting,
and others such as Pandoc may not recognise the delimiter if there are spaces in any remaining text.

'Extract' uses these superfluous characters to indicate a pattern that will be matched against a pattern passed
as a common-line argument.
For example,
when run,
the following command line matches pre-formatted sections in the specified files that contain the pattern 'tables'.

```
extract -p "tables" source/mt/file.txt > sql/tables.sql
```

When using Markdown, such a pre-formatted block would begin with a line such as:

```
 ```tables
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

Source code extracted from this file may be viewed under the 'source/c' directory.

```!main.c
/*
 *  !!! This document has been auto-generated using quasi !!!
 */
```

### Dependencies

'Extract' depends on functions declared in the following header files.

```main.c
#include <fcntl.h>
```

This header is included to provide access to the 'fcntl' function:
'fcntl' is used to make the file descriptors non-blocking.

```main.c
#include <stdlib.h>
```

This header is included to provide access to the 'calloc', 'realloc', and 'free' functions:
'calloc' is used to allocate memory that has been zeroed,
'realloc' is used in the 'readline' function to increase the size of the line buffer,
while 'free' is used to deallocate allocated memory.

```main.c
#include <stdio.h>
```

This header is included to provide access to the 'fopen', 'fclose', and 'fprintf' functions:
'fopen' is used to open the files indicated by the command-line arguments, as well as open '/dev/null',
'fclose' is used to close those files, and
'fprintf' is used to write pre-formatted text blocks to the standard output stream.


```main.c
#include <string.h>
```

This header is included to provide access to the 'strdup', 'strcmp', 'strncmp', 'strlen', and 'stpcpy' functions:
'strdup' is used to duplicate the passed string,
'strcmp' and 'strncmp' are used to compare whether two strings are equal ('strcmp'), or whether one is a prefix of the other ('strncmp').
'strlen' is used to find the length of a string, and
'stpcpy' is used to concatenate a prefix, pattern, and suffix to create the appropriate line delimiter for either Markdown or MaxText.


```main.c
#include <curl/curl.h>
```

This header is included to provide access to the 'curl' related functions
'curl_global_init',
'curl_global_cleanup',
'curl_easy_init',
'curl_easy_escape',
'curl_easy_setopt',
'curl_easy_perform',
'curl_easy_cleanup', and
'curl_free';
which are used to call the 'spgen' webservice in order to translate any 'spgen' specifications into SQL statements.

In the future,
once 'spgen' as implemented as a command line executable,
this dependency will be removed and extract will instead be called locally.

### Defined types and values

The 'int' type is typecast to a 'bool' for readability.

```main.c
typedef int bool;
```

The 'Patterns' abstract data type is used to create and associated a buffer with each pattern.

```main.c
typedef struct _Patterns Patterns;
```

### Definition of booleans

Also for readability, the booleans TRUE and FALSE are defined.

```main.c
#define FALSE 0;
#define TRUE 1;
```

### Global state

The implementation has the following global variables:
If 'DEBUG' is set to 'TRUE', extra debugging lines are output to the standard error stream.
If 'STRIP' is set to 'TRUE', extract will strip lines beginning with 'DELIMITER' from the ouput.
The 'DEV_NULL' global variable is used to open the file '/dev/null' to which unwanted content is printed.
Previously, this file was being opened and closed numerous times, which led to execution instability.

```main.c
static bool  DEBUG    = FALSE;
static bool  STRIP    = FALSE;
static FILE* DEV_NULL = NULL;
```

### High-level function structure

'Extract' has a simple call-graph, which is shown below:

```
main
 |-- argumentsContains -- stringEquals
 |-- argumentsGetValue -- stringEquals
 |-- usage
 |-- Patterns_new
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
if so, calling tryToProcess on each file,
or;
if not, calling the 'usage' function then exiting.

```main.c
bool argumentsContains( int argc, char** argv, char* flag );
```

The 'argumentsContains' function check each of the arguments passed to main to see if it matches the specified flag.

```main.c
char* argumentsGetValue    ( int n, char** files, char* flag );
char* argumentsGetValueList( int n, char** files, char* flag );
```

Extract now supports either:
the original mode where the flag '-p' is passed followed by a single pattern;
or a new multi-pattern mode where '-P' is passed followed by a list of patterns.
Thus each execution of 'extract' can now extract the content of multiple patterns while only processing files once.

The 'argumentsGetValue' function retrieves (if appropriate) the value following a specific flag.
The 'argumentsGetValueList' returns a comma separated list of arguments that follow a flag and are terminated by the argument '--'.

```main.c
int usage();
```

The 'usage' function simply prints out the usage string and always returns -1.

```main.c
Patterns* Patterns_Populate( const char* patterns );
```

The 'Patterns_Populate' function takes a comma-separated list of one or more patterns and
initialised the 'Patterns' structure which maintains:

1)  An array of pattern names
2)  An array of corresponding memstream objects
3)  An array of memory buffers
4)  An array of mem sizes
5)  The value 'next', which is used during population of the structure
6)  The value 'max', which is keeps track of the total number of patterns that were passed.

```main.c
Patterns* Patterns_new( int maxSize );
```

The function 'Patterns_new' is used to initialise the Patterns object.

```main.c
int Patterns_add( Patterns* self, const char* pattern );
```

The function 'Patterns_add' is used to populate each array with an entry corresponding to that pattern added.

```main.c
FILE* Patterns_retrieveOutputStream( Patterns* self, const char* pattern );
```

Later the 'Patterns_retrieveOutputStream' function is used to retrieve the mem stream associated with a pattern.

```main.c
void tryToProcess( char* file, Patterns* p );
```

The 'tryToProcess' function tries to open the passed file, and if successful passes the opened input stream to
'process' along with the passed line pattern delimiter.

```main.c
void process( FILE* stream, Patterns* p );
```

The 'process' function reads each line of the passed stream,
and if the passed line pattern delimiter is found outputs any further lines to the mem stream associated with that pattern
until an end delimiter is encountered.

```main.c
char* readline( FILE* );
char* readline2( FILE* );
```

The 'readline' function reimplements the UNIX readline function for portability.

!
```
char* generateDelimiter( const char* prefix, const char* pattern, const char* suffix );
```

The 'generateDelimiter' function is used to generate the line delimiter to be searched for in the input files.
!

```main.c
void processPreformatted( const char* line, FILE* stream, Patterns* p );
```

If the strip ('-s') flag has been used, the 'processPreformatted' function is used to strip out lines starting with specific keywords such as 'DELIMITER', or 'DROP',
which (in the case of SQL) it is often desirous to remove for installation scripts.

```main.c
char* extractPattern( const char* line );
```

The 'extractPattern' function is used to extract a pattern from the first line of a pre-formatted text section.

```main.c
char* canonicaliseSPGenURL( const char* url );
```

If an SPGen pattern (~spgen~) is encountered the content of the preformatted block is passed in a request
to generate SQL.
The 'canonicaliseSPGenURL' function is used to remove any whitespace from the URL.


```main.c
char* stringAppend( char* orig, char sep, const char* suffix );
```

The 'stringAppend' function concatenates two strings and returns a pointer to a combined string.

```main.c
int stringEquals( const char* one, const char* two );
```

The 'stringEquals' function compares whether the two passed strings contain the same characters.

```main.c
char* stringCopy( const char* src );
```

The 'stringCopy' function copies and returns a string.

```main.c
int stringHasPrefix( const char* string, const char* prefix );
```

The 'stringHasPrefix' function determins whether the passed string has the passed prefix.

```main.c
void Patterns_printMemstreamsTo( Patterns* p, FILE* out );
```

The 'Patterns_printMemstreamsTo' function will write the content of all memory streams to standard out.

```main.c
Patterns* Patterns_free( Patterns* self );
```

The 'Patterns_free' function releases all memory allocatted related to patterns.

### Low-level implemention function descriptions

#### Function: main

The main function first processes the passed arguments to retrieve the pattern,
and to determine whether the *strip* flag has been passed, e.g., ('-s').
The caller may either pass a lower case 'p' flag followed by a pattern ( '-p <pattern>' ),
or an uppercase 'P' flag followed by a list of patterns terminated by a '--' argument.

If a pattern has not been passed, or the number of arguments passed are less than expected,
the usage() function is called and exit is called.

Otherwise, each file path in the arguments is passed to the 'tryToProcess' function
along with the Patterns object.

```main.c
int main( int argc, char** argv )
{
    curl_global_init( CURL_GLOBAL_ALL );

    DEV_NULL  = fopen( "/dev/null", "a" );
    STRIP     = argumentsContains( argc, argv, "-s" ) ? 1 : 0;
    char* pat = argumentsGetValue( argc, argv, "-p" );

    if ( !pat )
    {
        pat = argumentsGetValueList( argc, argv, "-P" );
    }
                                        // 1.       2.  3.  4.            5.
    int expected_arguments = 4 + STRIP; // extract [-s] -p "some patterns" file...

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

        Patterns* p = Patterns_Populate( pat );

        int i;
        for ( i = 1; i < argc; i++ )
        {
            if ( '-' != argv[i][0] )
            {
                tryToProcess( argv[i], p );
            }
            else
            if ( 'p' == argv[i][1] )
            {
                i++;
            }
        }

        Patterns_printMemstreamsTo( p, stdout );
        
        Patterns_free( p );
    }
    free( pat );

    curl_global_cleanup();

    return 0;
}
```

#### Function: argumentContains

The 'argumentContains' function iterates through the passed arguments and returns TRUE if the argument is found;
otherwise FALSE.

```main.c
int argumentsContains( int n, char** files, char* flag )
{
    int b = FALSE;
    int i;

    for ( i=0; i < n; i++ )
    {
        if ( stringEquals( flag, files[i] ) )
        {
            b = TRUE;
            i = n;
        }
    }
    return b;
}
```

#### Function: argumentGetValue

The 'argumentGetValue' function iterates through the passed arguments and
if the passed flag is found returns the following argument;
otherwise returns NULL.

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

#### Function: argumentsGetValueList

The 'argumentsGetValueList' function iterates through the passed arguments and
if the passed flag is found returns each of the following arguments until a '--' argument is found
as a comma separated string;
otherwise returns NULL.

```main.c
char* argumentsGetValueList( int n, char** files, char* flag )
{
    char* ret = NULL;
    int i;

    for ( i=0; i < n; i++ )
    {
        if ( stringEquals( flag, files[i] ) )
        {
            i++;
            for ( i=i; i < n; i++ )
            {
                if ( stringEquals( "--", files[i] ) )
                {
                    i = n;
                }
                else
                {
                    ret = stringAppend( ret, ',', files[i] );
                }
            }
        }
    }
    return ret;
}
```

#### Function: usage

The 'usage' function prints out expected usage to standard error.

```main.c
int usage()
{
    fprintf( stderr, "Usage:\n\textract [-s] [-p <pattern>] [-P <pattens,...,'--'>] [source file,...]\n" );
    return -1;
}
```

#### Function: tryToProcess

The 'tryToProcess' function attempts to open the passed file path and
if successful passes the opened file stream and patterns to the 'process' function.

```main.c
void tryToProcess( char* file, Patterns* p )
{
    FILE* stream = NULL;
    if ( (stream = fopen( file, "r" )) )
    {
        if ( DEBUG ) fprintf( stderr, "Processing: %s\n", file );

        process( stream, p );

        fclose( stream );
    }
}
```

#### Function: process

The process function reads each line of passed input stream using the 'readline' function.
If a line starts with a tilde ('~') or with a Markdown comment prefix ('```'),
the line, stream, and patterns are passed to 'processPreformatted',
which will process each line until an end delimiter is found.

```main.c
void process( FILE* stream, Patterns* p )
{
    char* line;
    int   use_new = 1;

    do
    {
        line = readline( stream );
        if ( line )
        {
            switch ( line[0] )
            {
            case '~':
                if ( DEBUG ) fprintf( stderr, "@%s", line );
                if ( 1 ) processPreformatted( line, stream, p );
                break;
        
            case '`':
                if ( ('`' == line[1]) && ('`' == line[2]) )
                {
                    if ( DEBUG ) fprintf( stderr, "@%s", line );
                    if ( 1 ) processPreformatted( line, stream, p );
                }
                break;
        
            default:
                if ( DEBUG ) fprintf( stderr, "@%s", line );
            }
            free( line );
        }
    } while ( line );
}
```

!
#### Function: generateDelimiter

```
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
!

#### Function: processPreformatted

If the passed line matches a pattern in 'p',
we will copy characters from 'in' into 'out'; otherwise we will copy to dev null.

However, if that pattern is 'spgen',
we want to process that content using the spgen web service then copy the response to out.
To achive this we will use a dummy stream called 'os'.
If the pattern is 'spgen', we will make 'os' be a temporary buffer, otherwise it will be out.



If the pattern is "spgen",
we want to read the characters into a temporary buffer 'buf',
which we will then use to query spgen.org,
and then we will write the response to out.


```main.c
void processPreformatted( const char* line, FILE* in, Patterns* p )
{
    char* pattern = extractPattern( line );
    FILE* out     = Patterns_retrieveOutputStream( p, pattern );
    bool  writing = (NULL != out);

    if ( !out )
    {
        out = DEV_NULL;
    }

    int    loop = 1;
    char*  pre  = NULL;
    char*  bp   = NULL;
    size_t size = 0;

    FILE*  buf  = NULL;
    FILE*  os   = NULL;

    if ( writing && stringEquals( "spgen", pattern ) )
    {
        buf = open_memstream( &bp, &size );
        if ( !buf )
        {
            perror( "Could not instantiate mem stream." );
            exit( -1 );
        }
        os  = buf;
    }
    else
    {
        os = out;
    }

    do
    {
        pre = readline( in );
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
                    if ( '/' == pre[1] ) fprintf( os, ";\n" );
                    break;

                case 'D':
                case 'd':
                    if ( stringHasPrefix( pre, "DROP"      ) ) break;
                    if ( stringHasPrefix( pre, "DELIMITER" ) ) break;
                    if ( stringHasPrefix( pre, "drop"      ) ) break;
                    if ( stringHasPrefix( pre, "delimiter" ) ) break;
                    // fall through

                default:
                    fprintf( os, "%s", pre );
                }
            }
            else
            {
                switch ( pre[0] )
                {
                case '~':
                case '`':
                    if ( DEBUG ) fprintf( stderr, "@%s", pre );
                    loop = 0;
                    break;

                case 'D':
                case 'd':
                    if ( stringHasPrefix( pre, "DROP"      ) ) break;
                    if ( stringHasPrefix( pre, "drop"      ) ) break;
                    // fall through

                default:
                    fprintf( os, "%s", pre );
                }
            }

            free( pre );
        } else {
            loop = 0;
        }
    }
    while ( loop );

    fflush( os );

    /* If 'buf' is set we need to use it's contents and call 'spgen' and write the result to 'out'. */

    if ( buf )
    {
        if ( stringEquals( "spgen", pattern ) )
        {
            char* host     = "http://sqlgen.azurewebsites.net/api/sqlgenerate/";
            char* field    = "table_info=";
            char* data     = canonicaliseSPGenURL( bp );

            void* handle   = curl_easy_init();
            char* encoded  = curl_easy_escape( handle, data, 0 );
            char* postdata = calloc( strlen( field ) + strlen( encoded ) + 1, sizeof(char) );
            {
                sprintf( postdata, "%s%s", field, encoded );

                curl_easy_setopt ( handle, CURLOPT_URL,        host     );
                curl_easy_setopt ( handle, CURLOPT_POST,       1L       );
                curl_easy_setopt ( handle, CURLOPT_POSTFIELDS, postdata );
                curl_easy_setopt ( handle, CURLOPT_WRITEDATA,  out      ); // <------ Writing to 'out'
                curl_easy_perform( handle );
                curl_easy_cleanup( handle );
            }
            free( postdata );
            curl_free( encoded );
        }
        fclose( buf );
    }

    fflush( out );
}
```

```main.c
char* extractPattern( const char* line )
{
    char* pattern = NULL;
    {
        char* copy = stringCopy( line );
        char* tmp  = copy;
        int   len;

        while ( ('~' == *tmp) || ('`' == *tmp) ) tmp++;

        len = strlen( tmp );

        int loop = TRUE;
        do
        {
            switch ( tmp[len-1] )
            {
                case ' ':
                case '\t':
                case '\n':
                case '~':
                case '`':
                    tmp[len-1] = '\0';
                    len--;
                    break;
                
                default:
                    loop = FALSE;
            }
        }
        while ( len && loop );

        pattern = stringCopy( tmp );

        free( copy );
    }
    return pattern;
}
```

```main.c
char* canonicaliseSPGenURL( const char* url )
{
    int   len       = strlen( url );
    char* canonical = calloc( len + 1, sizeof(char) );

    int index = 0;
    for ( int i=0; i < len; i++ )
    {
        switch ( url[i] )
        {
        case ' ':
        case '\t':
        case '\n':
            break;

        default:
            canonical[index++] = url[i];
        }
    }

    return canonical;
}
```



### General purpose functions

The following functions are general purpose, and can be used by any program.

#### Function: readline

```main.c
char* readline( FILE* stream )
{
    int  n     = 0;
    int  sz    = 10000;
    char ch[2] = { 0, 0 };
    char* line = calloc( sz + 1, sizeof( char ) );

    int read;
    do
    {
        read = fread( ch, sizeof(char), 1, stream );
        if ( -1 == read )
        {
            read = 0;
        }
        else
        if ( read > 0 )
        {
            if ( *ch > 127 ) fprintf( stderr, "%s", "(!!)" );

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
                fprintf( stderr, "%s", "#" );
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


```main.c
char* readline2( FILE* stream )
{
    int   n    = 0;
    int   sz   = 10000;
    int   chx  = 0;
    char  ch   = 0;
    char* line = calloc( sz + 1, sizeof( char ) );

    do
    {
        chx = fgetc( stream );
        ch  = (char) chx;
        if ( EOF != chx )
        {
            if ( chx > 127 ) fprintf( stderr, "%s", "(!!)" );

            switch ( ch )
            {
            case '\n':
                line[n++] = '\0';
                chx       = EOF;
                break;

            default:
                line[n++] = (char) ch;
                line[n]   = '\0';
            }

            if ( n == sz )
            {
                sz  *= 2;
                line = realloc( line, sz );
                fprintf( stderr, "%s", "#" );
            }
        }
        
    }
    while ( EOF != chx );

    if ( 0 == n )
    {
        free( line );
        line = NULL;
    }

    return line;
}
```

#### Function: setNonBlocking

```main.c
void setNonBlocking( int fd )
{
    int flags = fcntl( fd, F_GETFL, 0 );

    if ( -1 == flags )
    {
        perror( "F_GETFL" );
    }
    else
    if ( -1 == fcntl( fd, F_SETFL, flags | O_NONBLOCK ) )
    {
        perror( "F_SETFL" );
    }

    //fcntl( fd, F_SETPIPE_SZ, 64000 );
}
```

#### Function: stringAppend

```main.c
char* stringAppend( char* orig, char sep, const char* suffix )
{
    char* dst = NULL;

    if ( orig )
    {
        int len_orig   = strlen( orig   );
        int len_suffix = strlen( suffix );
        int len_sep    = 1;
        int len_new    = len_orig + len_suffix + len_sep + 1;

        dst = calloc( len_new, sizeof( char ) );

        strncpy( dst,                        orig, len_orig   );
        strncpy( dst + len_orig,             &sep, len_sep    );
        strncpy( dst + len_orig + len_sep, suffix, len_suffix );

        free( orig );
    }
    else
    {
        dst = stringCopy( suffix );
    }

    return dst;
}
```

#### Function: stringCopy

```main.c
char* stringCat( const char* a, const char* b )
{
    int   n1  = strlen( a );
    int   n2  = strlen( b );
    char* dst = calloc( n1 + n2 + 1, sizeof( char ) );

    strncpy( dst,      a, n1 );
    strncpy( dst + n1, b, n2 );

    return dst;
}
```

#### Function: stringCopy

```main.c
char* stringCopy( const char* src )
{
    int   n   = strlen( src );
    char* dst = calloc( n + 1, sizeof( char ) );

    strncpy( dst, src, n );

    return dst;
}
```

#### Function: stringEquals

```main.c
int stringEquals( const char* one, const char* two )
{
    int l1 = strlen( one );
    int l2 = strlen( two );

    return (l1 == l2) && (0 == strncmp( one, two, l1 ));
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

#### ADT: Patterns

```main.c
struct _Patterns
{
    char**   names;
    FILE**   memstreams;
    char**   membuffers;
    size_t*  memsize;

    int      next;
    int      max;
};
```

```main.c
Patterns*
Patterns_Populate( const char* patterns )
{
    char* copy  = stringCopy( patterns );
    char* first = copy;
    char* end   = copy;
    int   max   = 1;

    //  First count the number of extra patterns - number of commas.

    while ( *end )
    {
        if ( ',' == *end ) max++;

        end++;
    }

    Patterns* p = Patterns_new( max );

    //  Reset the end point back to start

    end = copy;

    while ( *end )
    {
        if ( ',' == *end )
        {
            if ( first != end )
            {
                *end = '\0';

                Patterns_add( p, first );

                first = end + 1;
            }
        }

        end++;
    }

    if ( first != end )
    {
        Patterns_add( p, first );
    }

    free( copy );

    return p;
}
```

```main.c
Patterns*
Patterns_new( int maxSize )
{
    Patterns* self = calloc( 1, sizeof( Patterns ) );
    if ( self )
    {
        self->names      = calloc( maxSize, sizeof( char*   ) );
        self->memstreams = calloc( maxSize, sizeof( FILE*   ) );
        self->membuffers = calloc( maxSize, sizeof( char*   ) );
        self->memsize    = calloc( maxSize, sizeof( size_t  ) );

        self->next    = 0;
        self->max     = maxSize;
    }

    return self;
}
```

```main.c
Patterns*
Patterns_free( Patterns* self )
{
    if ( self )
    {
        for ( int i=0; i < self->max; i++ )
        {
            free  ( self->names[i]      );
            fclose( self->memstreams[i] );
        }

        self->next = 0;
        self->max  = 0;
    }
    free( self );

    return NULL;
}
```

```main.c
int
Patterns_add( Patterns* self, const char* pattern )
{
    if ( self->next == self->max )
    {
        return 0;
    }
    else
    {
        FILE* memstream
        =
        open_memstream
        (
            &(self->membuffers[self->next]),
            &(self->memsize[self->next])
        );

        if ( !memstream )
        {
            perror( "Could not instantiate memstream." );
            exit( -1 );
        }

        self->names     [self->next] = stringCopy( pattern  );
        self->memstreams[self->next] = memstream;
        self->next++;

        return 1;
    }
}
```

```main.c
FILE*
Patterns_retrieveOutputStream( Patterns* self, const char* pattern )
{
    FILE* os = NULL;
    int   n  = self->next;

    for ( int i=0; i < n; i++ )
    {
        if ( stringEquals( self->names[i], pattern ) )
        {
            os = self->memstreams[i];
        }
    }

    return os;
}
```

```main.c
void Patterns_printMemstreamsTo( Patterns* self, FILE* out )
{
    FILE*  stream;
    char*  buffer;
    size_t size;
    int   n = self->next;

    for ( int i=0; i < n; i++ )
    {
        stream = self->memstreams[i];
        buffer = self->membuffers[i];
        size   = self->memsize[i];

        fflush( stream );

        fprintf( out, "%s", buffer );
    }
}
```
