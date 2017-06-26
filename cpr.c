/*
	 CPR.C       " Prints 'C' Files with Table of Contents "
					 "             AND LOTS MORE               "

	 Microsoft C Version 6.0

	 Note:  Turbo C Version 1.0 or 1.5 is available as CPR31.ARC 
			  in Borland Forum

Originally Written by :
		 Paul Breslin
		 Human Computing Resources Corp.
		 10 South Mary Street
		 Toronto, Ontario
		 Canada, M4Y 1P9

Modified by :
		Lance E. Shepard     Rick Wise               Blake McBride
		CCI & RIT            CALCULON Corp.          3900 SW 52nd Ave. #803
		Rochester, NY        Rockville, MD 20850     Pembroke Park, FL 33023

		John Stidd           Dale Loftis             Tom Hiscox
		1570 Neston Way      3020 Yorba Linda #F-16  136 Knorr Road
		Los Altos, CA 94022  Fullerton, CA 92631     Monroe, CT 06468  72106,3307

		Dave Perkowski [70650,1420]
		Argo Data Resource Corp.
		15301 Dallas Prkwy.
		Dallas, TX 75248

Special Notes:
	 Compiles with the /AL large model switch to ensure enough
	 space for the Table of Contents.  In MSC v5.1+. malloc will default 
	 to _fmalloc or _nmalloc depending on compile line switch.

	 Tab stops on this version source code have been fully expanded to spaces
	 This takes more space but eliminates the mess when you load it into
	 your tab stops.

*/

/* *************************************************************************
**                        Program Revision History
**
**  By Dale Loftis
**  10-3-87   Changed WIDTH to 132, Tabs are now every 4, modified so
*             that temporary file is deleted if control - break is hit,
**             Improved the clarity of the instructions if no parmeters
**              are given.
**
**  10-17-87  Changed so that no blank lines are printed in between structures.
**
**  11-06-87  Added -@ RESPONSE FILE feature since you couldn't fit all the
**             filenames onto the command line, upto 400 files can be printed.
**
**            Also adding the "Reading file # x of y" string and made program
**             check the amount of free disk space before writing the files
**             as the program would write the output file and run out of disk
**             space and just put alot of junk on the disk.
**
**  11-08-87  Ver 1.0    Added -h HEADER FILE, -* "Print All 'C' Files,
**                             -x Extra Lines and Program Version Number
**
**  01-10-88  Ver 1.1    Added call to setvbuf() to speed up file access.
**
**  01-25-88  Ver 1.2    Added -t  for option of just table of contents list
**
**  01-29-88  Ver 1.3    Fixed Bug.  funcname () would not be put in Table of
**                         Contents because of space between funcname and ().
**
**  04-26-88  Ver 1.4    Added -p* option for starting page number and changed
**                       Table of Contents so File Date and Time Shown.
**
**  05-12-88  Ver 3.0    Added -q* option to print files by section number
**                       rather than by page number.
**                       Added -t- option to suppress the Table of Contents
**
**  07-18-88  Ver 3.1    Fixed bug in printing by section where form feed was
**                       not being output at the top of the file.
**
**  (T. Hiscox)
**  03-25-89  Ver 3.2    Translation to Microsoft C Version 5.1 
**                       Sort by actual function name(not case sensitive)
**                       ignoring type cast, print aligned with function name.
**                       {No messages on Warning Level 3 !!} 
**
**  ( T. Hiscox)
**  09-01-89  Ver 3.3    Add Cross Reference of function calls between modules
**                       Printer Support for Epson and Hewlett Packard Deskjet
**
**  (D. Perkowski)
**  08-01-91  Ver 3.4	 Added support for OS/2, ANSI C decls and aligned
**								 braces.
*/

/* --------------------------------------------------------------------- */
/*
		 Include Files for Library Function Prototypes
*/
#ifdef OS2
#define INCL_DOSFILEMGR
#define INCL_ERRORS
#include <os2.h>
#endif
#include  <stdio.h>
#include  <stdlib.h>
#include  <fcntl.h>
#include  <malloc.h>
#include  <dos.h>
#include  <conio.h>
#include  <time.h>
#include  <string.h>
#include  <ctype.h>
#include  <io.h>

/* --------------------------------------------------------------------- */
/*
		 Some Valuable Definitions
*/
#define      O_RAW  O_BINARY                  /* Handle File as Binary       */

#define      TEMPORARY_FILENAME   "CPR$.TMP"  /* temporary file name (always)*/

#define      MAJOR_VERSION  3                 /* Current Software Version #   */
#define      MINOR_VERSION  4

#define      MAX_S          256      /* Maximum string length        */
#define      LENGTH         60       /* Default Page Length          */
#define      WIDTH          132      /* Default page width           */
#define      N_FILES        400      /* Maximum number of files      */
#define      TOC_LEN        1024     /* Max Table of Contents entries*/
#define      MAX_FNAME_SIZE 50       /* Max filename length in response file */
#define      MAX_HDR_BYTES  500      /* Max bytes in header file     */
#define      HEADER_SIZE    3        /* Header is 3 lines big        */
/* -------------------------------------------------------------------------
/* These Macros are used to determine valid characters for Table of Contents
*/

/* returns true if letter, digit, '_' , '*', or ' '      */
#define isidchr(c)      (isalnum(c) || (c == '*') || (c == '_') || (c == ' '))

/* returns true if letter, digit, '_', '*', '\t' or '\n' */
#define isiechr(c)      (isidchr(c) || c=='*' || c=='\t' || c=='\n')

/* returns true is letter, digit, '_', '*'               */  
#define isnamechr(c)    (isalnum(c) || (c == '*') || (c == '_') )

/* --------------------------------------------------------------------- */
/*
		 Internal Program Function Prototypes
*/
void  main(int argc,char * *argv);
void  Usage(void );
void  StartTempFile(void );
void  EndTempFile(void );
void  DumpTempFile(void );
int store_mod_call(struct cptr *modlist[],int m1,int m2);
int store_call(struct tnode *stree,int tindex);
int store_uses(struct tnode *stree,int tindex,int p);
int search_call(struct tnode *stree, int tindex, struct cptr **cp);
int search_use(struct tnode *stree, int tindex, struct cptr **cp);
void module_map(struct cptr *modlist[],char *title);
void  proc_map( int ( *sroutine )( struct tnode *proctree,
									int tproc, struct cptr **pc ), char *title );
void  DumpCrossReference(void);
void  Done(void );
void  List(void );
void  NewPage(void );
void  NewFile(void );
int   PutLine(char *l);
void  NewFunction(void );
void  BreakPage(void );
void  PutHeader(void );
int   keycomp( char *elem1, char *elem2);
void  write_xref(char *s);
int   LooksLikeFunction(char *s);
void   AddToTableOfContents(void );
void   DumpTableOfContents(void );
char   *strupper(char *str);
char   *root_name(char *ns);
void   SortTableOfContents(void );
char   *EndComment(char *p, int *flag);
char   *EndString(char *p,int *flag);
char   *substr1(char *string,int start,int end);
char   *expand(char *string);
char   *Rs(char *s);    
int    Cmemb(char a,char *b);
int    StarDot(char *file_string);
void   ReportMemoryError(void );
void   ReadHeaderFile(char *header_file);
int    ReadResponseFile(char *response_file);
int    c_break(void);
void   file_stats(char *filename);
int   printer_code_init(void);
void  normal_print(void);
void  emphasized_print_on(void);
void  emphasized_print_off(void);
void  print_double_width(void);
void  compressed_print(void);
void  new_print_page(void);
void  underline_on(void);
void  underline_off(void);
void  pspace(int ns);
char *strupper(char *str);
char *strsave(char *s);
void strfree(char *s);
char *strcompress( char *s);
int  CrossRefFunction( char *s);
int searchtree_index(struct tnode *stree,char *word,int tindex);
int searchtree_word(struct tnode *stree,char *word,int *tindex);
struct tnode *entertree(struct tnode *stree,char *word,int *lindex,int *tindex);
int FindFirst( char *filename, unsigned attr, struct find_t *finfo );
int FindNext( struct find_t *finfo );
int FindClose( void );
int GetDiskFree( unsigned drive, struct diskfree_t *disk_space );

/* -------------------------------------------------------------------- */
/*
		 Global Variables
*/
FILE  *TempFile;                 /* Program Temporary File       */
FILE  *File;                     /* Current File being worked on */
FILE  *Rfile;                    /* Response File handle         */
FILE  *Hfile;                    /* Header   File handle         */

struct find_t fblock;             /* struct used for calculating file size */
struct diskfree_t disk_space;     /* struct used for finding free disk space */

char  file_time_buff[20];      /* file time is placed here      */
char  file_date_buff[20];      /* file date is placed here      */

