/*
 *  !!! This document has been auto-generated using quasi !!!
 */

#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include <curl/curl.h>

typedef int bool;

#define FALSE 0;
#define TRUE 1;

static bool DEBUG = FALSE;
static bool STRIP = FALSE;

int main( int argc, char** argv );

bool argumentsContains( int argc, char** argv, char* flag );

char* argumentsGetValue( int n, char** files, char* flag );

int usage();

void tryToProcess( char* file, const char* pattern );

void process( FILE*, const char* pattern );

char* readline( FILE* );

char* generateDelimiter( const char* prefix, const char* pattern, const char* suffix );

void processPreformatted( const char* line, FILE*, const char* pattern );

char* canonicaliseSPGenURL( const char* url );

int stringEquals( const char* one, const char* two );

int stringHasPrefix( const char* string, const char* prefix );

int main( int argc, char** argv )
{
    curl_global_init( CURL_GLOBAL_ALL );

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

    curl_global_cleanup();

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
    char* line_delimiter = NULL;
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
                free( line_delimiter );
                break;

            case '`':
                line_delimiter = generateDelimiter( "```", pattern, "" );
                if ( ('`' == line[1]) && ('`' == line[2]) )
                {
                    if ( DEBUG ) fprintf( stderr, "@%s", line );
                    processPreformatted( line, stream, line_delimiter );
                }
                free( line_delimiter );
                break;

            default:
                if ( DEBUG ) fprintf( stderr, "@%s", line );
            }
            free( line );
        }
    } while ( line );
}

char* generateDelimiter( const char* prefix, const char* pattern, const char* suffix )
{
    char* delimiter = calloc( strlen( prefix ) + strlen( pattern ) + strlen( suffix ) + 1, sizeof(char) );
    char* tmp       = delimiter;
          tmp       = stpcpy( tmp, prefix  );
          tmp       = stpcpy( tmp, pattern );
          tmp       = stpcpy( tmp, suffix  );

    return delimiter;
}

void processPreformatted( const char* line, FILE* stream, const char* line_delimiter )
{
    int    loop = 1;
    char*  pre;
    char*  bp;
    size_t size;

    FILE* dev_null = fopen( "/dev/null", "a" );
    FILE* out      = dev_null;

    if ( stringHasPrefix( line, line_delimiter ) )
    {
        if ( stringEquals( "~spgen~", line_delimiter ) )
        {
            out = open_memstream( &bp, &size );
        }
        else
        {
            out = stdout;
        }
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
                    if ( stringHasPrefix( pre, "DROP"      ) ) break;
                    if ( stringHasPrefix( pre, "DELIMITER" ) ) break;
                    if ( stringHasPrefix( pre, "drop"      ) ) break;
                    if ( stringHasPrefix( pre, "delimiter" ) ) break;
                    // fall through

                default:
                    fprintf( out, "%s", pre );
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
                    if ( stringHasPrefix( pre, "DROP"      ) ) break;
                    if ( stringHasPrefix( pre, "drop"      ) ) break;
                    // fall through

                default:
                    fprintf( out, "%s", pre );
                }
            }

            free( pre );
        } else {
            loop = 0;
        }
    }
    while ( loop );

    fflush( out );

    if ( stringHasPrefix( line, line_delimiter ) )
    {
        if ( stringEquals( "~spgen~", line_delimiter ) )
        {
            char* host     = "http://sqlgen.azurewebsites.net/api/sqlgenerate/";
            char* field    = "table_info=";
            char* data     = canonicaliseSPGenURL( bp );

            void* handle   = curl_easy_init();
            char* encoded  = curl_easy_escape( handle, data, 0 );
            char* postdata = calloc( strlen( field ) + strlen( encoded ) + 1, sizeof(char) );
            {
                sprintf( postdata, "%s%s", field, encoded );

                curl_easy_setopt ( handle, CURLOPT_URL, host );
                curl_easy_setopt ( handle, CURLOPT_POST, 1L );
                curl_easy_setopt ( handle, CURLOPT_POSTFIELDS, postdata );
                curl_easy_perform( handle );
                curl_easy_cleanup( handle );
            }
            free( postdata );
            curl_free( encoded );

            fclose( out );
        }
    }

    fclose( dev_null );
}

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
        if ( read )
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

int stringEquals( const char* one, const char* two )
{
    int l1 = strlen( one );
    int l2 = strlen( two );

    return (l1 == l2) && (0 == strncmp( one, two, l1 ));
}

int stringHasPrefix( const char* string, const char* prefix )
{
    int len = strlen( prefix );

    return (0 == strncmp( string, prefix, len ));
}

