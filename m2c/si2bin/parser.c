/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse         si2bin_yyparse
#define yylex           si2bin_yylex
#define yyerror         si2bin_yyerror
#define yylval          si2bin_yylval
#define yychar          si2bin_yychar
#define yydebug         si2bin_yydebug
#define yynerrs         si2bin_yynerrs


/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 20 "parser.y"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arch/southern-islands/asm/arg.h>
#include <arch/southern-islands/asm/bin-file.h>
#include <lib/util/debug.h>
#include <lib/util/elf-encode.h>
#include <lib/util/hash-table.h>
#include <lib/util/list.h>
#include <lib/util/string.h>

#include "arg.h"
#include "data.h"
#include "inner-bin.h"
#include "id.h"
#include "inst.h"
#include "metadata.h"
#include "outer-bin.h"
#include "si2bin.h"
#include "string.h"
#include "symbol.h"
#include "task.h"


#define YYERROR_VERBOSE



/* Line 268 of yacc.c  */
#line 112 "parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     TOK_SCALAR_REGISTER = 258,
     TOK_VECTOR_REGISTER = 259,
     TOK_SPECIAL_REGISTER = 260,
     TOK_MEMORY_REGISTER = 261,
     TOK_DECIMAL = 262,
     TOK_HEX = 263,
     TOK_FLOAT = 264,
     TOK_ID = 265,
     TOK_COMMA = 266,
     TOK_COLON = 267,
     TOK_FORMAT = 268,
     TOK_OFFEN = 269,
     TOK_IDXEN = 270,
     TOK_OFFSET = 271,
     TOK_OBRA = 272,
     TOK_CBRA = 273,
     TOK_OPAR = 274,
     TOK_CPAR = 275,
     TOK_NEW_LINE = 276,
     TOK_AMP = 277,
     TOK_ABS = 278,
     TOK_NEG = 279,
     TOK_STAR = 280,
     TOK_EQ = 281,
     TOK_UAV = 282,
     TOK_HL = 283,
     TOK_GLOBAL = 284,
     TOK_METADATA = 285,
     TOK_ARGS = 286,
     TOK_DATA = 287,
     TOK_TEXT = 288,
     TOK_CONST = 289,
     TOK_INT_DECL = 290,
     TOK_SHORT_DECL = 291,
     TOK_FLOAT_DECL = 292,
     TOK_HALF_DECL = 293,
     TOK_WORD_DECL = 294,
     TOK_BYTE_DECL = 295
   };
#endif
/* Tokens.  */
#define TOK_SCALAR_REGISTER 258
#define TOK_VECTOR_REGISTER 259
#define TOK_SPECIAL_REGISTER 260
#define TOK_MEMORY_REGISTER 261
#define TOK_DECIMAL 262
#define TOK_HEX 263
#define TOK_FLOAT 264
#define TOK_ID 265
#define TOK_COMMA 266
#define TOK_COLON 267
#define TOK_FORMAT 268
#define TOK_OFFEN 269
#define TOK_IDXEN 270
#define TOK_OFFSET 271
#define TOK_OBRA 272
#define TOK_CBRA 273
#define TOK_OPAR 274
#define TOK_CPAR 275
#define TOK_NEW_LINE 276
#define TOK_AMP 277
#define TOK_ABS 278
#define TOK_NEG 279
#define TOK_STAR 280
#define TOK_EQ 281
#define TOK_UAV 282
#define TOK_HL 283
#define TOK_GLOBAL 284
#define TOK_METADATA 285
#define TOK_ARGS 286
#define TOK_DATA 287
#define TOK_TEXT 288
#define TOK_CONST 289
#define TOK_INT_DECL 290
#define TOK_SHORT_DECL 291
#define TOK_FLOAT_DECL 292
#define TOK_HALF_DECL 293
#define TOK_WORD_DECL 294
#define TOK_BYTE_DECL 295




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 53 "parser.y"

	int num;
	float num_float;
	struct si2bin_id_t *id;
	struct si2bin_inst_t *inst;
	struct si_label_t *label;
	struct list_t *list;
	struct si2bin_arg_t *arg;
	struct si_arg_t *si_arg;