int    Braces;                 /* Keeps track of brace depth    */
int    LineNumber;             /* Count output lines           */
int    PageNumber = 1;         /* You figure this one out       */
int    SectionNumber = 1;      /* and this one to               */
int    PageLength = LENGTH;    /* Normal paper length           */
int    PageWidth  = WIDTH;     /* normal page width             */
int    OnePerPage = 0;         /* non-zero puts 1 function/page */
int    NumLines   = 0;         /* non-zero puts line # in file  */
int    Number     = 1;
int    WantSorted = 1;         /* Sort the table of contents   */
int    PageEnd;             /* Accounts for space at bottom */
int    InComment;           /* Indicates if inside Comment  */
int    InXComment;          /* Indicates if inside Comment (Xref) */
int    InString;            /* Indicates if inside String   */
int    InXString;          /*  Indicates in String for Xref Function */
int    double_strike;       /* # of times + 1 to write filename             */
int    header_flag;         /* non-zero enables print codes/Dates/Page #'s  */
int    header_lines;        /* number of extra lines from header file       */
int    extra_lines;         /* non-zero enables extra lines after functions */
int    table_contents;      /* non-zero indicates Table of Contents Only    */
int    sectionflg;          /* non-zero indicates print by section          */
int    nprinter = 1;        /* zero = Epson, 1 = Deskjet, 2 = ASCII(none)   */

unsigned long   memfree;    /* amount of local memory left for file buffer  */

char   *TempName;           /* pointer to temporary filename   */
char   *fname[N_FILES];     /* file names to be worked on      */
char   *STDIN = "\n";       /* special string pointer value to */
/* flag a file as being stdin      */
char   *Name;               /* Current file name               */
char   *ProgName;           /* This programs Path and Name     */

char   *Toc[TOC_LEN];       /* pointers to files in TOC        */
int TocPages[TOC_LEN];      /* page numbers                    */
int TocIndex[TOC_LEN];
int TocCount;               /* index into TOC arrays           */
char  *cproc;
int   nextf;                /* index into fname[]                */

char   header_buffer[MAX_HDR_BYTES]; /* 500 byte buffer for user defined Page Headers */
char   FunctionName[255];            /* Work area for parsing Func names */
char   Todayv[45];                   /* Today's Time and Date        */
/* --------------------------------------------------------------------- */
/*
	  The filenames[N_FILES][25] array allocates an array capable of holding
	 N_FILES filenames that are up to 25 characters long.

	  Each filename in DOS can be 12 characters long and the response file
	 has a CRLF with each line and we also need a terminating zero byte
	 bringing the count up to 15. MAX_FNAME_SIZE bytes are provided for a
	 possible path
*/
/* --------------------------------------------------------------------- */
/* char *filenames[N_FILES]; */

#ifdef OS2
static HDIR hDir;
static FILEFINDBUF FileInfo;
#endif

/* --------------------------------------------------------------------- */
/*
**  The following string is the definition of tab stops.  Every 'T' is a
**  tab, any other character is not.  It is currently defined as a tab
**  every 3 spaces.  The comment below helps if you want to modify this,
**  each row being 0+n, 50+n, 100+n, and 150+n, where n is the number
**  above the declaration.  Don't try to number each row with a comment,
**  because you'll notice that the '\'s make it one big string.
*/
char   *TabDef = "\
--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T\
--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T--T";
/* ------------------------------------------------------------------------ */
#define  PRINTERS 3

struct   esc_codes   {
	char  *id;
	char  *prt_init;
	char  *prt_compress;
	char  *prt_normal;
	char  *prt_double_width_on;
	char  *prt_double_width_off;
	char  *prt_emphasize_on;
	char  *prt_emphasize_off;
	char  *prt_new_page;
	char  *prt_underline_on;
	char  *prt_underline_off;
};

struct   esc_codes   print_code[PRINTERS] = {
	{"Epson FX","\x01B\x040","\x00F","\x012","\x00E","\x014","\x01B\x045","\x01b\x046","\x00C","\x01B\x02D\x000","\x01B\x02D\x001"},
	{"DeskJet ","\x01B(s16.67h0B\x01b&a10L",
		"\x0",
		"\x01B(s16.67h0B",
		"\x01b(s5h3B",
		"\x01B(s16.67h0B",
		"\x01b(s3B",
		"\x01b(s0B",
		"\x01B&l0H",
		"\x01b&d2D",
		"\x01b&d@"},
	{"ASCII Text","","","","","","","","\x0C","",""},
};

struct esc_codes  *printer;

typedef char ckey_type[10];
ckey_type ckeys[] =
  {"auto     ",
	"break    ",
	"case     ",
	"cdecl    ",
	"char     ",
	"const    ",
	"continue ",
	"default  ",
	"do       ",
	"double   ",
	"else     ",
	"enum     ",
	"extern   ",
	"far      ",
	"float    ",
	"for      ",
	"fortran  ",
	"goto     ",
	"huge     ",
	"if       ",
	"int      ",
	"interrupt",
	"long     ",
	"near     ",
	"pascal   ",
	"register ",
	"return   ",
	"short    ",
	"signed   ",
	"sizeof   ",
	"static	 ",
	"struct   ",
	"switch   ",
	"typedef  ",
	"union    ",
	"unsigned ",
	"void     ",
	"volatile ",
	"while    "};
#define  KEYSIZE  sizeof(ckeys) / sizeof(ckey_type)

/*------------------------------- tree types ------------------*/

struct cptr
{
	struct cptr  *next_ptr;
	int   proc;
};

struct entrytype
{
	char         *proc;
	int          module;
	unsigned int index;
	struct   cptr  *calls;
	struct   cptr  *uses;
};

struct tnode
{
	struct entrytype *this_entry;
	struct tnode     *left;
	struct tnode     *right;
};
struct   tnode *proctree;
struct   cptr *modcall[N_FILES];
struct   cptr *moduse[N_FILES];
int   module_index,proc_index,last_module_index,last_proc_index;
struct   call_type
{
	int   module;
	int   proc;
	char  *call;
};

#define  MAX_XREF 6144
struct call_type  call_list[MAX_XREF];
int   num_call;
/* ------------------------------------------------------------------------ */

