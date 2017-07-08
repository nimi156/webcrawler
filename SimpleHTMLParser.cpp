
#include "SimpleHTMLParser.h"
#include "openhttp.h"
#include <string.h>

SimpleHTMLParser::SimpleHTMLParser()
{
}

bool
SimpleHTMLParser::match(char **b, const char *m) {
	int len = strlen(m);
	if ( !strncasecmp(*b, m, len)) {
		(*b)+=len;
		return true;
	}
	return false;
}

bool
SimpleHTMLParser::parse(char * buffer, int n)
{
	enum { START, TAG, SCRIPT, ANCHOR, HREF,
	       COMMENT, FRAME, SRC, META, TITLE} state;

	state = START;
	
	char * bufferEnd = buffer + n;
	char * b = buffer;
	bool lastCharSpace = false;
	while (b < bufferEnd) {
		switch (state) {
		case START: {
			if (match(&b,"<SCRIPT")) {
				state = SCRIPT;
			}
			else if (match(&b,"<!--")) {
				state = COMMENT;
			}
			else if (match(&b,"<A ")) {
		printf("<%c,%d,%d>\n", *b, *b,state);
				state = ANCHOR;
			}
			else if (match(&b,"<FRAME ")) {
				state = FRAME;
			}
			else if	(match(&b,"<")) {
				state = TAG;
			} 
			else if (match(&b, "<META CONTENT=\"")) {
				state = META;
			}
			else if (match(&b, "<TITLE>")){
				state = TITLE;
			}
			else {
				char c = *b;
				//Substitute one or more blank chars with a single space
				if (c=='\n'||c=='\r'||c=='\t'||c==' ') {
					if (!lastCharSpace) {
					//	onContentFound(' ');
					}
					lastCharSpace = true;
				}
				else {
					//onContentFound(c);
					lastCharSpace = false;
				}
				
				b++;
			}
			break;
		}
		case ANCHOR: {
			if (match(&b,"href=\"")) {
				state = HREF;
				urlAnchorLength=0;
				//printf("href=");
			}
			else if (match(&b,">")) {
				// End script
				state = START;
			}
			else {
				b++;
			}
			break;
				
		}
		case HREF: {
			if (match(&b,"\"")) {
				// Found ending "
				state = ANCHOR;
				urlAnchor[urlAnchorLength] = '\0';
				onAnchorFound(urlAnchor);
				//printf("\n");
			}
			else {
				if ( urlAnchorLength < MaxURLLength-1) {
					urlAnchor[urlAnchorLength] = *b;
					urlAnchorLength++;
				}
				//printf("%c", *b, *b);
				b++;
			}
			break;
		}
		case FRAME: {
			if (match(&b,"src=\"")) {
				state = SRC;
				urlAnchorLength=0;
				//printf("href=");
			}
			else if (match(&b,">")) {
				// End script
				state = START;
			}
			else {
				b++;
			}
			break;
				
		}
		case SRC: {
			if (match(&b,"\"")) {
				// Found ending "
				state = FRAME;
				urlAnchor[urlAnchorLength] = '\0';
				onAnchorFound(urlAnchor);
				//printf("\n");
			}
			else {
				if ( urlAnchorLength < MaxURLLength-1) {
					urlAnchor[urlAnchorLength] = *b;
					urlAnchorLength++;
				}
				//printf("%c", *b, *b);
				b++;
			}
			break;
		}
		case SCRIPT: {
			if (match(&b,"/SCRIPT>")) {
				// End script
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		case COMMENT: {
			if (match(&b,"-->")) {
				// End comments
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		case TAG: {
			if (match(&b, ">")) {
				state = START;
			}
			else {
				b++;
			}
			break;
		}
		case META: {
			//if (match(&b, "/>")){
	printf("META\n");
			if (match(&b, "NAME=\"DESCRIPTION\"/>")){
				onContentFound('{');
				state = START;
			} else if (match(&b, "/>")) {
				onContentFound('}');
				state = START;
			} else {
				onContentFound(*b);
				b++;
			}
			break;
		}
		case TITLE: {
			if (match(&b, "</TITLE>")){
				onContentFound('{');
				state = START;
			} else {
				onContentFound(*b);
				b++;
			}
		}

		default:;
		}
		
	}
}

void
SimpleHTMLParser::onContentFound(char c) {
}

void
SimpleHTMLParser::onAnchorFound(char * url) {
}
