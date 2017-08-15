# Extract

'Extract' is a command line tool for extracting SQL and other code
from marked up text files that are able to represent pre-formated text sections.

Currently, two such text formats are explicitly supported - Markdown and MaxText.
MaxText is a plain text markup system that is similar conceptually to Markdown.

Markdown uses a triplet of back-ticks ("```") in the left-most column to indicate the start and end of a pre-formatted text section, while Maxtext uses a tilde ('~') character.
Techically, both text systems ignore any remaining text on the line,
however, some systems such as github, allow remaining text to hint at a programming language.

Likewise, 'Extract' uses these superfluous characters as a pattern that identifies the pre-formatted text block.
The following pre-formatted text block begins with the tag "```tables`, which identifies it with the pattern "tables".

```sql
CREATE TABLE users
(
	USER        INT  AUTO_INCREMENT,
	given_name  TEXT DEFAULT '',
	family_name TEXT DEFAULT '',

	PRIMARY KEY (USER)
);
```

Calling extract and supplying the pattern "tables", any such pre-formatted text sections, in any processed files, will be output.

```
extract -p "tables" source/mt/*.txt > sql/tables.sql
```