/****************************************************************************/
void main( int argc, char **argv )
{
	unsigned long int total_disk_free = 0;
	unsigned int   ch;
	register int    i;
	long    thetime;

	nextf = 0;
	double_strike = 1;            /* default to no double strike       */
	header_flag   = 1;            /* default to codes and headers      */
	header_lines  = 0;            /* no extra lines from header file   */
	extra_lines   = 0;            /* no extra lines after functions    */
	cproc = NULL;
	last_proc_index = last_module_index = 0;
	num_call = 0;
	TocCount = 0;
	proctree=NULL;
	memset(modcall,'\0',sizeof(modcall) );
	memset(moduse,'\0',sizeof(moduse) );

	ProgName = strupr(argv[0]);   /* grab pointer to the programs name */

	time(&thetime);               /* grab the DOS time and date        */
	strcpy(Todayv,ctime(&thetime));  /* convert to ascii (LF at END OF STRING) */
	strtok(Todayv,"\n");          /* strip the annoying LF off         */

	if (argc == 1)                /*if only the program name given     */
		Usage();                   /* if no args, show instructions     */

	fprintf(stderr,"\nCPR - A C Source Code Printing and Cross Reference Utility\n");
	fprintf(stderr,"Version %d.%d  August 1991\n\n",
															MAJOR_VERSION, MINOR_VERSION );

	for (i = 1; i < argc; i++) 
	{  /* other parse command line          */
		if (argv[i][0] == '-')  
		{  /* check for switch flag "-"         */
			switch (toupper(argv[i][1])) 
			{ /* and go thru the switches   */

				case '@':               /* Response file */
					nextf = ReadResponseFile(argv[i]);
					break;

				case 'A':               /* "*.???" Print all specified files */
					nextf = StarDot(argv[i]);
					break;

				case 'C':               /* Print codes/Dates/Time and page # */
					header_flag = 0;
					break;

				case 'D':               /* Double Strike function names      */
					double_strike = 3;
					break;

				case 'H':               /* Header file                       */
					ReadHeaderFile(argv[i]);
					break;

				case 'L':                        /* Number of lines per page          */
					if ( ! argv[i][2]) 
					{                /* if -L ?? (ie.. a space)           */
						if (++i >= argc)                 /* see if another argument           */
							Usage();                      /* if no then print Help             */

						if ( ! isdigit (argv[i][0]))     /* is it a number?       */
							Usage();                      /* No! Print Help        */

						PageLength = atoi (argv[i]);     /* assign number         */
					}
					else 
					{      /* Switch came as -L?? form (with no space)      */
						if (!isdigit(argv[i][2]))        /* is it a number?       */
							Usage();             /* No! Print Help        */

						PageLength = atoi(&argv[i][2]);  /* assign number         */
					}
					break;

				case 'N':               /* Number Source Lines               */
					NumLines = 1;
					break;

				case 'O':               /* 1 function per page               */
					OnePerPage = 1;
					break;

				case 'P':               /* Starting Page Number              */
					if ( ! argv[i][2]) 
					{          /* if -P ?? (ie.. a space)           */
						if (++i >= argc)           /* see if another argument           */
							Usage();                /* if no then print Help             */
						if ( ! isdigit (argv[i][0]))  /* is it a number?       */
							Usage();                   /* No! Print Help        */
						PageNumber = atoi (argv[i]);  /* assign number         */
					}
					else 
					{      /* Switch came as -P?? form (with no space)      */
						if (!isdigit(argv[i][2]))     /* is it a number?       */
							Usage();                   /* No! Print Help        */
						PageNumber = atoi(&argv[i][2]);  /* assign number         */
					}
					break;

				case 'Q':               /* Print by Section                  */
					if ( ! argv[i][2]) 
					{             /* if -Q ?? (ie.. a space)           */
						if (++i >= argc)              /* see if another argument           */
							Usage();                      /* if no then print Help             */
						if ( ! isdigit (argv[i][0]))     /* is it a number?       */
							Usage();                      /* No! Print Help        */
						SectionNumber = atoi (argv[i]);  /* assign number         */
					}
					else 
					{      /* Switch came as -Q?? form (with no space)      */
						if (!isdigit(argv[i][2]))        /* is it a number?       */
							Usage();                      /* No! Print Help        */
						SectionNumber=atoi(&argv[i][2]); /* assign number         */
					}
					sectionflg = 1;
					break;

				case 'S':               /* Sorted Table of Contents          */
					WantSorted = 0;
					break;

				case 'T':               /* Print Table of Contents Only      */
					table_contents = 1;        /* enable flag                       */
					if( argv[i][2] == '-')     /* if disabling TOC                  */
						table_contents = -1;    /* disable it                        */
					break;

				case 'W':                                 /* Page Width                        */
					if ( ! argv[i][2]) 
					{             /* if -W ?? (ie.. a space)           */
						if (++i >= argc)              /* see if another argument           */
							Usage();                   /* if no then print Help             */
						if ( ! isdigit (argv[i][0]))  /* is it a number?       */
							Usage();                   /* No! Print Help        */
						PageWidth = atoi (argv[i]);   /* assign number         */
					}
					else 
					{                           /* Switch came as -W?? form (with no space)      */
						if (!isdigit(argv[i][2]))     /* is it a number?       */
							Usage();                   /* No! Print Help        */
						PageWidth = atoi(&argv[i][2]);   /* assign number         */
					}
					break;

				case 'X':               /* Extra Lines after Functions       */
					extra_lines = 1;
					break;

				case '\0':              /* End of string?(out of enviroment) */
					if (nextf >= N_FILES) 
					{
						fprintf (stderr, "%s: too many files\n", argv[0]);
						exit (1);
					}
					fname[nextf++] = STDIN;    /* any in excess goes to STDIN   */
					break;

				default:                /* No arguments given            */
					Usage();             /* show program instructions     */
					break;
			}
		}
		else 
		{      /* Argument (argv[i]) wasn't a switch parmeter           */
			if (nextf >= N_FILES) 
			{
				fprintf (stderr, "%s: too many files\n", argv[0]);
				exit (1);
			}
			fname[nextf++] = argv[i];  /* copy pointer to filename          */
		}
	}

	if ( ! nextf)           /* No files were specified                   */
		fname[nextf++] = STDIN; /* so use STDIN as the input file            */

	PageEnd = PageLength - (1 + PageLength / 20);

	StartTempFile();                    /* open temporary Output file               */

	printer_code_init();
	for (i = 0; i < nextf; i++) 
	{       /* walk thru file list               */
		if (fname[i] == STDIN) 
		{         /* if no files                       */
			File = stdin;                 /* assign stdin as input             */
			Name = "Standard Input";      /* assign name for printout          */
			fprintf(stderr,"Accepting Input from STDIN, ^Z to END\n");
		}
		else  
		{                          /* there was a filename              */
			if( (File = fopen( Name = fname[i], "r" )) == NULL )  
			{
				fprintf (stderr, "%s: Can't open file \"%s\"\n",
					ProgName, Name );
				continue;                  /* keep going if not found           */
			}
			else 
			{                        /* file opened OK so read            */
				fprintf(stderr, "\nReading File #%3d of %3d: \" %s \"",
					i+1, nextf, fname[i]);

				memfree = _memmax();              /* compute amount of free memory */
				memfree = ( (memfree > 32767L) ?  /* if to much memory then    */
					32767L  :          /* set max for setvbuf       */
					memfree );       /* else take as much as u can*/

				if( setvbuf(File, NULL, _IOFBF, (int) memfree) != 0) 
				{
					fprintf(stderr,"Input Buffer failed to allocate.\n");
				}
			}
		}
		if ( cproc ) free(cproc);
		cproc = NULL;
		module_index = i;
		List();              /* Read File in and process data             */

		if (File != stdin) 
		{ /* if it's not STDIN                         */
			fclose(File);     /* close file & deallocate buffer            */
			fprintf(stderr, " Table Entries: %4d  Cross Refs: %4d",TocCount,num_call);
		}
	}

	if( PageNumber > 1 || LineNumber > 0 )
		BreakPage();

	EndTempFile();          /* Close Temporary File                     */

	fprintf(stderr,"\n\n");

	GetDiskFree(0,&disk_space);
	total_disk_free = ( ( (long)disk_space.bytes_per_sector *   /* bytes per sector    */
		(long)disk_space.sectors_per_cluster) *   /* sectors per cluster */
		(long)disk_space.avail_clusters);      /* clusters available  */
	FindFirst(TEMPORARY_FILENAME,0,&fblock);
	if(fblock.size >= total_disk_free) 
	{   /* see if copy can be output */
		fprintf(stderr,"Insufficient Disk Space!\n\
                   \nMinimum of %lu bytes needed.\
                   \n%lu bytes are available\n\
                   \nDo You wish to continue? (Y/N)",
			fblock.size, total_disk_free);

		do 
		{
			ch =  toupper( getche() );
		}
		while((ch != 'Y') && (ch != 'N'));

		if(ch != 'Y')
			Done();              /* Delete Temporary File             */

		fprintf(stderr,"\n\n");
	}


	if((!table_contents) ||       /* if TOC and file enabled           */
		(table_contents == 1))     /* or TOC only     enabled           */
		DumpTableOfContents();     /* Print Table of Contents           */

	DumpCrossReference();

	if((!table_contents) ||       /* if TOC and file enabled           */
		(table_contents == -1))    /* or no TOC                         */
		DumpTempFile();            /* then dump the file                */


	Done();                    /* Clean things up and leave         */
}
/****************************************************************************/
void Usage( void )
{
	fprintf (stderr,"\nUsage: %s SWITCHES filename [...]", ProgName );
	fprintf (stderr,"\n----------------------------- VERSION %d.%d SWITCHES ------------------------\n",MAJOR_VERSION,MINOR_VERSION);
	fprintf (stderr,"           -@*    Replace * with Response Filename (1 filename per line    )\n");
	fprintf (stderr,"           -a*    Replace * with Drive Wildcard\n");
	fprintf (stderr,"           -c     Printer Codes/Dates/Page #'s off (default is Enabled     )\n");
	fprintf (stderr,"           -d     Double Strike Printing Enabled   (default is Disabled    )\n");
	fprintf (stderr,"           -h*    Replace * with Header Filename\n");
	fprintf (stderr,"           -l##   Replace ## with PageLength       (default = %3d          )\n",LENGTH);
	fprintf (stderr,"           -n     Numbers Source Lines             (Resets to 1 w/each file)\n");
	fprintf (stderr,"           -o     Places 1 function on a Page      (default is multiple    )\n");
	fprintf (stderr,"           -p*    Replace * with starting Page Number\n");
	fprintf (stderr,"           -q*    Replace * with starting Section Number\n");
	fprintf (stderr,"           -s     Sort Table of Contents by Page # (default is Func. Name  )\n");
	fprintf (stderr,"           -t     Print Table of Contents Only     (-t- suppress's Table   )\n");
	fprintf (stderr,"           -w##   Replace ## with Width            (default = %3d          )\n",WIDTH);
	fprintf (stderr,"           -x     Enables Extra Lines after Functions (default is Disabled )\n");
	fprintf (stderr,"          [...]   Means multiple filenames may be listed\n\n");
	fprintf (stderr,"               Response Files should have 1 filename per line up to %d      \n",MAX_FNAME_SIZE-1);
	fprintf (stderr,"         characters followed by 1 CRLF and may contain up to %d filenames.  \n\n",N_FILES);
	fprintf (stderr,"               Header Files contain the Strings to Print at the top of      \n");
	fprintf (stderr,"         every page of the listing, entire file can be up to %d bytes.",MAX_HDR_BYTES);
	exit(1);
}

/****************************************************************************/
void StartTempFile( void )
{
	TempName = TEMPORARY_FILENAME;         /* Temporary Filename */

	if( (TempFile = fopen(TempName, "w")) == NULL ) 
	{
		fprintf (stderr, "%s: Can't open temp file!\n", ProgName);
		exit(1);
	}
}

/****************************************************************************/
void EndTempFile( void )
{
	fclose (TempFile);
}

/****************************************************************************/
void DumpTempFile( void )
{
	int     fd,n;
	char    buff[1025];

	if ((fd = open (TempName, O_RDONLY | O_RAW)) == -1) 
	{
		fprintf (stderr, "%s: can't open temp file\n", ProgName);
		exit (1);
	}

	fprintf(TempFile,"%s",printer->prt_init);
	while ((n = read (fd, buff, 1024)) != 0) 
	{
		if (write (1, buff, n) == -1) 
		{
			fprintf (stderr, "%s: write error (1)\n", ProgName);
			exit (1);
		}
	}
}

