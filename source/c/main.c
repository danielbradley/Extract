/*
 *  !!! This document has been auto-generated using quasi !!!
 */

#include <fcntl.h>

#include <stdlib.h>

#include <stdio.h>

#include <string.h>

#include <curl/curl.h>

#include <errno.h>

typedef int bool;

typedef struct _Patterns Patterns;

#define FALSE 0;
#define TRUE 1;

static bool  DEBUG    = FALSE;
static bool  STRIP    = FALSE;
static FILE* DEV_NULL = NULL;

int main( int argc, char** argv );

bool argumentsContains( int argc, char** argv, char* flag );

char* argumentsGetValue    ( int n, char** files, char* flag );
char* argumentsGetValueList( int n, char** files, char* flag );

int usage();

Patterns* Patterns_Populate( const char* patterns );

Patterns* Patterns_new( int maxSize );

int Patterns_add( Patterns* self, const char* pattern );

FILE* Patterns_retrieveOutputStream( Patterns* self, const char* pattern );

void tryToProcess( char* file, Patterns* p );

void process( FILE* stream, Patterns* p );

char* readline( FILE* );

void processPreformatted( const char* line, FILE* stream, Patterns* p );

char* extractPattern( const char* line );

char* canonicaliseSPGenURL( const char* url );

char* stringAppend( char* orig, char sep, const char* suffix );

int stringEquals( const char* one, const char* two );

char* stringCopy( const char* src );

int stringHasPrefix( const char* string, const char* prefix );

void Patterns_printMemstreamsTo( Patterns* p, FILE* out );

Patterns* Patterns_free( Patterns* self );

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

int usage()
{
    fprintf( stderr, "Usage:\n\textract [-s] [-p <pattern>] [-P <pattens,...,'--'>] [source file,...]\n" );
    return -1;
}

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
            if ( 1 )
            {
                FILE* file = popen( "sqlgen", "w" );
                fprintf( file, "%s", bp );
                pclose( file );
            }
            else
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
        }
        fclose( buf );
    }

    fflush( out );
}

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

char* stringCat( const char* a, const char* b )
{
    int   n1  = strlen( a );
    int   n2  = strlen( b );
    char* dst = calloc( n1 + n2 + 1, sizeof( char ) );

    strncpy( dst,      a, n1 );
    strncpy( dst + n1, b, n2 );

    return dst;
}

char* stringCopy( const char* src )
{
    int   n   = strlen( src );
    char* dst = calloc( n + 1, sizeof( char ) );

    strncpy( dst, src, n );

    return dst;
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

struct _Patterns
{
    char**   names;
    FILE**   memstreams;
    char**   membuffers;
    size_t*  memsize;

    int      next;
    int      max;
};

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

