/*
 *		Extract
 *
 *		This is a generalisatin of the extract SQL program -- its purpose
 *		is to extract pre-formatted segments from MaxText documentation
 *		source files.
 *
 *		If the first argument is "-p", the second argument is treated as a
 *		"pattern" specifier. The pattern matched is '~' . <pattern> . '~'.
 *
 *		Further arguments to the command-line are treated as MaxText files. Each
 *		file is (read-only) processed in turn and output to the standard out.
 *
 *		For each file lines are output to stdout if they existing between a
 *		line that starts with the specified pattern and a line that starts with "~".
 *
 *		Usage:
 *
 *			extract -p "sql" <MaxText Files> [ > output file]
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