/****************************************************************************/
/* Enter module list with m1 calling or using m2 */
int store_mod_call( struct cptr *modlist[], int m1, int m2 )
{
	struct cptr *pc,*cp;

	if( (pc = ( struct cptr *)malloc( sizeof(struct cptr) )) == NULL ) exit(0);
	pc->next_ptr = NULL;
	pc->proc = m2;
	if ( modlist[m1] ) 
	{
		cp = modlist[m1];
		do 
		{
			if ( cp->proc == m2 ) 
			{
				free(pc);
				return(1);
			}
			if ( cp->next_ptr )
				cp = cp->next_ptr;
			else
				break;
		} while ( cp );
		cp->next_ptr = pc;
	}
	else
		modlist[m1] = pc;
	return(1);
}

/****************************************************************************/
/* Enter tree with tindex, stores proc_index and module_index in Calls*/
int store_call( struct tnode *stree, int tindex )
{
	struct cptr *pc,*cp;

	if ( stree ) 
	{
		if (stree->this_entry->index == tindex) 
		{
			if ( (pc = ( struct cptr *)malloc( sizeof(struct cptr) )) == NULL ) exit(0);
			pc->next_ptr = NULL;
			pc->proc = proc_index;
			if ( stree->this_entry->calls ) 
			{
				cp = stree->this_entry->calls;
				do 
				{
					if ( cp->proc == proc_index ) 
					{
						free(pc);
						return(1);
					}
					if ( cp->next_ptr )
						cp = cp->next_ptr;
					else
						break;
				} while ( cp );
				cp->next_ptr = pc;
			}
			else stree->this_entry->calls = pc;
			return(1);
		}
		else 
		{
			if(store_call(stree->left,tindex) == 0)
				store_call(stree->right,tindex);
		}
	}
	return(0);
}

/****************************************************************************/
/* Enter tree with tindex, stores proc_index and module_index in Uses area*/
int store_uses( struct tnode *stree, int tindex, int p )
{
	struct cptr *pc,*cp;

	if ( stree ) 
	{
		if (stree->this_entry->index == tindex) 
		{
			if ( (pc = ( struct cptr *)malloc( sizeof(struct cptr) )) == NULL ) exit(0);
			pc->next_ptr = NULL;
			pc->proc = p;
			if ( stree->this_entry->uses ) 
			{
				cp = stree->this_entry->uses;
				do 
				{
					if ( cp->proc == p ) 
					{
						free(pc);
						return(1);
					}
					if ( cp->next_ptr )
						cp = cp->next_ptr;
					else
						break;
				} while ( cp );
				cp->next_ptr = pc;
			}
			else stree->this_entry->uses = pc;
			return(1);
		}
		else 
		{
			if(store_uses(stree->left,tindex,p) == 0)
				store_uses(stree->right,tindex,p);
		}
	}
	return(0);
}

/****************************************************************************/
/* Enter tree with tindex, returns ptr to call structure */
int search_call( struct tnode *stree, int tindex, struct cptr **pc )
{
	if ( stree ) 
	{
		if (stree->this_entry->index == tindex) 
		{
			*pc = stree->this_entry->calls;
			return(1);
		}
		else 
		{
			if( search_call(stree->left,tindex,pc) == 0)
				search_call(stree->right,tindex,pc);
		}
	}
	return(0);
}

/****************************************************************************/
/* Enter tree with tindex, returns ptr to uses structure */
int search_use( struct tnode *stree, int tindex, struct cptr **pc )
{
	if ( stree ) 
	{
		if (stree->this_entry->index == tindex) 
		{
			*pc = stree->this_entry->uses;
			return(1);
		}
		else 
		{
			if( search_use(stree->left,tindex,pc) == 0)
				search_use(stree->right,tindex,pc);
		}
	}
	return(0);
}

/****************************************************************************/
void module_map( struct cptr *modlist[], char *title )
{
	register int   i,j;
	struct cptr *pc,*cp;

	NewPage();
	underline_on();
	fprintf(TempFile,title);
	underline_off();

	cp = modlist[0];
	for ( i=0 ; i < nextf; i++) 
	{
		cp = modlist[i];
		if ( cp ) 
		{
			fprintf(TempFile,"          %-19.19s ",fname[i]);
			j = 0;
			pc = cp;
			while ( pc ) 
			{
				if ( j > 2 ) 
				{
					j = 0;
					fprintf(TempFile,"\n                              ");
					LineNumber++;
					if ( LineNumber + 5 > PageEnd ) 
					{
						NewPage();
						underline_on();
						fprintf(TempFile,title);
						underline_off();
						fprintf(TempFile,"\n                              ");
						LineNumber++;
						j = 0;
					}
				}
				fprintf(TempFile,"%-19.19s ",fname[pc->proc]);
				j++;
				pc = pc->next_ptr;
			}
			fprintf(TempFile,"\n\n");
			LineNumber++;
			LineNumber++;
		}
	}
}

/****************************************************************************/
void  proc_map( int ( *sroutine )( struct tnode *proctree,
									int tproc, struct cptr **pc ), char *title )
{
	char  ws[100];
	struct cptr *pc;
	register int   i,j;
	int   tproc;

	NewPage();
	underline_on();
	fprintf(TempFile,title);
	underline_off();

	i = 0;
	do 
	{
		tproc = TocIndex[i];
		if ( tproc ) 
		{
			searchtree_index(proctree,ws,tproc);
			fprintf(TempFile,"          %-19.19s ",ws);
			j = 0;
			sroutine(proctree,tproc,&pc);
			while ( pc ) 
			{
				searchtree_index(proctree,ws,pc->proc);
				if ( j > 2 ) 
				{
					j = 0;
					fprintf(TempFile,"\n                              ");
					LineNumber++;
					if ( LineNumber + 5 > PageEnd ) 
					{
						NewPage();
						underline_on();
						fprintf(TempFile,title);
						underline_off();
						fprintf(TempFile,"\n                              ");
						LineNumber++;
						j = 0;
					}
				}
				fprintf(TempFile,"%-19.19s ",ws);
				j++;
				pc = pc->next_ptr;
			}
			fprintf(TempFile,"\n");
			LineNumber++;
		}
		else 
		{
			printf("%s", Toc[i]);
			LineNumber += 2;
		}
		if ( LineNumber + 5 > PageEnd ) 
		{
			NewPage();
			underline_on();
			fprintf(TempFile,title);
			underline_off();
			j = 0;
		}
		i++;
	} while ( i < TocCount );
}

/*****************************************************************************/
void DumpCrossReference( void )
{
	int   i;

	PageNumber = -1; 
	LineNumber = 0; 
	TempFile = stdout;         /* default output to standard out     */

	fprintf(stderr, "\nLinking Cross References.....");

	/* strip out non-module calls (maybe use these later) */
	/* build calls and uses references */

	for ( i = 0; i < num_call ; i++) 
	{
		proc_index = 0;
		searchtree_word(proctree,call_list[i].call,&proc_index);
		if ( proc_index ) 
		{
			store_call(proctree,call_list[i].proc);
			store_uses(proctree,proc_index,call_list[i].proc);
			store_mod_call(modcall,call_list[i].module,module_index);
			store_mod_call(moduse,module_index,call_list[i].module);
		}
	}

	fprintf(TempFile,"%s",printer->prt_init);
	/* complete the call list */
	/* first, do the module to module maps */
	Name = "MODULE INTERACTION MAP";
	module_map(modcall,"          Module Name         Uses These Modules:\n\n");
	module_map(moduse,"          Module Name         Is Used by These Modules:\n\n");

	/* dump procedure cross references */
	/* will be in order to Table of Contents */

	Name = "PROCEDURE CROSS REFERENCE MAP";
	proc_map(search_call,"          Procedure           Uses These Procedures:\n\n");
	proc_map(search_use,"          Procedure Name      Is Used By These Procedures:\n\n");

	fflush(stdout);                     /* make sure everything is written out*/
	BreakPage();
}

/****************************************************************************/
void Done( void )
{
	if( TempName )       /* if you created a file  */
		unlink( TempName );  /* Delete temporary file  */
	exit(0);          /* and leave the hard way */
}