/* Line 293 of yacc.c  */
#line 241 "parser.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 253 "parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  10
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   180

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  41
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  56
/* YYNRULES -- Number of rules.  */
#define YYNRULES  129
/* YYNRULES -- Number of states.  */
#define YYNSTATES  206

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   295

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     8,    10,    13,    16,    18,    21,
      23,    25,    27,    29,    31,    34,    35,    40,    42,    45,
      48,    50,    53,    54,    60,    61,    79,    80,    88,    90,
      92,    94,    96,    99,   102,   104,   107,   110,   114,   117,
     120,   123,   126,   129,   132,   133,   136,   140,   141,   146,
     147,   150,   154,   155,   160,   161,   164,   168,   169,   174,
     175,   178,   182,   183,   188,   189,   192,   196,   197,   202,
     203,   206,   210,   211,   216,   218,   221,   224,   226,   229,
     230,   237,   238,   248,   249,   257,   258,   269,   271,   272,
     275,   276,   279,   282,   285,   288,   290,   293,   296,   298,
     301,   304,   305,   309,   311,   314,   317,   318,   320,   324,
     326,   328,   330,   332,   334,   336,   338,   340,   347,   352,
     355,   365,   367,   369,   370,   373,   376,   381,   383,   387
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      42,     0,    -1,    43,    -1,    21,    43,    -1,    44,    -1,
      44,    43,    -1,    47,    45,    -1,    46,    -1,    46,    45,
      -1,    50,    -1,    58,    -1,    74,    -1,    84,    -1,    48,
      -1,    48,    21,    -1,    -1,    29,    10,    49,    21,    -1,
      51,    -1,    51,    52,    -1,    30,    21,    -1,    53,    -1,
      53,    52,    -1,    -1,    10,    26,    57,    54,    21,    -1,
      -1,    10,    17,     7,    18,    26,    10,    11,     7,    11,
      10,    17,     7,    12,     7,    18,    55,    21,    -1,    -1,
      10,    12,    10,    26,    57,    56,    21,    -1,    21,    -1,
       7,    -1,     8,    -1,    59,    -1,    59,    60,    -1,    32,
      21,    -1,    61,    -1,    61,    60,    -1,    10,    12,    -1,
      10,    12,    21,    -1,    35,    62,    -1,    36,    64,    -1,
      37,    66,    -1,    39,    68,    -1,    38,    70,    -1,    40,
      72,    -1,    -1,    62,    21,    -1,    62,     7,    11,    -1,
      -1,    62,     7,    63,    21,    -1,    -1,    64,    21,    -1,
      64,     7,    11,    -1,    -1,    64,     7,    65,    21,    -1,
      -1,    66,    21,    -1,    66,     9,    11,    -1,    -1,    66,
       9,    67,    21,    -1,    -1,    68,    21,    -1,    68,    57,
      11,    -1,    -1,    68,    57,    69,    21,    -1,    -1,    70,
      21,    -1,    70,    57,    11,    -1,    -1,    70,    57,    71,
      21,    -1,    -1,    72,    21,    -1,    72,    57,    11,    -1,
      -1,    72,    57,    73,    21,    -1,    75,    -1,    75,    76,
      -1,    31,    21,    -1,    77,    -1,    77,    76,    -1,    -1,
      10,    10,     7,    82,    78,    21,    -1,    -1,    10,    17,
       7,    18,    10,     7,    82,    79,    21,    -1,    -1,    10,
      25,    10,     7,    83,    80,    21,    -1,    -1,    10,    17,
       7,    18,    25,    10,     7,    83,    81,    21,    -1,    21,
      -1,    -1,    82,    34,    -1,    -1,    83,    10,    -1,    83,
      27,    -1,    83,    28,    -1,    83,    34,    -1,    85,    -1,
      85,    86,    -1,    33,    21,    -1,    87,    -1,    87,    86,
      -1,    89,    21,    -1,    -1,    90,    88,    21,    -1,    21,
      -1,    10,    12,    -1,    10,    91,    -1,    -1,    93,    -1,
      93,    11,    91,    -1,     3,    -1,     4,    -1,     5,    -1,
       6,    -1,     7,    -1,     8,    -1,     9,    -1,    92,    -1,
      10,    17,     7,    12,     7,    18,    -1,    23,    19,    93,
      20,    -1,    24,    93,    -1,    92,    94,    13,    12,    17,
      10,    11,    10,    18,    -1,    10,    -1,    95,    -1,    -1,
      94,    14,    -1,    94,    15,    -1,    94,    16,    12,     7,
      -1,    96,    -1,    96,    22,    95,    -1,    10,    19,     7,
      20,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   121,   121,   122,   126,   127,   131,   135,   136,   140,
     141,   142,   143,   159,   160,   165,   164,   187,   188,   192,
     196,   197,   202,   201,   257,   256,   289,   288,   347,   351,
     355,   368,   369,   373,   377,   378,   382,   386,   390,   391,
     392,   393,   394,   395,   398,   400,   401,   412,   411,   423,
     425,   426,   437,   436,   448,   450,   451,   462,   461,   473,
     475,   476,   487,   486,   498,   500,   501,   512,   511,   523,
     525,   526,   537,   536,   549,   550,   554,   558,   559,   565,
     564,   588,   587,   610,   609,   632,   631,   654,   659,   667,
     681,   692,   707,   720,   731,   744,   745,   749,   753,   754,
     758,   760,   759,   771,   775,   802,   823,   827,   841,   854,
     867,   880,   888,   894,   899,   908,   915,   920,   965,   989,
    1013,  1054,  1070,  1079,  1083,  1094,  1105,  1118,  1120,  1144
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "TOK_SCALAR_REGISTER",
  "TOK_VECTOR_REGISTER", "TOK_SPECIAL_REGISTER", "TOK_MEMORY_REGISTER",
  "TOK_DECIMAL", "TOK_HEX", "TOK_FLOAT", "TOK_ID", "TOK_COMMA",
  "TOK_COLON", "TOK_FORMAT", "TOK_OFFEN", "TOK_IDXEN", "TOK_OFFSET",
  "TOK_OBRA", "TOK_CBRA", "TOK_OPAR", "TOK_CPAR", "TOK_NEW_LINE",
  "TOK_AMP", "TOK_ABS", "TOK_NEG", "TOK_STAR", "TOK_EQ", "TOK_UAV",
  "TOK_HL", "TOK_GLOBAL", "TOK_METADATA", "TOK_ARGS", "TOK_DATA",
  "TOK_TEXT", "TOK_CONST", "TOK_INT_DECL", "TOK_SHORT_DECL",
  "TOK_FLOAT_DECL", "TOK_HALF_DECL", "TOK_WORD_DECL", "TOK_BYTE_DECL",
  "$accept", "program", "kernel_list", "kernel", "section_list", "section",
  "global_section", "global_header", "$@1", "metadata_section",
  "metadata_header", "metadata_stmt_list", "metadata_stmt", "$@2", "$@3",
  "$@4", "hex_or_dec_value", "data_section", "data_header",
  "data_stmt_list", "data_stmt", "int_vals", "$@5", "short_vals", "$@6",
  "float_vals", "$@7", "word_vals", "$@8", "half_vals", "$@9", "byte_vals",
  "$@10", "args_section", "args_header", "args_stmt_list", "args_stmt",
  "$@11", "$@12", "$@13", "$@14", "val_stmt_list", "ptr_stmt_list",
  "text_section", "text_header", "text_stmt_list", "text_stmt", "$@15",
  "label", "instr", "arg_list", "operand", "arg", "maddr_qual",
  "waitcnt_arg", "waitcnt_elem", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    41,    42,    42,    43,    43,    44,    45,    45,    46,
      46,    46,    46,    47,    47,    49,    48,    50,    50,    51,
      52,    52,    54,    53,    55,    53,    56,    53,    53,    57,
      57,    58,    58,    59,    60,    60,    61,    61,    61,    61,
      61,    61,    61,    61,    62,    62,    62,    63,    62,    64,
      64,    64,    65,    64,    66,    66,    66,    67,    66,    68,
      68,    68,    69,    68,    70,    70,    70,    71,    70,    72,
      72,    72,    73,    72,    74,    74,    75,    76,    76,    78,
      77,    79,    77,    80,    77,    81,    77,    77,    82,    82,
      83,    83,    83,    83,    83,    84,    84,    85,    86,    86,
      87,    88,    87,    87,    89,    90,    91,    91,    91,    92,
      92,    92,    92,    92,    92,    92,    93,    93,    93,    93,
      93,    93,    93,    94,    94,    94,    94,    95,    95,    96
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     1,     2,     2,     1,     2,     1,
       1,     1,     1,     1,     2,     0,     4,     1,     2,     2,
       1,     2,     0,     5,     0,    17,     0,     7,     1,     1,
       1,     1,     2,     2,     1,     2,     2,     3,     2,     2,
       2,     2,     2,     2,     0,     2,     3,     0,     4,     0,
       2,     3,     0,     4,     0,     2,     3,     0,     4,     0,
       2,     3,     0,     4,     0,     2,     3,     0,     4,     0,
       2,     3,     0,     4,     1,     2,     2,     1,     2,     0,
       6,     0,     9,     0,     7,     0,    10,     1,     0,     2,
       0,     2,     2,     2,     2,     1,     2,     2,     1,     2,
       2,     0,     3,     1,     2,     2,     0,     1,     3,     1,
       1,     1,     1,     1,     1,     1,     1,     6,     4,     2,
       9,     1,     1,     0,     2,     2,     4,     1,     3,     4
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     2,     4,     0,    13,     3,    15,
       1,     5,     0,     0,     0,     0,     6,     7,     9,    17,
      10,    31,    11,    74,    12,    95,    14,     0,    19,    76,
      33,    97,     8,     0,    28,    18,    20,     0,    44,    49,
      54,    64,    59,    69,    32,    34,     0,    87,    75,    77,
     106,   103,    96,    98,     0,   101,    16,     0,     0,     0,
      21,    36,    38,    39,    40,    42,    41,    43,    35,     0,
       0,     0,    78,   109,   110,   111,   112,   113,   114,   115,
     121,   104,     0,     0,   105,   123,   107,   122,   127,    99,
     100,     0,     0,     0,    29,    30,    22,    37,    47,    45,
      52,    50,    57,    55,    65,    67,    60,    62,    70,    72,
      88,     0,     0,     0,     0,     0,   119,     0,   106,     0,
     102,     0,     0,     0,    46,     0,    51,     0,    56,     0,
      66,     0,    61,     0,    71,     0,    79,     0,    90,     0,
       0,     0,     0,   124,   125,     0,   108,     0,   128,    26,
       0,    23,    48,    53,    58,    68,    63,    73,    89,     0,
       0,     0,    83,     0,   129,   118,     0,     0,     0,     0,
      80,    88,     0,    91,    92,    93,    94,     0,     0,     0,
     126,    27,     0,    81,    90,    84,   117,     0,     0,     0,
      85,     0,     0,    82,     0,     0,     0,    86,   120,     0,
       0,     0,     0,    24,     0,    25
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,     5,    16,    17,     6,     7,    27,    18,
      19,    35,    36,   123,   204,   168,    96,    20,    21,    44,
      45,    62,   125,    63,   127,    64,   129,    66,   133,    65,
     131,    67,   135,    22,    23,    48,    49,   159,   189,   177,
     194,   136,   162,    24,    25,    52,    53,    91,    54,    55,
      84,    85,    86,   117,    87,    88
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -81
static const yytype_int16 yypact[] =
{
      -5,   -11,    12,    63,   -81,   -11,    54,    55,   -81,   -81,
     -81,   -81,    62,    71,    72,    73,   -81,    54,   -81,    52,
     -81,    -6,   -81,    57,   -81,    59,   -81,    76,   -81,   -81,
     -81,   -81,   -81,    40,   -81,   -81,    52,    67,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,    -6,     3,   -81,   -81,    57,
       2,   -81,   -81,    59,    77,   -81,   -81,    85,    89,    74,
     -81,    78,    47,    53,     6,    32,    34,    51,   -81,    93,
      94,    92,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
      58,   -81,    84,    41,   -81,    50,    95,   -81,    82,   -81,
     -81,    86,    79,    90,   -81,   -81,   -81,   -81,    98,   -81,
      99,   -81,   100,   -81,   -81,   101,   -81,   102,   -81,   103,
     -81,    97,   109,   110,   111,    41,   -81,    75,    41,   112,
     -81,    74,   104,   105,   -81,   106,   -81,   107,   -81,   108,
     -81,   113,   -81,   114,   -81,   115,    87,    13,   -81,   119,
     117,   118,   120,   -81,   -81,   121,   -81,   122,   -81,   -81,
     129,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   123,
     116,   130,     9,   135,   -81,   -81,   126,   138,   125,   136,
     -81,   -81,   141,   -81,   -81,   -81,   -81,   128,   132,   142,
     -81,   -81,   144,    87,   -81,   -81,   -81,   143,   145,   134,
       9,   147,   148,   -81,   139,   146,   149,   -81,   -81,   152,
     150,   154,   151,   -81,   153,   -81
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -81,   -81,    16,   -81,   155,   -81,   -81,   -81,   -81,   -81,
     -81,    83,   -81,   -81,   -81,   -81,   -65,   -81,   -81,    80,
     -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,   -81,
     -81,   -81,   -81,   -81,   -81,   124,   -81,   -81,   -81,   -81,
     -81,   -51,   -60,   -81,   -81,   127,   -81,   -81,   -81,   -81,
      35,   -81,   -80,   -81,    44,   -81
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -117
static const yytype_int16 yytable[] =
{
     105,   107,   109,   116,    37,    73,    74,    75,    76,    77,
      78,    79,    80,    69,    81,   102,     1,     8,     2,   173,
      70,    11,     9,   160,     2,    82,    83,   103,    71,    38,
      39,    40,    41,    42,    43,   141,   174,   175,   161,    94,
      95,    94,    95,   176,    73,    74,    75,    76,    77,    78,
      79,    80,    57,   104,    98,   106,   149,    58,    94,    95,
     100,  -116,    33,    10,    82,    83,    59,    46,    99,    50,
    -116,  -116,   108,    34,   101,   113,    26,   114,    47,    61,
      51,    94,    95,    28,    12,    13,    14,    15,   142,   143,
     144,   145,    29,    30,    31,    92,    93,    56,    90,    97,
     110,   111,   112,   115,   119,   121,   118,   120,   122,   124,
     126,   128,   130,   132,   134,   137,   138,   139,   140,    60,
     183,   158,   147,   171,   190,    68,   151,   152,   153,   154,
     150,   163,   166,   167,   155,   156,   157,   164,   165,   169,
     172,   114,   178,   179,   170,   180,   181,   182,   184,   185,
     186,   188,   187,   146,   191,   193,   192,   195,   196,   200,
     197,   202,   201,   148,   198,     0,   199,     0,     0,   203,
       0,     0,    32,    72,   205,     0,     0,     0,     0,     0,
      89
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-81))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      65,    66,    67,    83,    10,     3,     4,     5,     6,     7,
       8,     9,    10,    10,    12,     9,    21,     1,    29,    10,
      17,     5,    10,    10,    29,    23,    24,    21,    25,    35,
      36,    37,    38,    39,    40,   115,    27,    28,    25,     7,
       8,     7,     8,    34,     3,     4,     5,     6,     7,     8,
       9,    10,    12,    21,     7,    21,   121,    17,     7,     8,
       7,    11,    10,     0,    23,    24,    26,    10,    21,    10,
      20,    21,    21,    21,    21,    17,    21,    19,    21,    12,
      21,     7,     8,    21,    30,    31,    32,    33,    13,    14,
      15,    16,    21,    21,    21,    10,     7,    21,    21,    21,
       7,     7,    10,    19,    22,    26,    11,    21,    18,    11,
      11,    11,    11,    11,    11,    18,     7,     7,     7,    36,
     171,    34,    10,     7,   184,    45,    21,    21,    21,    21,
      26,    12,    12,    12,    21,    21,    21,    20,    20,    10,
      10,    19,     7,    17,    21,     7,    21,    11,     7,    21,
      18,     7,    10,   118,    11,    21,    11,    10,    10,     7,
      21,     7,    12,   119,    18,    -1,    17,    -1,    -1,    18,
      -1,    -1,    17,    49,    21,    -1,    -1,    -1,    -1,    -1,
      53
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    21,    29,    42,    43,    44,    47,    48,    43,    10,
       0,    43,    30,    31,    32,    33,    45,    46,    50,    51,
      58,    59,    74,    75,    84,    85,    21,    49,    21,    21,
      21,    21,    45,    10,    21,    52,    53,    10,    35,    36,
      37,    38,    39,    40,    60,    61,    10,    21,    76,    77,
      10,    21,    86,    87,    89,    90,    21,    12,    17,    26,
      52,    12,    62,    64,    66,    70,    68,    72,    60,    10,
      17,    25,    76,     3,     4,     5,     6,     7,     8,     9,
      10,    12,    23,    24,    91,    92,    93,    95,    96,    86,
      21,    88,    10,     7,     7,     8,    57,    21,     7,    21,
       7,    21,     9,    21,    21,    57,    21,    57,    21,    57,
       7,     7,    10,    17,    19,    19,    93,    94,    11,    22,
      21,    26,    18,    54,    11,    63,    11,    65,    11,    67,
      11,    71,    11,    69,    11,    73,    82,    18,     7,     7,
       7,    93,    13,    14,    15,    16,    91,    10,    95,    57,
      26,    21,    21,    21,    21,    21,    21,    21,    34,    78,
      10,    25,    83,    12,    20,    20,    12,    12,    56,    10,
      21,     7,    10,    10,    27,    28,    34,    80,     7,    17,
       7,    21,    11,    82,     7,    21,    18,    10,     7,    79,
      83,    11,    11,    21,    81,    10,    10,    21,    18,    17,
       7,    12,     7,    18,    55,    21
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 12:

