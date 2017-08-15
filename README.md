# Extract

'Extract' is a command line tool for extracting SQL and other code
from marked up text files that are able to represent pre-formated text sections.

Currently, two such text formats are explicitly supported - Markdown and MaxText.
MaxText is a plain text markup system that is similar conceptually to Markdown.

Markdown uses a triplet of back-ticks ("```") in the left-most column to indicate the start and end of a pre-formatted text section, while Maxtext uses a tilde ('~') character.

```c
int main( int argc, char** argv )
{
	return 0;
}
```

Techically, both text systems ignore any remaining text on the line,
however, some systems such as github, allow remaining text to hint at a programming language, e.g:

```c`
int main( int argc, char** argv )
{
	return 0;
}
```