/****************************************************************************/
/* Process Incoming File */
void List( void )
{
	register int    bp;
	char            buffer[257];

	if( sectionflg ) 
	{                  /* if by section             */
		PageNumber = 1;                  /* Pages start at 1          */
		LineNumber = 0;                  /* Lines reset each time     */
		BreakPage();                     /* form feed if allowed      */
	}
	NewPage();                          /* bump page & output header */
	NewFile();
	bp = Braces = 0;                    /* no begin page and braces  */

	while( fgets(buffer, 256, File) != NULL ) 
	{  /* read a line upto 256 bytes*/
		Rs(buffer);                               /* Remove Trailing Spaces    */
		if( bp )                                  /* if beginning page         */
			NewFunction();                         /* setup for new function    */
		if( (LineNumber + 1) > PageEnd )          /* if going past page end   */
			NewPage();                               /* start a new page         */
		if(!Braces && LooksLikeFunction(buffer))    /* no braces and a function */
		{
			AddToTableOfContents();                   /* then add to TOC          */
		}
		else
			CrossRefFunction(buffer);

		bp = PutLine(buffer);
		LineNumber++;
	}
	if( sectionflg )                     /* if by sections           */
		SectionNumber ++;                 /* Bump to next Section     */
}
/****************************************************************************/
/* Process New Page */
void NewPage( void )
{
	if( LineNumber > HEADER_SIZE )  
	{
		if( PageNumber >= 0 ) 
		{
			PageNumber ++;             /* bump Page Number      */
		}
		BreakPage();                  /* form feed if allowed  */
		LineNumber = 0;               /* set flag for a header */
	}
	if( LineNumber == 0 )            /* Each new page         */
		PutHeader();                  /* Outputs a Header      */
}
/****************************************************************************/
void NewFile( void )
{
	register int    i, l;
	char            temp[80];

	if (TocCount >= TOC_LEN) 
	{
		fprintf (stderr, "%s: too many table of contents entries\n", ProgName);
		exit (1);
	}

	if( (Toc[TocCount] = (char *) malloc(130)) == NULL)   /* allocate some memory */
		ReportMemoryError();
	sprintf (Toc[TocCount], "\n\tModule: %s%s%s", printer->prt_double_width_on,Name,printer->prt_normal);
	l = strlen(Name)*3  + 6;                  /* and findout how long it is */

	if( l < 64 )  
	{                                 /* if not to long */
		i = (64 - l) / 8;                            /* calc. TAB increment */
		for( l=0; l < i; ++l )                       /* keep within 64 columns */
			strcat(Toc[TocCount],"\t");               /* and put some TAB's out */
	}

	file_stats(Name);                     

	if(! sectionflg ) 
	{
		sprintf (temp, "  Page %4d  (%s  %s)\n",     /* Page #, Time & date */
			PageNumber, file_date_buff, file_time_buff);
	}
	else 
	{
		sprintf (temp, "  Section %4d (%s  %s)\n",   /* Section, Time & Date*/
			SectionNumber, file_date_buff, file_time_buff);
	}

	strcat(Toc[TocCount], temp);              /* copy to main buffer */
	++ TocCount;                              /* bump the entries    */
	if (NumLines)                             /* if numbering lines  */
		Number  = 1;                           /* reset linenumber    */
	InComment = InString = InXComment = InXString = 0;     /* not in comment or string */
}

/****************************************************************************/
int PutLine( register char   *l )
{
	register char   c;
	int             bp;
	char            *save;
	char            *section, *p;
	int             offset;
	char            Digits[15];
	int             Size;
	int             pos;

	bp = 0;

	for( save = expand(l); ( (c = *l) != 0); ++l ) 
	{  /* bump thru the string */
		if( InComment )                                /* if your in a comment  */
			l = EndComment(l,&InComment);                          /* process till your out */
		else
			if( InString )                              /* if your in a string   */
				l = EndString(l,&InString);               /* process till your out */
			else
				switch(c)  
				{                             /* it must be something  */
					case '{':                             /* curly brace IN        */
						++Braces;                    /* increment brace count */
						break;
					case '}':                             /* curly brace OUT       */
						if( --Braces == 0)           /* coming out yet?       */
							if (*(l+1) == ';')        /* is it structure?      */
								++l;                   /* continue thru string  */
							else
								if(extra_lines)        /* if option enabled    */
									bp = 1;             /* flag for extra lines */
						break;
					case '\'':                            /* Forward Slash         */
						++l;                         /* continue thru string  */
						break;
					case '"':                             /* Quotation Mark        */
						InString = 1;                /* must be in a string   */
						break;
					case '/':                             /* Start of Comment      */
						if( *(l+1) == '*' ) 
						{        /* is next character * ? */
							InComment = 1;            /* must be in a comment  */
							++l;                      /* continue thru string  */
						}
						break;
				}
	}

	if (NumLines)  
	{                                  /* if line numbering enabled */
		sprintf (Digits,"[%4d]  ", Number);            /* make a string         */
		Size = strlen(Digits);                         /* calc. its size        */
	}
	else 
	{
		Size = 0;                                      /* else it's size is 0       */
	}

	if (strlen(save) +                                /* if original strings size  */
		Size  >                                 /* and the size of numbers   */
		PageWidth)  
	{                           /* are bigger than the width */
		section = substr1(save, 0, PageWidth - Size);
		if (section[strlen(section) - 1] != ' ')
			if (NULL == (p = strrchr(section, ' ')))
				offset = strlen(section);
			else
				offset = p - section;
		else
			offset = strlen(section) - 1;

		section[offset] = NULL;

		if (NumLines)  
		{                               /* if line numbering is enabled */
			fprintf (TempFile, "[%4d]  %s\n", Number++, section);
		}
		else  
		{
			fprintf (TempFile, "%s\n", section);
		}

		pos = offset + 1;
		do  
		{
			section = substr1(save, pos, pos + PageWidth - 8);
			if (strlen(save) - pos + 8 > PageWidth)
				if (section[strlen(section) - 1] != ' ')
					if (NULL == (p = strrchr(section, ' ')))
						offset = strlen(section);
					else
						offset = p - section;
				else
					offset = strlen (section) - 1;
			else
				offset = strlen(section);

			section[offset] = NULL;

			/** ----------------------------------------------------------------- **/
			/* Removed this for some reason
			/*
			/*           if (section[strlen(section) - 1] == '\n')
			/*              section[strlen(section) - 1] = NULL;
			*/

			/*  Line is to long for pagewidth so continue on next line */

			fprintf (TempFile, "C       %s\n", section);
			if (++LineNumber > PageEnd)
				NewPage();
		} while ((pos += offset + 1) < strlen(save));
	}
	else  
	{
		if (NumLines)     /* if line numbering enabled */
			fprintf (TempFile, "[%4d]  %s\n", Number++, save);
		else
			fprintf (TempFile, "%s\n", save);
	}

	return(bp);
}

/****************************************************************************/
/* Process New Function */
void NewFunction( void )              
{
	register int    i;

	if( LineNumber > (PageLength * 3 / 4) )       /*  49.5 lines            */
		NewPage();
	else  
	{
		if (!OnePerPage)  
		{              /* if mult. functions per page  */
			for( i=0; i < (PageLength/7); ++i )
				putc ('\n', TempFile);      /*   add extra lines           */
			LineNumber += PageLength/7; /*   set line counter also     */
		}
		else
			NewPage();              /* otherwise its 1 func. per page*/
	}
}

/****************************************************************************/
/* Process Page Break */
void BreakPage( void )        
{
	if(header_flag)            /* if its allowed send a FORM FEED           */
		new_print_page();
}

/****************************************************************************/
void PutHeader( void )
{
	register int    i, l;

	putc ('\n', TempFile);

	if(header_flag)  
	{ 
		print_double_width();
		l = strlen(Name) * 3;   
		fprintf (TempFile, "%s", Name);        /* Write out Filename or TOC */
		normal_print();
	}
	if( PageNumber > 0 ) 
	{                    /* if you're not on the TOC pages */
		for( i = (l+7)/8; i < 6; ++i )
			putc ('\t', TempFile);              /* Tab out for position         */

		if(! sectionflg ) 
		{
			fprintf (TempFile, "%s\tPage: %d\n\n\n", Todayv,PageNumber);
		}
		else 
		{
			fprintf (TempFile, "%s\tSection %3d - %d\n\n\n",
				Todayv,SectionNumber,PageNumber);
		}
		if(header_lines)                       /* Header for TOP OF PAGE */
			fprintf (TempFile, "%s", header_buffer);
	}
	else 
	{                                    /* 1st line of Table of Contents page gets the time and date */
		for( i = (l+7)/8; i < 6; ++i )         /* Tab out for time/date  */
			putc ('\t', TempFile);
		fprintf (TempFile, "\n%s\n", Todayv);  /* Todays Time and Date   */
		if(header_lines)                       /* Header for TOP OF PAGE */
			fprintf (TempFile, "%s", header_buffer);
	}
	LineNumber += (HEADER_SIZE + header_lines);     /* bump line number counter */
}

/****************************************************************************/
int keycomp( char *elem1, char *elem2 )
{
	int   i;

	i = strcmp(strcompress(elem1),strcompress(elem2) );
	return(i);
}

/****************************************************************************/
void write_xref( char *s )
{
	if ( cproc ) 
	{
		if ( strlen( s = strcompress(s) ) ) 
		{
			if ( bsearch(s,ckeys,KEYSIZE,sizeof(ckey_type), keycomp ) == NULL ) 
			{
				/* printf(xref,"%3d %3d %-20.20s\n",module_index,proc_index, s ); */
				if ( num_call < MAX_XREF-1 ) 
				{
					call_list[num_call].module = module_index;
					call_list[num_call].proc   = proc_index;
					call_list[num_call].call   = strsave(s);
					num_call++;
				}
			}
		}
	}
}