/* Line 1806 of yacc.c  */
#line 144 "parser.y"
    {
		/* Process any tasks still left */
		si2bin_task_list_process();
		
		/* Clean up tasks and symbol table when finished parsing kernel */
		si2bin_task_list_done();
		si2bin_symbol_table_done();

		/* Set up new tasks and symbol table for next kernel */
		si2bin_task_list_init();
		si2bin_symbol_table_init();
	}
    break;

  case 15:

/* Line 1806 of yacc.c  */
#line 165 "parser.y"
    {
		/* Create new objects for each kernel */
		si2bin_inner_bin = si2bin_inner_bin_create((yyvsp[(2) - (2)].id)->name);
		si2bin_metadata = si2bin_metadata_create();
		si2bin_entry = si2bin_inner_bin_entry_create();
		
		/* Add objects and values to over all elf (outer_bin) */
		si2bin_metadata->uniqueid = si2bin_uniqueid;
		si2bin_inner_bin_add_entry(si2bin_inner_bin, si2bin_entry);
		si2bin_outer_bin_add(si2bin_outer_bin, si2bin_inner_bin, si2bin_metadata);

		/* Increment id */
		si2bin_uniqueid ++;

		/* Free id */
		si2bin_id_free((yyvsp[(2) - (2)].id));
	}
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 202 "parser.y"
    {
		/* Find memory information and store it in metadata */
		if (!strcmp("uavprivate", (yyvsp[(1) - (3)].id)->name))
		{
			si2bin_metadata->uavprivate = (yyvsp[(3) - (3)].num);
		}
		else if (!strcmp("hwregion", (yyvsp[(1) - (3)].id)->name))
		{	
			si2bin_metadata->hwregion = (yyvsp[(3) - (3)].num);
		}
		else if (!strcmp("hwlocal", (yyvsp[(1) - (3)].id)->name))
		{
			si2bin_metadata->hwlocal = (yyvsp[(3) - (3)].num);
		}
		else if (!strcmp("userElementCount", (yyvsp[(1) - (3)].id)->name))
		{
			warning("User has provided 'userElementCount' but this number is automatically calculated");
		}
		else if (!strcmp("NumVgprs", (yyvsp[(1) - (3)].id)->name))
		{	
			warning("User has provided 'NumVgprs' but this number is automatically calculated");
		}
		else if (!strcmp("NumSgprs", (yyvsp[(1) - (3)].id)->name))
		{
			warning("User has provided 'NumSgprs' but this number is automatically calculated");
		}
		else if (!strcmp("FloatMode", (yyvsp[(1) - (3)].id)->name))
		{
			si2bin_inner_bin->FloatMode = (yyvsp[(3) - (3)].num);
		}
		else if (!strcmp("IeeeMode", (yyvsp[(1) - (3)].id)->name))
		{	
			si2bin_inner_bin->IeeeMode = (yyvsp[(3) - (3)].num);
		}
		else if (!strcmp("COMPUTE_PGM_RSRC2", (yyvsp[(1) - (3)].id)->name))
		{
			warning("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else if(!strcmp("rat_op", (yyvsp[(1) - (3)].id)->name))
		{	
			si2bin_inner_bin->rat_op = (yyvsp[(3) - (3)].num);
		}
		else if (!strcmp("COMPUTE_PGM_RSRC2", (yyvsp[(1) - (3)].id)->name))
		{
			warning("User has provided 'COMPUTE_PGM_RSRC2' but this number is automatically calculated from provided PGM_RSRC2 fields");
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized assignment: %s", (yyvsp[(1) - (3)].id)->name);
		}

		/* Free id */
		si2bin_id_free((yyvsp[(1) - (3)].id));
	}
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 257 "parser.y"
    {
		struct si_bin_enc_user_element_t *user_elem;
		int err;

		/* check for correct syntax */
		if (strcmp("userElements", (yyvsp[(1) - (15)].id)->name))
			si2bin_yyerror_fmt("User Elements not correctly specified: %s", (yyvsp[(1) - (15)].id)->name);
		
		/* Make sure userElement index is in correct range */
		if ((yyvsp[(3) - (15)].num) > 15 || (yyvsp[(3) - (15)].num) < 0)
			si2bin_yyerror_fmt("User Elements index is out of allowed range (0 to 15)");

		/* Create userElement object */
		user_elem = si_bin_enc_user_element_create();
		user_elem->dataClass = str_map_string_err(&si_bin_user_data_class, (yyvsp[(6) - (15)].id)->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data class: %s", (yyvsp[(6) - (15)].id)->name);

		user_elem->apiSlot = (yyvsp[(8) - (15)].num);
		user_elem->startUserReg = (yyvsp[(12) - (15)].num);
		user_elem->userRegCount = (yyvsp[(14) - (15)].num) - (yyvsp[(12) - (15)].num) + 1;
		
		/* Add userElement to userElement list */
		si2bin_inner_bin_add_user_element(si2bin_inner_bin, user_elem, (yyvsp[(3) - (15)].num));

		/* Free id */
		si2bin_id_free((yyvsp[(1) - (15)].id));
		si2bin_id_free((yyvsp[(6) - (15)].id));
		si2bin_id_free((yyvsp[(10) - (15)].id));

	}
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 289 "parser.y"
    {
		/* Find pgm_rsrc2 information */
		if (strcmp("COMPUTE_PGM_RSRC2", (yyvsp[(1) - (5)].id)->name))
			si2bin_yyerror_fmt("Unrecognized assignment: %s", (yyvsp[(1) - (5)].id)->name);
		
		if (!strcmp("SCRATCH_EN", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->scrach_en = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("USER_SGPR", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->user_sgpr = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("TRAP_PRESENT", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->trap_present = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("TGID_X_EN", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_x_en = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("TGID_Y_EN", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_y_en = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("TGID_Z_EN", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tgid_z_en = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("TG_SIZE_EN", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tg_size_en = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("TIDIG_COMP_CNT", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->tidig_comp_cnt = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("EXCP_EN_MSB", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->excp_en_msb = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("LDS_SIZE", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->lds_size = (yyvsp[(5) - (5)].num);
		}
		else if (!strcmp("EXCP_EN", (yyvsp[(3) - (5)].id)->name))
		{
			si2bin_inner_bin->pgm_rsrc2->excp_en = (yyvsp[(5) - (5)].num);
		}
		else
		{
			si2bin_yyerror_fmt("Unrecognized field of COMPUTE_PGM_RSRC2: %s", (yyvsp[(3) - (5)].id)->name);
		}

		/* Free id's */
		si2bin_id_free((yyvsp[(1) - (5)].id));
		si2bin_id_free((yyvsp[(3) - (5)].id));
	}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 352 "parser.y"
    {
		(yyval.num) = (yyvsp[(1) - (1)].num);
	}
    break;

  case 30:

/* Line 1806 of yacc.c  */
#line 356 "parser.y"
    {
		int value;
		
		sscanf((yyvsp[(1) - (1)].id)->name, "%x", &value);

		si2bin_id_free((yyvsp[(1) - (1)].id));
		(yyval.num) = value;
	}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 383 "parser.y"
    {	
		si2bin_id_free((yyvsp[(1) - (2)].id));
	}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 387 "parser.y"
    {
		si2bin_id_free((yyvsp[(1) - (3)].id));
	}
    break;

  case 46:

/* Line 1806 of yacc.c  */
#line 402 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_int;
		data->int_value = (yyvsp[(2) - (3)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 412 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_int;
		data->int_value = (yyvsp[(2) - (2)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 427 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_short;
		data->short_value = (short)(yyvsp[(2) - (3)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 437 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_short;
		data->short_value = (short)(yyvsp[(2) - (2)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 452 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_float;
		data->float_value = (yyvsp[(2) - (3)].num_float);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 462 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_float;
		data->float_value = (yyvsp[(2) - (2)].num_float);

		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 477 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_word;
		data->word_value = (unsigned int)(yyvsp[(2) - (3)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 487 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_word;
		data->word_value = (unsigned int)(yyvsp[(2) - (2)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 502 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_half;
		data->half_value = (unsigned short)(yyvsp[(2) - (3)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 512 "parser.y"
    { 
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_half;
		data->half_value = (unsigned short)(yyvsp[(2) - (2)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 527 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_byte;
		data->byte_value = (unsigned char)(yyvsp[(2) - (3)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 537 "parser.y"
    {
		struct si2bin_data_t *data;
		
		data = si2bin_data_create();
		data->data_type = si2bin_data_byte;
		data->byte_value = (unsigned char)(yyvsp[(2) - (2)].num);
		
		si2bin_outer_bin_add_data(si2bin_outer_bin, data);
	}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 565 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(4) - (4)].si_arg);
		int err;

		
		/* Set argument name */
		si_arg_name_set(arg, (yyvsp[(2) - (4)].id)->name);
		
		/* Set arg fields */
		arg->value.data_type = str_map_string_err(&si_arg_data_type_map, (yyvsp[(1) - (4)].id)->name, &err);
		if(err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", (yyvsp[(1) - (4)].id)->name);

		arg->value.num_elems = 1;
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = (yyvsp[(3) - (4)].num);
		
		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free((yyvsp[(1) - (4)].id));
		si2bin_id_free((yyvsp[(2) - (4)].id));
	}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 588 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(7) - (7)].si_arg);
		int err;

		/* Set argument name */
		si_arg_name_set(arg, (yyvsp[(5) - (7)].id)->name);
		
		/* Set argument fields */
		arg->value.data_type = str_map_string_err(&si_arg_data_type_map, (yyvsp[(1) - (7)].id)->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", (yyvsp[(1) - (7)].id)->name);
		
		arg->value.num_elems = (yyvsp[(3) - (7)].num);
		arg->value.constant_buffer_num = 1;
		arg->value.constant_offset = (yyvsp[(6) - (7)].num);
		
		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free((yyvsp[(1) - (7)].id));
		si2bin_id_free((yyvsp[(5) - (7)].id));
	}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 610 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(5) - (5)].si_arg);
		int err;

		/* Set new argument name */
		si_arg_name_set(arg, (yyvsp[(3) - (5)].id)->name);
		
		/* Initialize argument */
		arg->pointer.num_elems = 1;
		arg->pointer.data_type = str_map_string_err(&si_arg_data_type_map, (yyvsp[(1) - (5)].id)->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", (yyvsp[(1) - (5)].id)->name);

		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = (yyvsp[(4) - (5)].num);

		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free((yyvsp[(1) - (5)].id));
		si2bin_id_free((yyvsp[(3) - (5)].id));
	}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 632 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(8) - (8)].si_arg);
		int err;

		/* Set new argument name */
		si_arg_name_set(arg, (yyvsp[(6) - (8)].id)->name);
		
		/* Initialize argument */
		arg->pointer.num_elems = (yyvsp[(3) - (8)].num);
		arg->pointer.data_type = str_map_string_err(&si_arg_data_type_map, (yyvsp[(1) - (8)].id)->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized data type: %s", (yyvsp[(1) - (8)].id)->name);
		
		arg->pointer.constant_buffer_num = 1;
		arg->pointer.constant_offset = (yyvsp[(7) - (8)].num);


		/* Insert argument and free identifiers */
		si2bin_metadata_add_arg(si2bin_metadata, arg);
		si2bin_id_free((yyvsp[(1) - (8)].id));
		si2bin_id_free((yyvsp[(6) - (8)].id));
	}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 659 "parser.y"
    {
		struct si_arg_t *arg;

		/* Create an argument with defaults*/
		arg = si_arg_create(si_arg_value, "arg");

		(yyval.si_arg) = arg;
	}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 668 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(1) - (2)].si_arg);

		/* set constarg field to true */
		arg->constarg = 1;
		
		/* Return argument */
		(yyval.si_arg) = arg;
	}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 681 "parser.y"
    {
		struct si_arg_t *arg;

		/* Create an argument with defaults*/
		arg = si_arg_create(si_arg_pointer, "arg");
		arg->pointer.scope = si_arg_uav;
		arg->pointer.buffer_num = 12;
		arg->pointer.access_type = si_arg_read_write;
		
		(yyval.si_arg) = arg;
	}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 693 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(1) - (2)].si_arg);
		struct si2bin_id_t *id = (yyvsp[(2) - (2)].id);
		int err;

		/* Translate access type */
		arg->pointer.access_type = str_map_string_err(&si_arg_access_type_map, id->name, &err);
		if (err)
			si2bin_yyerror_fmt("Unrecognized access type: %s", id->name);
		
		/* Free ID and return argument */
		si2bin_id_free(id);
		(yyval.si_arg) = arg;
	}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 708 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(1) - (2)].si_arg);
		struct si2bin_id_t *id = (yyvsp[(2) - (2)].id);
	
		/* Obtain UAV index */
		arg->pointer.scope = si_arg_uav;
		arg->pointer.buffer_num = atoi(id->name + 3);

		/* Free ID and return argument */
		si2bin_id_free(id);
		(yyval.si_arg) = arg;
	}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 721 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(1) - (2)].si_arg);
	
		/* Set scope to hl */
		arg->pointer.scope = si_arg_hw_local;
		arg->pointer.buffer_num = 1;

		/* Return argument */
		(yyval.si_arg) = arg;
	}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 732 "parser.y"
    {
		struct si_arg_t *arg = (yyvsp[(1) - (2)].si_arg);
	
		/* set constarg field to true */
		arg->constarg = 1;

		/* Return argument */
		(yyval.si_arg) = arg;
	}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 760 "parser.y"
    {
		struct si2bin_inst_t *inst = (yyvsp[(1) - (1)].inst);

		/* Generate code */
		si2bin_inst_gen(inst);
		elf_enc_buffer_write(si2bin_entry->text_section_buffer, inst->inst_bytes.bytes, inst->size);
		
		/* Dump Instruction Info */
		//si2bin_inst_dump(inst, stdout);
		si2bin_inst_free(inst);
	}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 776 "parser.y"
    {
		struct si2bin_id_t *id = (yyvsp[(1) - (2)].id);
		struct si2bin_symbol_t *symbol;

		
		/* Check if symbol exists */
		symbol = hash_table_get(si2bin_symbol_table, id->name);
		if (symbol && symbol->defined)
			si2bin_yyerror_fmt("multiply defined label: %s", id->name);

		/* Create if it does not exists */
		if (!symbol)
		{
			symbol = si2bin_symbol_create(id->name);
			hash_table_insert(si2bin_symbol_table, id->name, symbol);
		}

		/* Define symbol */
		symbol->defined = 1;
		symbol->value = si2bin_entry->text_section_buffer->offset;		

		/* End */
		si2bin_id_free(id);
	}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 803 "parser.y"
    {
		struct si2bin_inst_t *inst;
		struct si2bin_id_t *id;
		struct list_t *arg_list;

		/* Get arguments */
		id = (yyvsp[(1) - (2)].id);
		arg_list = (yyvsp[(2) - (2)].list);
		
		/* Create instruction */
		inst = si2bin_inst_create_with_name(id->name, arg_list);

		/* Return instructions */
		si2bin_id_free(id);
		(yyval.inst) = inst;
	}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 823 "parser.y"
    {
		(yyval.list) = NULL;
	}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 828 "parser.y"
    {
		struct list_t *arg_list;
		
		/* Create */
		arg_list = list_create();
		
		/* Initialize */
		list_add(arg_list, (yyvsp[(1) - (1)].arg));
		
		/* Return the arg list */
		(yyval.list) = arg_list;
	}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 842 "parser.y"
    {
		/* Add argument to head of list_t in $3 */
		list_insert((yyvsp[(3) - (3)].list), 0, (yyvsp[(1) - (3)].arg));
		
		/* Return the arg list */
		(yyval.list) = (yyvsp[(3) - (3)].list);

	}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 855 "parser.y"
    {	
		int value;

		value = atoi((yyvsp[(1) - (1)].id)->name + 1);
		(yyval.arg) = si2bin_arg_create_scalar_register(value); 

		if (value >= si2bin_inner_bin->num_sgprs)
			si2bin_inner_bin->num_sgprs = value + 1;
		
		si2bin_id_free((yyvsp[(1) - (1)].id));
	}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 868 "parser.y"
    {
		int value;

		value = atoi((yyvsp[(1) - (1)].id)->name + 1);
		(yyval.arg) = si2bin_arg_create_vector_register(value); 
		
		if (value >= si2bin_inner_bin->num_vgprs)
			si2bin_inner_bin->num_vgprs = value + 1;

		si2bin_id_free((yyvsp[(1) - (1)].id));
	}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 881 "parser.y"
    {
		enum si_inst_special_reg_t reg;
		reg = str_map_string(&si_inst_special_reg_map, (yyvsp[(1) - (1)].id)->name);
		(yyval.arg) = si2bin_arg_create_special_register(reg); 
		si2bin_id_free((yyvsp[(1) - (1)].id));
	}
    break;

  case 112:

/* Line 1806 of yacc.c  */
#line 889 "parser.y"
    {
		(yyval.arg) = si2bin_arg_create_mem_register(atoi((yyvsp[(1) - (1)].id)->name + 1));
		si2bin_id_free((yyvsp[(1) - (1)].id));
	}
    break;

  case 113:

/* Line 1806 of yacc.c  */
#line 895 "parser.y"
    {
		(yyval.arg) = si2bin_arg_create_literal((yyvsp[(1) - (1)].num));
	}
    break;

  case 114:

/* Line 1806 of yacc.c  */
#line 900 "parser.y"
    {
		int value;

		sscanf((yyvsp[(1) - (1)].id)->name, "%x", &value);
		(yyval.arg) = si2bin_arg_create_literal(value); 
		si2bin_id_free((yyvsp[(1) - (1)].id));
	}
    break;

  case 115:

/* Line 1806 of yacc.c  */
#line 909 "parser.y"
    {
		(yyval.arg) = si2bin_arg_create_literal_float((yyvsp[(1) - (1)].num_float));
	}
    break;

  case 116:

/* Line 1806 of yacc.c  */
#line 916 "parser.y"
    {
		(yyval.arg) = (yyvsp[(1) - (1)].arg);
	}
    break;

  case 117:

/* Line 1806 of yacc.c  */
#line 921 "parser.y"
    {
		struct si2bin_arg_t *arg;
		struct si2bin_id_t *id;

		int low;
		int high;

		/* Read arguments */
		id = (yyvsp[(1) - (6)].id);
		low = (yyvsp[(3) - (6)].num);
		high = (yyvsp[(5) - (6)].num);
		
		/* Create argument */
		arg = si2bin_arg_create(); 
		
		/* Initialize */
		if (!strcmp(id->name, "s"))
		{
			arg->type = si2bin_arg_scalar_register_series;
			arg->value.scalar_register_series.low = low;
			arg->value.scalar_register_series.high = high;

			if (high >= si2bin_inner_bin->num_sgprs)
				si2bin_inner_bin->num_sgprs = high + 1;
		}
		else if (!strcmp(id->name, "v"))
		{
			arg->type = si2bin_arg_vector_register_series;
			arg->value.vector_register_series.low = low;
			arg->value.vector_register_series.high = high;
			
			if (high >= si2bin_inner_bin->num_vgprs)
				si2bin_inner_bin->num_vgprs = high + 1;
		}
		else
		{
			si2bin_yyerror_fmt("invalid register series: %s", id->name);
		}
		
		/* Return created argument */
		si2bin_id_free(id);
		(yyval.arg) = arg;
	}
    break;

  case 118:

/* Line 1806 of yacc.c  */
#line 966 "parser.y"
    {
		struct si2bin_arg_t *arg = (yyvsp[(3) - (4)].arg);

		/* Activate absolute value flag */
		arg->abs = 1;

		/* Check valid application of 'abs' */
		switch (arg->type)
		{
		case si2bin_arg_scalar_register:
		case si2bin_arg_scalar_register_series:
		case si2bin_arg_vector_register:
		case si2bin_arg_vector_register_series:
			break;

		default:
			si2bin_yyerror("abs() function not allowed for argument");
		}

		/* Return */
		(yyval.arg) = arg;
	}
    break;

  case 119:

/* Line 1806 of yacc.c  */
#line 990 "parser.y"
    {
		struct si2bin_arg_t *arg = (yyvsp[(2) - (2)].arg);

		/* Activate absolute value flag */
		arg->neg = 1;

		/* Check valid application of 'abs' */
		switch (arg->type)
		{
		case si2bin_arg_scalar_register:
		case si2bin_arg_scalar_register_series:
		case si2bin_arg_vector_register:
		case si2bin_arg_vector_register_series:
			break;

		default:
			si2bin_yyerror("abs() function not allowed for argument");
		}

		/* Return */
		(yyval.arg) = arg;
	}
    break;

  case 120:

/* Line 1806 of yacc.c  */
#line 1014 "parser.y"
    {
		struct si2bin_arg_t *arg;
		struct si2bin_arg_t *soffset;
		struct si2bin_arg_t *qual;

		struct si2bin_id_t *id_data_format;
		struct si2bin_id_t *id_num_format;
		
		enum si_inst_buf_data_format_t data_format;
		enum si_inst_buf_num_format_t num_format;
		
		int err;

		/* Read arguments */
		soffset = (yyvsp[(1) - (9)].arg);
		qual = (yyvsp[(2) - (9)].arg);
		id_data_format = (yyvsp[(6) - (9)].id);
		id_num_format = (yyvsp[(8) - (9)].id);
		
		/* Data format */
		data_format = str_map_string_err(&si_inst_buf_data_format_map,
				id_data_format->name, &err);
		if (err)
			si2bin_yyerror_fmt("%s: invalid data format", id_data_format->name);
			
		/* Number format */
		num_format = str_map_string_err(&si_inst_buf_num_format_map,
				id_num_format->name, &err);
		if (err)
			si2bin_yyerror_fmt("%s: invalid number format", id_num_format->name); 

		/* Create argument */
		arg = si2bin_arg_create_maddr(soffset, qual, data_format, num_format);	
			
		/* Return */
		si2bin_id_free(id_data_format);
		si2bin_id_free(id_num_format);
		(yyval.arg) = arg;
	}
    break;

  case 121:

/* Line 1806 of yacc.c  */
#line 1055 "parser.y"
    {
		struct si2bin_arg_t *arg;
		struct si2bin_id_t *id;

		/* Get ID */
		id = (yyvsp[(1) - (1)].id);
		
		/* Create argument */
		arg = si2bin_arg_create_label(id->name);
		si2bin_id_free(id);

		/* Return argument */
		(yyval.arg) = arg;
	}
    break;

  case 122:

/* Line 1806 of yacc.c  */
#line 1071 "parser.y"
    {
		/* The return value is given by 'waitcnt_arg's definition */
	}
    break;

  case 123:

/* Line 1806 of yacc.c  */
#line 1079 "parser.y"
    {
		(yyval.arg) = si2bin_arg_create_maddr_qual();
	}
    break;

  case 124:

/* Line 1806 of yacc.c  */
#line 1084 "parser.y"
    {
		struct si2bin_arg_t *qual = (yyvsp[(1) - (2)].arg);

		assert(qual->type == si2bin_arg_maddr_qual);
		if (qual->value.maddr_qual.offen)
			si2bin_yyerror("redundant qualifier 'offen'");
		qual->value.maddr_qual.offen = 1;
		(yyval.arg) = qual;
	}
    break;

  case 125:

/* Line 1806 of yacc.c  */
#line 1095 "parser.y"
    {
		struct si2bin_arg_t *qual = (yyvsp[(1) - (2)].arg);

		assert(qual->type == si2bin_arg_maddr_qual);
		if (qual->value.maddr_qual.idxen)
			si2bin_yyerror("redundant qualifier 'idxen'");
		qual->value.maddr_qual.idxen = 1;
		(yyval.arg) = qual;
	}
    break;

  case 126:

/* Line 1806 of yacc.c  */
#line 1106 "parser.y"
    {
		struct si2bin_arg_t *qual = (yyvsp[(1) - (4)].arg);
		int offset = (yyvsp[(4) - (4)].num);

		assert(qual->type == si2bin_arg_maddr_qual);
		qual->value.maddr_qual.offset = offset;
		/* FIXME - check range of 'offset' */
		(yyval.arg) = qual;
	}
    break;

  case 128:

/* Line 1806 of yacc.c  */
#line 1121 "parser.y"
    {
		if ((yyvsp[(3) - (3)].arg)->value.wait_cnt.vmcnt_active && (yyvsp[(1) - (3)].arg)->value.wait_cnt.vmcnt_active)
			si2bin_yyerror("duplicate 'vmcnt' token");
		(yyvsp[(3) - (3)].arg)->value.wait_cnt.vmcnt_active += (yyvsp[(1) - (3)].arg)->value.wait_cnt.vmcnt_active;
		(yyvsp[(3) - (3)].arg)->value.wait_cnt.vmcnt_value += (yyvsp[(1) - (3)].arg)->value.wait_cnt.vmcnt_value;		
		
		if ((yyvsp[(3) - (3)].arg)->value.wait_cnt.expcnt_active && (yyvsp[(1) - (3)].arg)->value.wait_cnt.expcnt_active)
			si2bin_yyerror("duplicate 'expcnt' token");
		(yyvsp[(3) - (3)].arg)->value.wait_cnt.expcnt_active += (yyvsp[(1) - (3)].arg)->value.wait_cnt.expcnt_active;
		(yyvsp[(3) - (3)].arg)->value.wait_cnt.expcnt_value += (yyvsp[(1) - (3)].arg)->value.wait_cnt.expcnt_value;	
		
		if ((yyvsp[(3) - (3)].arg)->value.wait_cnt.lgkmcnt_active && (yyvsp[(1) - (3)].arg)->value.wait_cnt.lgkmcnt_active)
			si2bin_yyerror("duplicate 'lgkmcnt' token");
		(yyvsp[(3) - (3)].arg)->value.wait_cnt.lgkmcnt_active += (yyvsp[(1) - (3)].arg)->value.wait_cnt.lgkmcnt_active;
		(yyvsp[(3) - (3)].arg)->value.wait_cnt.lgkmcnt_value += (yyvsp[(1) - (3)].arg)->value.wait_cnt.lgkmcnt_value;	
		
		si2bin_arg_free((yyvsp[(1) - (3)].arg));
		(yyval.arg) = (yyvsp[(3) - (3)].arg);
	}
    break;

  case 129:

/* Line 1806 of yacc.c  */
#line 1145 "parser.y"
    {
		struct si2bin_arg_t *arg;
		struct si2bin_id_t *id;

		/* Read arguments */
		id = (yyvsp[(1) - (4)].id);
		
		/* Create argument */
		arg = si2bin_arg_create(); 
		
		/* Initialize */
		arg->type = si2bin_arg_waitcnt;
		
		if (!strcmp(id->name, "vmcnt"))
		{
			arg->value.wait_cnt.vmcnt_active = 1;
			arg->value.wait_cnt.vmcnt_value = (yyvsp[(3) - (4)].num);
		}
		else if (!strcmp(id->name, "expcnt"))
		{
			arg->value.wait_cnt.expcnt_active = 1;
			arg->value.wait_cnt.expcnt_value = (yyvsp[(3) - (4)].num);
		}
		else if (!strcmp(id->name, "lgkmcnt"))
		{
			arg->value.wait_cnt.lgkmcnt_active = 1;
			arg->value.wait_cnt.lgkmcnt_value = (yyvsp[(3) - (4)].num);
		}
		
		/* Return */
		si2bin_id_free(id);
		(yyval.arg) = arg;
	}
    break;



/* Line 1806 of yacc.c  */
#line 2832 "parser.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 1180 "parser.y"


