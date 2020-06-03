/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>
#include <string>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;
static std::string cur_string;
int nul_char;


extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

static int comment_depth = 0;

%}

/*
 * Define names for regular expressions here.
 */

DARROW          =>
ASSIGN		<-
LE		<=
DIGIT		[0-9]
INTEGER		{DIGIT}+
%x		COMMENT_LINE COMMENT_BLOCK STR

%%

 /*
  *  Nested comments
  */
<INITIAL>--					{ BEGIN COMMENT_LINE; }

<COMMENT_LINE>[^\n]*				{}

<COMMENT_LINE>[\n]				{
							++curr_lineno; 
							BEGIN INITIAL; 
						}

<COMMENT_LINE><<EOF>>				{ BEGIN 0; }

<INITIAL,COMMENT_LINE,COMMENT_BLOCK>"(*"	{ 
							++comment_depth; 
							BEGIN COMMENT_BLOCK;
						}

<COMMENT_BLOCK>[^\n(*)]*			{}

<COMMENT_BLOCK>[()*]				{}

<COMMENT_BLOCK>"*)" 				{
							--comment_depth;
							if(comment_depth == 0)
								BEGIN INITIAL;
						}

<COMMENT_BLOCK>\n				curr_lineno++;

<COMMENT_BLOCK><<EOF>>				{ 
							BEGIN INITIAL; 
							cool_yylval.error_msg = "EOF in comment."; 
							return ERROR; 
						}

"*)"						{
							BEGIN INITIAL; 
							cool_yylval.error_msg = "Unmatched *)"; 
							return ERROR; 
						}

 /*
  *  The multiple-character operators.
  */
"<-"						{ return ASSIGN; }
"<="						{ return LE; }
"=>"						{ return DARROW; }
"+"						{ return int('+'); }
"-"						{ return int('-'); }
"*"						{ return int('*'); }
"/"						{ return int('/'); }
"<"						{ return int('<'); }
"="						{ return int('='); }
"."						{ return int('.'); }
";"						{ return int(';'); }
"~"						{ return int('~'); }
"{"						{ return int('{'); }
"}"						{ return int('}'); }
"("						{ return int('('); }
")"						{ return int(')'); }
":"						{ return int(':'); }
"@"						{ return int('@'); }
","						{ return int(','); }
 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */
(?i:class)					{ return CLASS; }
(?i:else)					{ return ELSE; }
(?i:if)						{ return IF; }
(?i:fi)						{ return FI; }
(?i:in)						{ return IN; }
(?i:inherits)					{ return INHERITS; }
(?i:let)					{ return LET; }
(?i:loop)					{ return LOOP; }
(?i:pool)					{ return POOL; }
(?i:then)					{ return THEN; }
(?i:while)					{ return WHILE; }
(?i:case)					{ return CASE; }
(?i:esac)					{ return ESAC; }
(?i:of)						{ return OF; }
(?i:new)					{ return NEW; }
(?i:isvoid)					{ return ISVOID; }
(?i:not)					{ return NOT; }

f(?i:alse)					{
							cool_yylval.boolean = false; 
							return BOOL_CONST;
						}

t(?i:rue)					{
							cool_yylval.boolean = true; 
							return BOOL_CONST; 
						}

[a-z][A-Za-z0-9_]*				{ 
						   cool_yylval.symbol = idtable.add_string(yytext);
 						   return OBJECTID; 
						}

[A-Z][A-Za-z0-9_]*				{ 
						   cool_yylval.symbol = idtable.add_string(yytext);
						   return TYPEID; 
						}

{INTEGER}					{ 
						   cool_yylval.symbol = inttable.add_string(yytext);
						   return INT_CONST; 
						}

"\n"						{curr_lineno++;}

[ \f\r\t\v]+ ;

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
<INITIAL>\"		{
				BEGIN STR; 
			}

<STR>[^\\\"\n]*		{
				yymore();
			}

<STR>\\\n		{
				++curr_lineno;
				yymore();
			}

<STR>\\[^\n]		{
				yymore();
			}

<STR>\n			{
				BEGIN INITIAL;
				++curr_lineno;
				cool_yylval.error_msg = "Unterminated string";
				return ERROR;
			}

<STR><<EOF>>		{
				BEGIN INITIAL;
				yyrestart(yyin);
				cool_yylval.error_msg = "EOF in string";
				return ERROR;
			}

<STR>\"			{
				BEGIN INITIAL;
				std::string cur_string(yytext,yyleng-1);
				std::string out_string = "";
				
				if(cur_string.find_first_of('\0') != std::string::npos) {
					cool_yylval.error_msg = "Unterminated string constant";
					return ERROR;
				}
				std::string::size_type pos;
				while((pos = cur_string.find('\\')) != std::string::npos) {
					out_string += cur_string.substr(0,pos);
					switch(cur_string[pos+1]) {
						case 't':
							out_string += '\t';
							break;
						case 'f':
							out_string += '\f';
							break;
						case 'b':
							out_string += '\b';
							break;
						case 'n':
							out_string += '\n';
							break;
						default:
							out_string += cur_string[pos+1];
							break;
					}
					cur_string = cur_string.substr(pos+2);
				}
				out_string += cur_string;
				if(out_string.size() >= MAX_STR_CONST) {
					cool_yylval.error_msg = "String constant too lang";
					return ERROR;
				}
				cool_yylval.symbol = stringtable.add_string((char*)out_string.c_str());
				return STR_CONST;
			}

.			{
				cool_yylval.error_msg = yytext;
				return ERROR;
			}
%%