/****************************************************************************/
LooksLikeFunction( char *s )
{
	char            *p;
	char            *save;
	int             nosl,nolp,norp,flg;
	int             AddOne = 0;

	if( InComment || InString)    /* if you're in a comment or in a string */
		return(0);                 /* just leave and return 0             */

	p = FunctionName;             /* assign pointer to function name string */
	save = s;                     /* save address of string                 */

	nosl = nolp = norp = 0;       /* no left or right paren or no slash     */

	flg = 1;

	for(; *s && flg  ; s++)  
	{    /* go until end of string or flag is set  */
		switch  (*s)  
		{            /* switch on each character in string     */
			case  '(':              /* is it a left paren?                    */
				if (!nolp) 
				{         /* if no left paren already               */
					nolp = 1;         /* flag that you've got one               */

					/* 3/4/87 added following line to include ARGS in TOC */
					*p++ = *s;        /* move byte of ARG into function buffer */
				}
				else
					return(0);        /* or return if you've already had one    */
				break;

			case  ')':              /* is it a right paren?                   */
				if (nolp && !norp) 
				{ /* if already a left paren and no right yet */
					norp = 1;         /* flag that you've got a right paren     */

					/* 3/4/87 added following line to include ARGS in TOC */
					*p++ = *s;        /* move byte of ARG into function buffer  */
				}
				else
					return(0);        /* or return if conditions were'nt right  */
				break;

			default:                   /* everything comes thru here             */
				if (!nolp)  
				{           /* if no left paren yet                   */
					if (isiechr(*s))  
					{  /* is it alpha,digit,'_*\t\n'?        */
						if (isidchr(*s))  /* is it alpha,digit,'_' or ' '?      */
							*p++ = *s;     /* if it is store the byte for TOC    */
						else            
							p = FunctionName; /* start over if '*\t\n' & nolp */
						break;
					}
					else
						return(0);        /* return if conditions weren't right     */
				}

				if (!norp)  
				{                       /* if no right paren yet                  */
					if (isiechr(*s) || *s == ',') 
					{  /* is it alpha,digit,'_ *\t\n'?*/

						/* 3/4/87 added following line to include ARGS in TOC */
						*p++ = *s;        /* move byte of ARG in function buffer    */
						break;
					}
					else
						return(0);     /* return if conditions weren't right */
				}

				if (Cmemb(*s," \t\n\r") && !nosl)   /* is it ' \t\n\r' or no slash yet */
					break;

				if (*s == '/' && !nosl)  
				{ /* is it a '/' and no slash yet   */
					nosl = 1;            /* flag that you've got one       */
					break;
				}

				if (*s == '*' && nosl)  
				{  /* if its '*' & you got a slash already */
					flg = 0;                /* set flag to abort loop         */
					break;
				}
				return(0);                    /* return if conditions not right */
		}                                   /* end of switch */
	}                                      /* end of for loop */

	if (nolp != 1)                         /* return if no left paren found  */
		return(0);

	*p = '\0';                             /* else terminate function buffer */

	if (NumLines)  
	{                       /* if line numbering enabled      */

		/* the following code indents the Funtion Names with Line Numbering
			It is removed in favor of inserting a blank line */
		/***
			 sprintf (Digits,"[%4d]  ", Number);        
			Cnt = strlen(Digits) + AddOne;            
			 while (Cnt-- > 0)
				 putc (' ', TempFile);
			 AddOne = 0;
		***/
		putc('\n', TempFile);
		LineNumber++;
	}
	/*
	 * This will cause the function name part of the line to
	 * be double striken.
	*/
	if( double_strike == 3) 
	{
		while (*save && *save != '(')    /* double strike up to func  name */
			putc (*save++, TempFile);
		putc ('\r', TempFile);           /* use CR for doublestrike        */
	}
	return(1);
}
/****************************************************************************/
/* Enter tree with tindex, returns with alfa string corresponding */
int searchtree_index( struct tnode *stree, char *word, int tindex )
{
	if ( stree ) 
	{
		if (stree->this_entry->index == tindex) 
		{
			strcpy(word,stree->this_entry->proc);
			module_index = stree->this_entry->module;
			return(1);
		}
		else 
		{
			if(searchtree_index(stree->left,word,tindex) == 0)
				searchtree_index(stree->right,word,tindex);
		}
	}
	return(0);
}

/****************************************************************************/
/* enter with word, returns index */
int searchtree_word( struct tnode *stree, char *word, int *tindex )
{
	if( stree ) 
	{
		if (stricmp(stree->this_entry->proc,word ) == 0) 
		{
			proc_index = stree->this_entry->index;
			module_index = stree->this_entry->module; 
			*tindex = stree->this_entry->index;
			return(1);
		}
		else 
		{
			if(searchtree_word(stree->left,word,tindex) == 0)
				searchtree_word(stree->right,word,tindex);
		}
	}
	return(0);
}

/****************************************************************************/
struct tnode *entertree( struct tnode *stree, char *word,
														int *lindex, int *tindex )
{
	if(stree == NULL) 
	{
		stree = (struct tnode *)malloc(sizeof(struct tnode));
		stree->this_entry = (struct entrytype *)malloc(sizeof(struct entrytype));
		stree->left = NULL;
		stree->right = NULL;
		stree->this_entry->calls = NULL;
		stree->this_entry->uses = NULL;
		stree->this_entry->proc = strsave(word);
		stree->this_entry->module = module_index;
		(*lindex)++;
		stree->this_entry->index = *lindex;
		*tindex = *lindex;
	}
	else
		switch(strcmp(stree->this_entry->proc,word)) 
		{
			case 0:
				*tindex = stree->this_entry->index;
				break;
			case 1:
				stree->left = entertree(stree->left,word,lindex,tindex);
				break;
			default:
				stree->right = entertree(stree->right,word,lindex,tindex);
		}
	return(stree);
}

/****************************************************************************/
void AddToTableOfContents( void )
{
	register int    l;
	register char   *p;

	if (TocCount >= TOC_LEN) 
	{
		fprintf (stderr, "%s: too many table of contents entries\n", ProgName);
		exit (1);
	}
	l = strlen(FunctionName);             
	p = Toc[TocCount] = (char *) malloc(l+1);
	if( p == NULL) 
		ReportMemoryError();        
	strcpy(p, FunctionName);         
	TocPages[TocCount] = PageNumber;
	/* build a tree of procedure names and modules */
	if ( cproc ) free(cproc);
	cproc = strsave(root_name(FunctionName) );
	if ( p = strchr(cproc,'(') ) 
	{
		*p = ' ';
		while ( *p && *p != ')' ) *p++ = ' ';
		if ( *p == ')' ) *p = ' ';
	}
	cproc = strcompress(cproc);
	proctree = entertree(proctree,cproc,&last_proc_index,&proc_index);
	TocIndex[TocCount] = proc_index;
	TocCount++;
}

/****************************************************************************/
void DumpTableOfContents( void )
{
	register int    i, j, l;
	char  *rn;

	if( TocCount == 0 )              /* if there nothing to print */
		return;                       /* then just return          */

	if (WantSorted)                  /* if you wanted it sorted by page number */
		SortTableOfContents();        /* then we must sort it first         */

	Name = "TABLE of CONTENTS";      /* give me a title for the page       */

	PageNumber = -1;                 /* Table of Contents pages are not numbered */
	LineNumber = 0;                  /* and neither are the lines          */
	TempFile = stdout;               /* default output to standard out     */

	fprintf(TempFile,"%s",printer->prt_init);
	NewPage();                       /* start out creating a new page      */

	for( i=0; i < TocCount; ++i ) 
	{
		if( Toc[i][0] == '\n' ) 
		{
			if( (LineNumber + 5) > PageEnd )
				NewPage();
			printf("%s", Toc[i]);
			LineNumber += 2;
			continue;
		}
		if( ++LineNumber > PageEnd )     /* if going off end of page           */
			NewPage();                    /* start a new page                   */
		rn = root_name(Toc[i]);
		l = ( (rn - Toc[i]) < 20 ) ? 20-(rn-Toc[i]) : 0;
		for ( j = 0; j < l ; j++) putchar(' ');
		printf("%s ",Toc[i]);
		l += strlen(Toc[i]);             /* length if function name with parms */
		for( j=l; j < 68; ++j )          /* put dots out to column 68          */
			putchar('.');                 /* put dots out to page num. in TOC   */
		printf(" %d\n", TocPages[i]);    /* print page number for function */
	}
	fflush(stdout);                     /* make sure everything is written out*/
	BreakPage();
}

/*********************************************************/
/*    Search the Table of Contents Entry and return a    */
/*    pointer to the root name part of the entry (v3.2)  */
/*********************************************************/
char *root_name( char *ns )
{
	char *p;

	p = ns;
	do 
	{
		p++;
	} while ( ( *p != '(' ) && ( *p ) );
	if ( *p  ) 
	{
		do 
		{
			p--;
		} while ( ( !isnamechr(*p) ) && ( p > ns) );
		do 
		{
			p--;
		} while ( ( *p != ' ' ) && ( p > ns) );
	}
	if ( *p == ' ') p++;
	if ( *p == '*') p++;
	return( p );
}

