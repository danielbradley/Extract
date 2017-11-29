/*
*        Extract
*
*        This is a generalisatin of the extract SQL program -- its purpose
*        is to extract pre-formatted segments from MaxText documentation
*        source files.
*
*        If the first argument is "-p", the second argument is treated as a
*        "pattern" specifier. The pattern matched is '~' . <pattern> . '~'.
*
*        Further arguments to the command-line are treated as MaxText files. Each
*        file is (read-only) processed in turn and output to the standard out.
*
*        For each file lines are output to stdout if they existing between a
*        line that starts with the specified pattern and a line that starts with "~".
*
*        Usage:
*
*            extract -p "sql" <MaxText Files> [ > output file]
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int DEBUG = 0;

int    argumentsContains();
char*  argumentsGetValue();
int         stringEquals( const char* one, const char* two );
int                usage();
char*    generatePattern( const char* );
void        tryToProcess( char*, const char* );
void             process( FILE*, const char* );
char*           readline( FILE* );
void processPreformatted( char* line, FILE*, const char* pattern );

int STRIP;

int main( int argc, char** argv )
{
    STRIP     = argumentsContains( argc, argv, "-s" ) ? 1 : 0;
    char* pat = argumentsGetValue( argc, argv, "-p" );

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
        char* line_delimiter = generateDelimiter( pat );

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
                tryToProcess( argv[i], pattern );
            }
            else
            if ( 'p' == argv[i][1] )
            {
                i++;
            }
        }

        free( line_delimiter );
    }
    free( pat );

    return 0;
}

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

int usage()
{
    fprintf( stderr, "Usage:\n\textract [-s] -p <pattern> <file> [more files] [ > output file ]\n" );
    return -1;
}

char* generateDelimiter( const char* pat )
{
char* delimiter = calloc( strlen( pat ) + 3, sizeof(char) );
char* tmp       = delimiter;
      tmp       = stpcpy( tmp, "~" );
      tmp       = stpcpy( tmp, pat );
      tmp       = stpcpy( tmp, "~" );

return delimiter;
}

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

        void processPreformatted( char* line, FILE* stream, const char* pattern )
        {
            int   loop = 1;
            char* pre;

            FILE* dev_null = fopen( "/dev/null", "a" );
            FILE* out      = dev_null;
            char* c        = "@";

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

            fclose( dev_null );
        }

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

int stringEquals( const char* one, const char* two )
{
    return (0 == strcmp( one, two ));
}

int stringHasPrefix( const char* string, const char* prefix )
{
    int len = strlen( prefix )

    return (0 == strcmp( string, prefix, len ));
}
