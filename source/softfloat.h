/*============================================================================
 
 This C source file is part of the SoftFloat IEC/IEEE Floating-point Arithmetic
 Package, Release 2b.
 
 Written by John R. Hauser.  This work was made possible in part by the
 International Computer Science Institute, located at Suite 600, 1947 Center
 Street, Berkeley, California 94704.  Funding was partially provided by the
 National Science Foundation under grant MIP-9311980.  The original version
 of this code was written as part of a project to build a fixed-point vector
 processor in collaboration with the University of California at Berkeley,
 overseen by Profs. Nelson Morgan and John Wawrzynek.  More information
 is available through the Web page `http://www.cs.berkeley.edu/~jhauser/
 arithmetic/SoftFloat.html'.
 
 THIS SOFTWARE IS DISTRIBUTED AS IS, FOR FREE.  Although reasonable effort has
 been made to avoid it, THIS SOFTWARE MAY CONTAIN FAULTS THAT WILL AT TIMES
 RESULT IN INCORRECT BEHAVIOR.  USE OF THIS SOFTWARE IS RESTRICTED TO PERSONS
 AND ORGANIZATIONS WHO CAN AND WILL TAKE FULL RESPONSIBILITY FOR ALL LOSSES,
 COSTS, OR OTHER PROBLEMS THEY INCUR DUE TO THE SOFTWARE, AND WHO FURTHERMORE
 EFFECTIVELY INDEMNIFY JOHN HAUSER AND THE INTERNATIONAL COMPUTER SCIENCE
 INSTITUTE (possibly via similar legal warning) AGAINST ALL LOSSES, COSTS, OR
 OTHER PROBLEMS INCURRED BY THEIR CUSTOMERS AND CLIENTS DUE TO THE SOFTWARE.
 
 Derivative works are acceptable, even for commercial purposes, so long as
 (1) the source code for the derivative work includes prominent notice that
 the work is derivative, and (2) the source code includes prominent notice with
 these four paragraphs for those parts of this code that are retained.
 
 =============================================================================*/

/*
 *  softfloat.h
 *  
 *	A derivative work.
 *  Compiled and tested by Eugene Rockey using OSX Command Line on 9/16/14.
 *	
 */
/*----------------------------------------------------------------------------
 | Software IEC/IEEE floating-point types.
 *----------------------------------------------------------------------------*/
typedef unsigned int float32;

// other types used
typedef int flag;
typedef int int8;
typedef int int16;

/*
 -------------------------------------------------------------------------------
 Each of the following `typedef's defines a type that holds integers
 of _exactly_ the number of bits specified.  For instance, for most
 implementation of C, `bits16' and `sbits16' should be `typedef'ed to
 `unsigned short int' and `signed short int' (or `short int'), respectively.
 -------------------------------------------------------------------------------
 */
typedef unsigned short int bits16;
typedef unsigned int bits32;
typedef signed int sbits32;


/*----------------------------------------------------------------------------
 | Software IEC/IEEE floating-point underflow tininess-detection mode.
 *----------------------------------------------------------------------------*/
int8 float_detect_tininess;
enum {
    float_tininess_after_rounding  = 0,
    float_tininess_before_rounding = 1
};

/*----------------------------------------------------------------------------
 | Software IEC/IEEE floating-point rounding mode.
 *----------------------------------------------------------------------------*/
signed char float_rounding_mode;
enum {
    float_round_nearest_even = 0,
    float_round_down         = 1,
    float_round_up           = 2,
    float_round_to_zero      = 3
};

/*----------------------------------------------------------------------------
 | Software IEC/IEEE floating-point exception flags.
 *----------------------------------------------------------------------------*/
signed char float_exception_flags;
enum {
    float_flag_invalid   =  1,
    float_flag_divbyzero =  4,
    float_flag_overflow  =  8,
    float_flag_underflow = 16,
    float_flag_inexact   = 32
};

/*----------------------------------------------------------------------------
 | Routine to raise any or all of the software IEC/IEEE floating-point
 | exception flags.
 *----------------------------------------------------------------------------*/
void float_raise( int );

/*----------------------------------------------------------------------------
 | Software IEC/IEEE integer-to-floating-point conversion routines.
 *----------------------------------------------------------------------------*/
float32 int32_to_float32( int );

/*----------------------------------------------------------------------------
 | Software IEC/IEEE single-precision conversion routines.
 *----------------------------------------------------------------------------*/
int float32_to_int32( float32 );
int float32_to_int32_round_to_zero( float32 );

/*----------------------------------------------------------------------------
 | Software IEC/IEEE single-precision operations.
 *----------------------------------------------------------------------------*/
float32 float32_round_to_int( float32 );
float32 float32_add( float32, float32 );
float32 float32_sub( float32, float32 );
char float32_eq( float32, float32 );
char float32_le( float32, float32 );
char float32_lt( float32, float32 );
char float32_eq_signaling( float32, float32 );
char float32_le_quiet( float32, float32 );
char float32_lt_quiet( float32, float32 );
flag float32_is_signaling_nan( float32 );

//Eugene's Operations
unsigned int ASCII_to_float32(char*);
char* float32_to_ASCII(float32);

/*----------------------------------------------------------------------------
 | Symbolic Boolean literals.
 *----------------------------------------------------------------------------*/
enum {
    FALSE = 0,
    TRUE  = 1
};