/****************************************************************************/
/* Sort Table of Contents by Funtion Name */
void SortTableOfContents( void )
{
	register int    i, tempint;
	char         *tempchar;
	int          flag;

	fprintf(stderr, "\nSorting Table of Contents.....");
	do 
	{
		flag = 0;            /* default to already sorted */

		for (i = 0; i < TocCount - 1; i++) 
		{   /* look at them all */
			if (Toc[i][0] == '\n' || Toc[i+1][0] == '\n')
				continue;       /* don't sort across file names */

			if (stricmp (root_name(Toc[i]), root_name(Toc[i+1])) > 0) 
			{/* compare the strings        */
				tempchar = Toc[i];            /* copy to temp pointer       */
				Toc[i] = Toc[i+1];            /* swap the pointers in array */
				Toc[i+1] = tempchar;       /* put the temp pointer back  */
				tempint = TocPages[i];        /* grab the page number       */
				TocPages[i] = TocPages[i+1];  /* swap page numbers          */
				TocPages[i+1] = tempint;      /* put the temp page number back */
				tempint = TocIndex[i];
				TocIndex[i] = TocIndex[i+1];
				TocIndex[i+1] = tempint;
				flag = 1;                  /* indicate you've swapped    */
			}
		}
	}
	while (flag);        /* go until no more swaps can be made */
}
/****************************************************************************/
/* Process string until you come out of COMMENT */
char *EndComment( char *p, int *flag )
{
	char   c;

	while( (c = *p++) )          /* while there are chars to look at */
		if( c == '*' && *p == '/' ) 
		{  /* and there a splat or a slash     */
			*flag = 0;              /* say your not in the comment      */
			break;                      /* and leave the loop               */
		}
	return(p-1);                      /* returning the new pointer addr.  */
}

/****************************************************************************/
/* Process string until you come out of STRING  */
char *EndString( char *p, int *flag )
{
	register char   c;

	while( (c = *p++) != 0 )         /* while there are chars to look at */
		if( c == '\\' ) 
		{             /* and forward slashs for next line */
			continue;                  /* means "Just read on McDuck"      */
		}
		else if( c == '"' ) 
		{         /* if you found the ending quote    */
			*flag = 0;                  /* say your not in a string anymore */
			break;                     /* and leave the loop               */
		}
	return(p-1);                     /* returning the new pointer addr.  */
}
/****************************************************************************/
/*
 *      This is the function substr1().  The calling sequence is:
 *
 *                      substr1(string, startpos, endpos).
 *
 *      The function returns a pointer to a static string (written over -
 *      on subsequent calls) which is a substring of the string `string'
 *      starting at `startpos' (the first position is 0 (zero)) and ending
 *      at `endpos' (non-inclusive).  All arguments must be present or
 *      strange things happen with the system stack.
 *
 *      An example of the use is:
 *
 *              x = substr1(string, 2, 5);
 *              (where string == "This is a test.")
 *
 *      This call returns a pointer to:
 *              "is "
 *      An error code of -1 is returned is the `endpos' is greater than
 *      `startpos'
 *
 *                                              Lance E. Shepard
 */
char *substr1( char *string, int start, int end )
{
	static char  retstr[MAX_S];
	int loop1;
	int loop2;

	if (end < start)  
	{
		exit(-1);
	}

	for (loop2 = 0; loop2 < MAX_S; loop2++)
		retstr[loop2] = NULL;

	for (loop1 = start, loop2 = 0; string[loop1] != NULL &&
		loop1 < end && loop2 <= MAX_S; loop1++, loop2++)
		retstr[loop2] = string[loop1];

	retstr[++loop2] = NULL;
	return(retstr);
}

/****************************************************************************/
/*
 *      This is the function `char *expand().'  This function takes as
 *      an argument a NULL terminated string and replaces all occurances
 *      of the tab character with 8 (eight) spaces.  The function returns
 *      a pointer to a static string which is overwritten on subsequent
 *      calls.
*/
/****************************************************************************/
char *expand( char *string )
{
	int count;
	static char retstr[MAX_S];

	for (count = 0; count < MAX_S; retstr[count++] = NULL);

	for (count = 0; *string != NULL; count++, string++)  
	{
		if (*string == '\t')  
		{
			retstr[count] = ' ';
			/*        while (((count + 1) % 8) != 0)     */
			while (TabDef[count] != 'T')
				retstr[++count] = ' ';
		}
		else
			retstr[count] = *string;
	}
	retstr[count] = NULL;
	return(retstr);
}

/****************************************************************************/
/* strip trailing blanks from string  */
char *Rs( char s[] )
{
	int n;
	for (n=strlen(s)-1 ; n >= 0 && isspace(s[n]) ; n--)
		;           /* find the last space in the string */
	s[n+1] = '\0';    /* plop a null char on top of it     */
	return(s);        /* return pointer to string          */
}

/****************************************************************************/
/*  is character "a" a member of string "b"  */
int Cmemb( char a, char *b )
{
	while  (*b)          /* go until the null character */
		if (a == *b++)    /* looking for the character and bumping the pointer */
			return(1);     /* returning 1 if its found */
	return( (!a) ? 1 : 0 ); /* return 0 if "a" was a non-zero character */
}

/****************************************************************************/
int StarDot( char *file_string )
{
	unsigned long int total_file_size = 0;
	unsigned long int total_disk_free = 0;
	int done;
	int i = 0;

	file_string += 2;                       /* bump past -A switch */

	if( (done = FindFirst(file_string,0,&fblock)) != 0) 
	{/* Attempt 1st file read */
		fprintf(stderr,"No Files Found\n");
		exit(1);
	}
	while(!done && i < N_FILES) 
	{            /* go until done or to many files */
		/*filenames[i] = strsave(fblock.name);*/
		/*strcpy(&filenames[i][0],fblock.name);*/ /* copy names into array  */
		fname[i] = strsave(fblock.name);
		i++;
		total_file_size += fblock.size;       /* keep running total of filesizes */

		done = FindNext(&fblock);        /* read next availables file info  */
	}
	FindClose( );
	if( i == N_FILES)
		fprintf(stderr,"Printing ONLY 1st %d Files\n", N_FILES);

	GetDiskFree(0, &disk_space);            /* find free disk space */
	total_disk_free = (  ((long)disk_space.bytes_per_sector  *  /* convert to bytes */
		(long)disk_space.sectors_per_cluster) *
		(long)disk_space.avail_clusters);

	if((total_file_size * 2) >= total_disk_free) 
	{
		fprintf(stderr,"Insufficient Disk Space!\nMinimum of %lu bytes needed.\
                   \nYou Have %lu bytes free\n",
			total_file_size * 2, total_disk_free);
		exit(1);
	}
	return(i);        /* return new number of files */
}

/****************************************************************************/
void ReportMemoryError( void )
{
	fprintf(stderr,"Memory Allocation Error\n");

	if( TempName ) 
	{        /* if there is a file to work on */
		EndTempFile();       /* Make sure file is closed */
		unlink( TempName );     /* Delete temporary file */
	}
	exit(1);
}

/****************************************************************************/
void ReadHeaderFile( char *header_file )
{
	extern char    header_buffer[MAX_HDR_BYTES];
	extern int     header_lines;
	int ch;
	int i = 0;
	int x = 0;
	char *string;

	header_file += 2;              /* strip the "-h" from the string */
	if(( Hfile = fopen(header_file, "r")) == NULL) 
	{
		fprintf(stderr,"Can't Open Header File %s\n",header_file);
		exit(1);      
	}
	else 
	{
		/* read entire file that is up to MAX_HDR_BYTES characters long */
		while( (ch = fgetc(Hfile)) != EOF) 
		{
			header_buffer[i++] = (unsigned char) ch;
			if(i < MAX_HDR_BYTES)
				continue;
			else
				break;
		}
		header_buffer[i] = '\0';      /* terminate buffer */

		fclose(Hfile);                /* and close the file */

		for(x = 0, string = header_buffer; x <= i; x++) 
		{
			if(*string++ == '\n')      /* count number of line feeds */
				header_lines++;         /* tell the NewPage Function  */
		}  
	}
}

/****************************************************************************/
ReadResponseFile( char *response_file )
{
	char fpntr[80];            /* area for string to be read into */

	unsigned long int total_file_size = 0;
	unsigned long int total_disk_free = 0;

	int i = 0;

	response_file += 2;                  /* strip the "-@" from the string */
	if(( Rfile = fopen(response_file, "r")) == NULL) 
	{
		fprintf(stderr,"Can't Open Response File %s\n",response_file);
		exit(1);       /* terminate abruptly */
	}
	else 
	{
		/* read a entire line from the Rfile that is up to 80 characters long */
		while(fgets(fpntr,80,Rfile) != NULL && i < N_FILES) 
		{
			/*strtok(fpntr," \n");  */        /* strip to LF or space */
			fname[i] = strsave( strtok(fpntr," \n") );
			/*strcpy(&filenames[i][0],fpntr); */    /* copy names into array */
			/*fname[i] = filenames[i]; */     /* copy pointer to name */
			i++;

			FindFirst(fpntr,0,&fblock);       /* read file size in */

			total_file_size += fblock.size;  /* keep running total of filesizes */
		}
		fclose(Rfile);    /* and close the file */

		GetDiskFree(0, &disk_space);      /* find free disk space */
		total_disk_free = (  ((long)disk_space.bytes_per_sector  *  /* convert to bytes */
			(long)disk_space.sectors_per_cluster) *
			(long)disk_space.avail_clusters);

		if((total_file_size * 2) >= total_disk_free) 
		{
			fprintf(stderr,"Insufficient Disk Space!\nMinimum of %lu bytes needed.\
                      \nYou Have %lu bytes free\n",
				total_file_size * 2, total_disk_free);
			exit(1);
		}
		return(i);        /* return new number of files */
	}
	return(0);
}

/*****************************************************************************
	 This routine is executed when the CONTROL-BREAK key combination is hit
	 { Not in Version 3.2 with Microsoft v5.1 but left in for others}
*****************************************************************************/
int c_break( void )
{
	fprintf(stderr,"Control-Break hit.  Program aborting ...\n");

	if( TempName ) 
	{           /* if you created a file    */
		EndTempFile();          /* Make sure file is closed */
		unlink( TempName );     /* Delete temporary file    */
	}
	return(0);                 /* ABORT sequence           */
}

/****************************************************************************/
void file_stats( char *filename )
{
	extern struct find_t fblock;

	FindFirst(filename,0,&fblock);    /* read data into structure     */

	/* ------------------- Show date in standard format --------------------*/

	sprintf(file_date_buff,"%02d-%02d-%02d",
		((fblock.wr_date >> 5) & 0x0f),   /* month     */
		(fblock.wr_date & 0x1f)       ,   /* day       */
		((fblock.wr_date >> 9) + 80  ));  /* year      */

	/* -----------------   Show time in 12 Hour Format   ------------------- */
	if ( (fblock.wr_time >> 0x0b) <= 12)
	{
		sprintf(file_time_buff,"%02d:%02d",
			(fblock.wr_time >> 0x0b),
			((fblock.wr_time >> 5 ) & 0x3f ));  /* minutes   */
	}
	else
	{
		sprintf(file_time_buff,"%02d:%02d",
			((fblock.wr_time >> 0x0b) - 12),
			((fblock.wr_time >> 5 ) & 0x3f ));  /* minutes   */
	}
	/* ------------------     Decipher whether its AM or PM  ---------------- */
	if ( (fblock.wr_time >> 0x0b) < 12)
		strcat(file_time_buff," am"); /* for AM */
	else
		strcat(file_time_buff," pm"); /* for PM */
}

/****************************************************************************/
/* routine to initialize and select printer codes - called from initialize */
int printer_code_init( void )
{
	printer = &(print_code[nprinter]);
	return(1);
}

/****************************************************************************/
void normal_print( void )
{
	fprintf(TempFile,"%s",printer->prt_normal);
}

/****************************************************************************/
void emphasized_print_on( void )
{
	fprintf(TempFile,"%s",printer->prt_emphasize_on);
}

/****************************************************************************/
void emphasized_print_off( void )
{
	fprintf(TempFile,"%s",printer->prt_emphasize_off);
}

/****************************************************************************/
void print_double_width( void )
{
	fprintf(TempFile,"%s",printer->prt_double_width_on);
}

/****************************************************************************/
void compressed_print( void )
{
	fprintf(TempFile,"%s",printer->prt_compress);
}

/****************************************************************************/
void new_print_page( void )
{
	fprintf(TempFile,"%s",printer->prt_new_page);
}

/****************************************************************************/
void underline_on( void )
{
	fprintf(TempFile,"%s",printer->prt_underline_on);
}

/****************************************************************************/
void underline_off( void )
{
	fprintf(TempFile,"%s",printer->prt_underline_off);
}

/****************************************************************************/
void pspace( int ns )
{
	register int   i;

	for (i=0; i<ns; i++) fprintf(TempFile," ");
}

/****************************************************************************/
char *strupper( char *str )
{
	register int c;
	register char *s = str;

	while (c = *s)
		*s++ = (char)toupper(c);
	return(str);
}

/****************************************************************************/
char *strsave( char *s )
{
	char *p;

	if ((p=(char *)malloc(strlen(s)+1)) != NULL)
		strcpy(p,s);
	return(p);
}

/****************************************************************************/
void strfree( char *s )
{
	free( s );
}

/****************************************************************************/
/* utility to strip leading and trailing blanks */
/* from a null terminated string */
char *strcompress( char *str )
{
	register char *s;

	s = str + strlen(str);
	*s--;
	while ( *s == ' ' ) s--;
	*++s = '\0';
	while ( *str == ' ' ) str++;
	return(str);
}

/****************************************************************************/
int CrossRefFunction( char *s )
{
	char  *p,c;
	char  refname[100];


	p = refname;             /* assign pointer to function name string */
	for(; *s ; s++)  
	{
		if( InXComment )  s = EndComment(s,&InXComment);
		if( InXString ) s = EndString(s,&InXString);
		switch  (*s)  
		{
			case '\'':  while ( (c=*(++s) ) && c != '\'') ;
				if ( *(s+1) == '\'') s++;  /* special case */
				break;

			case '"':   InXString = 1;
				while( (c = *(++s) ) && c != '\"' ) ;
				if ( c == '\"') InXString = 0;
				break;
			case '/':
				if( *(s+1) == '*' ) 
				{
					InXComment = 1;
					s++;
					/***
					while( (c = *s++) ) {
						if( c == '*' && *s == '/' ) {
							InXComment = 0;
							break;
							}
						}
					s--;
					***/
				}
				break;
			case  '(':
				*p = '\0';
				write_xref(refname);
				p = refname;
				break;
			case '|':
			case '^':
			case '!':
			case '.':
			case '+':
			case '-':
			case '%':
			case '*':
			case '&':
			case '?':
			case '[':
			case ']':
			case '<':
			case '>':
			case ';':
			case ':':
			case '=':
			case ',':
			case ')':     /* start name over */
				p = refname;
				break;

			default:
				if (isiechr(*s))  
				{  /* is it alpha,digit,'_*\t\n'?        */
					if (isidchr(*s))  /* is it alpha,digit,'_' or ' '?      */
						*p++ = *s;     /* if it is store the byte for TOC    */
					else
						p = refname; /* start over if '*\t\n' & nolp */
					break;
				}
		}                                   /* end of switch */
	}                                      /* end of for loop */
	return(1);
}

int FindFirst( char *filename, unsigned attr, struct find_t *finfo )
{
#ifndef OS2
   return( _dos_findfirst( filename, attr, finfo ) );
#else
	USHORT      nCount  = 1 ;

   hDir = HDIR_CREATE;

	_doserrno = DosFindFirst ( filename, &hDir, attr ,&FileInfo,
                              sizeof(FileInfo), &nCount, 0L) ;
   if ( nCount == 1 )
   {
      strcpy( finfo->name, FileInfo.achName );
      finfo->attrib = FileInfo.attrFile;
      finfo->size   = FileInfo.cbFile;
		memcpy( &finfo->wr_date, &FileInfo.fdateLastWrite,
												sizeof( finfo->wr_date ) );
		memcpy( &finfo->wr_time, &FileInfo.ftimeLastWrite,
												sizeof( finfo->wr_time ) );
   }
	return( (nCount) ? 0 : -1 );
#endif
}

int FindNext( struct find_t *finfo )
{
#ifndef OS2
   return( _dos_findnext( finfo ) );
#else
   USHORT      nCount  = 1 ;

	_doserrno = DosFindNext(hDir, &FileInfo, sizeof(FileInfo), &nCount);
   if ( nCount == 1 )
   {
      strcpy( finfo->name, FileInfo.achName );
      finfo->attrib = FileInfo.attrFile;
      finfo->size   = FileInfo.cbFile;
		memcpy( &finfo->wr_date, &FileInfo.fdateLastWrite,
												sizeof( finfo->wr_date ) );
		memcpy( &finfo->wr_time, &FileInfo.ftimeLastWrite,
												sizeof( finfo->wr_time ) );
   }
	return( (nCount) ? 0 : -1 );
#endif
}

int FindClose( void )
{
#ifndef OS2
   return( 0 );
#else
	return( DosFindClose( hDir ) );
#endif
}

int GetDiskFree( unsigned drive, struct diskfree_t *disk_space )
{
	int	rc;

#ifndef OS2
	rc = _dos_getdiskfree( drive, disk_space );
#else
	USHORT		usDrv;
	FSALLOCATE	FileInfo;

	usDrv = drive & 0x1F;

	rc = DosQFSInfo( usDrv, 1, ( PBYTE )&FileInfo, sizeof( FileInfo ) );

	switch ( rc )
	{
		case NO_ERROR :
			disk_space->total_clusters = FileInfo.cUnit;
			disk_space->avail_clusters = FileInfo.cUnitAvail;
			disk_space->sectors_per_cluster = FileInfo.cSectorUnit;
			disk_space->bytes_per_sector = FileInfo.cbSector;
			rc = 0;
			break;
		default:
			rc = -1;        /* set default return code */
			break;
	}
#endif
	return( rc );
}

