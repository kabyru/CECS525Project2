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
 *  softfloat.c
 *  
 *	A derivative work.
 *  Compiled and tested by Eugene Rockey using OSX Command line on 9/16/14.
*/

#include "softfloat.h"

/*----------------------------------------------------------------------------
 | Returns 1 if the single-precision floating-point value `a' is a signaling
 | NaN; otherwise returns 0.
 *----------------------------------------------------------------------------*/

flag float32_is_signaling_nan( float32 a )
{
	
    return ( ( ( a>>22 ) & 0x1FF ) == 0x1FE ) && ( a & 0x003FFFFF );
	
}

/*----------------------------------------------------------------------------
 | Underflow tininess-detection mode, statically initialized to default value.
 | (The declaration in `softfloat.h' must match the `int8' type here.)
 *----------------------------------------------------------------------------*/
int8 float_detect_tininess = float_tininess_after_rounding;


/*----------------------------------------------------------------------------
 | Returns 1 if the single-precision floating-point value `a' is a NaN;
 | otherwise returns 0.
 *----------------------------------------------------------------------------*/

flag float32_is_nan( float32 a )
{
	
    return ( 0xFF000000 < (bits32) ( a<<1 ) );
	
}



/*----------------------------------------------------------------------------
 | Returns the fraction bits of the single-precision floating-point value `a'.
 *----------------------------------------------------------------------------*/

bits32 extractFloat32Frac( float32 a )
{
	
    return a & 0x007FFFFF;
	
}

/*----------------------------------------------------------------------------
 | Returns the exponent bits of the single-precision floating-point value `a'.
 *----------------------------------------------------------------------------*/

int16 extractFloat32Exp( float32 a )
{
	
    return ( a>>23 ) & 0xFF;
	
}

/*----------------------------------------------------------------------------
 | Returns the sign bit of the single-precision floating-point value `a'.
 *----------------------------------------------------------------------------*/

flag extractFloat32Sign( float32 a )
{
	
    return a>>31;
	
}

/*----------------------------------------------------------------------------
 | Takes two single-precision floating-point values `a' and `b', one of which
 | is a NaN, and returns the appropriate NaN result.  If either `a' or `b' is a
 | signaling NaN, the invalid exception is raised.
 *----------------------------------------------------------------------------*/

static float32 propagateFloat32NaN( float32 a, float32 b )
{
    flag aIsNaN, aIsSignalingNaN, bIsNaN, bIsSignalingNaN;
	
    aIsNaN = float32_is_nan( a );
    aIsSignalingNaN = float32_is_signaling_nan( a );
    bIsNaN = float32_is_nan( b );
    bIsSignalingNaN = float32_is_signaling_nan( b );
    a |= 0x00400000;
    b |= 0x00400000;
    if ( aIsSignalingNaN | bIsSignalingNaN ) float_raise( float_flag_invalid );
    if ( aIsSignalingNaN ) {
        if ( bIsSignalingNaN ) goto returnLargerSignificand;
        return bIsNaN ? b : a;
    }
    else if ( aIsNaN ) {
        if ( bIsSignalingNaN | ! bIsNaN ) return a;
	returnLargerSignificand:
        if ( (bits32) ( a<<1 ) < (bits32) ( b<<1 ) ) return b;
        if ( (bits32) ( b<<1 ) < (bits32) ( a<<1 ) ) return a;
        return ( a < b ) ? a : b;
    }
    else {
        return b;
    }
	
}

/*----------------------------------------------------------------------------
 | Shifts `a' right by the number of bits given in `count'.  If any nonzero
 | bits are shifted off, they are ``jammed'' into the least significant bit of
 | the result by setting the least significant bit to 1.  The value of `count'
 | can be arbitrarily large; in particular, if `count' is greater than 32, the
 | result will be either 0 or 1, depending on whether `a' is zero or nonzero.
 | The result is stored in the location pointed to by `zPtr'.
 *----------------------------------------------------------------------------*/

void shift32RightJamming( bits32 a, int16 count, bits32 *zPtr )
{
    bits32 z;
	
    if ( count == 0 ) {
        z = a;
    }
    else if ( count < 32 ) {
        z = ( a>>count ) | ( ( a<<( ( - count ) & 31 ) ) != 0 );
    }
    else {
        z = ( a != 0 );
    }
    *zPtr = z;
	
}

/*----------------------------------------------------------------------------
 | Packs the sign `zSign', exponent `zExp', and significand `zSig' into a
 | single-precision floating-point value, returning the result.  After being
 | shifted into the proper positions, the three fields are simply added
 | together to form the result.  This means that any integer portion of `zSig'
 | will be added into the exponent.  Since a properly normalized significand
 | will have an integer portion equal to 1, the `zExp' input should be 1 less
 | than the desired result exponent whenever `zSig' is a complete, normalized
 | significand.
 *----------------------------------------------------------------------------*/

float32 packFloat32( flag zSign, int16 zExp, bits32 zSig )
{
	
    return ( ( (bits32) zSign )<<31 ) + ( ( (bits32) zExp )<<23 ) + zSig;
	
}


/*----------------------------------------------------------------------------
 | Takes an abstract floating-point value having sign `zSign', exponent `zExp',
 | and significand `zSig', and returns the proper single-precision floating-
 | point value corresponding to the abstract input.  Ordinarily, the abstract
 | value is simply rounded and packed into the single-precision format, with
 | the inexact exception raised if the abstract input cannot be represented
 | exactly.  However, if the abstract value is too large, the overflow and
 | inexact exceptions are raised and an infinity or maximal finite value is
 | returned.  If the abstract value is too small, the input value is rounded to
 | a subnormal number, and the underflow and inexact exceptions are raised if
 | the abstract input cannot be represented exactly as a subnormal single-
 | precision floating-point number.
 |     The input significand `zSig' has its binary point between bits 30
 | and 29, which is 7 bits to the left of the usual location.  This shifted
 | significand must be normalized or smaller.  If `zSig' is not normalized,
 | `zExp' must be 0; in that case, the result returned is a subnormal number,
 | and it must not require rounding.  In the usual case that `zSig' is
 | normalized, `zExp' must be 1 less than the ``true'' floating-point exponent.
 | The handling of underflow and overflow follows the IEC/IEEE Standard for
 | Binary Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/

static float32 roundAndPackFloat32( flag zSign, int16 zExp, bits32 zSig )
{
    int8 roundingMode;
    flag roundNearestEven;
    int8 roundIncrement, roundBits;
    flag isTiny;
	
    roundingMode = float_rounding_mode;
    roundNearestEven = roundingMode == float_round_nearest_even;
    roundIncrement = 0x40;
    if ( ! roundNearestEven ) {
        if ( roundingMode == float_round_to_zero ) {
            roundIncrement = 0;
        }
        else {
            roundIncrement = 0x7F;
            if ( zSign ) {
                if ( roundingMode == float_round_up ) roundIncrement = 0;
            }
            else {
                if ( roundingMode == float_round_down ) roundIncrement = 0;
            }
        }
    }
    roundBits = zSig & 0x7F;
    if ( 0xFD <= (bits16) zExp ) {
        if (    ( 0xFD < zExp )
			|| (    ( zExp == 0xFD )
				&& ( (sbits32) ( zSig + roundIncrement ) < 0 ) )
			) {
            float_raise( float_flag_overflow | float_flag_inexact );
            return packFloat32( zSign, 0xFF, 0 ) - ( roundIncrement == 0 );
        }
        if ( zExp < 0 ) {
            isTiny =
			( float_detect_tininess == float_tininess_before_rounding )
			|| ( zExp < -1 )
			|| ( zSig + roundIncrement < 0x80000000 );
            shift32RightJamming( zSig, - zExp, &zSig );
            zExp = 0;
            roundBits = zSig & 0x7F;
            if ( isTiny && roundBits ) float_raise( float_flag_underflow );
        }
    }
    if ( roundBits ) float_exception_flags |= float_flag_inexact;
    zSig = ( zSig + roundIncrement )>>7;
    zSig &= ~ ( ( ( roundBits ^ 0x40 ) == 0 ) & roundNearestEven );
    if ( zSig == 0 ) zExp = 0;
    return packFloat32( zSign, zExp, zSig );
	
}

/*----------------------------------------------------------------------------
 | Returns the result of adding the absolute values of the single-precision
 | floating-point values `a' and `b'.  If `zSign' is 1, the sum is negated
 | before being returned.  `zSign' is ignored if the result is a NaN.
 | The addition is performed according to the IEC/IEEE Standard for Binary
 | Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/

static float32 addFloat32Sigs( float32 a, float32 b, flag zSign )
{
    int16 aExp, bExp, zExp;
    bits32 aSig, bSig, zSig;
    int16 expDiff;
	
    aSig = extractFloat32Frac( a );
    aExp = extractFloat32Exp( a );
    bSig = extractFloat32Frac( b );
    bExp = extractFloat32Exp( b );
    expDiff = aExp - bExp;
    aSig <<= 6;
    bSig <<= 6;
    if ( 0 < expDiff ) {
        if ( aExp == 0xFF ) {
            if ( aSig ) return propagateFloat32NaN( a, b );
            return a;
        }
        if ( bExp == 0 ) {
            --expDiff;
        }
        else {
            bSig |= 0x20000000;
        }
        shift32RightJamming( bSig, expDiff, &bSig );
        zExp = aExp;
    }
    else if ( expDiff < 0 ) {
        if ( bExp == 0xFF ) {
            if ( bSig ) return propagateFloat32NaN( a, b );
            return packFloat32( zSign, 0xFF, 0 );
        }
        if ( aExp == 0 ) {
            ++expDiff;
        }
        else {
            aSig |= 0x20000000;
        }
        shift32RightJamming( aSig, - expDiff, &aSig );
        zExp = bExp;
    }
    else {
        if ( aExp == 0xFF ) {
            if ( aSig | bSig ) return propagateFloat32NaN( a, b );
            return a;
        }
        if ( aExp == 0 ) return packFloat32( zSign, 0, ( aSig + bSig )>>6 );
        zSig = 0x40000000 + aSig + bSig;
        zExp = aExp;
        goto roundAndPack;
    }
    aSig |= 0x20000000;
    zSig = ( aSig + bSig )<<1;
    --zExp;
    if ( (sbits32) zSig < 0 ) {
        zSig = aSig + bSig;
        ++zExp;
    }
roundAndPack:
    return roundAndPackFloat32( zSign, zExp, zSig );
	
}

/*----------------------------------------------------------------------------
 | Raises the exceptions specified by `flags'.  Floating-point traps can be
 | defined here if desired.  It is currently not possible for such a trap
 | to substitute a result value.  If traps are not implemented, this routine
 | should be simply `float_exception_flags |= flags;'.
 *----------------------------------------------------------------------------*/

void float_raise( int8 flags )
{
	
    float_exception_flags |= flags;
	
}

/*----------------------------------------------------------------------------
 | Returns the number of leading 0 bits before the most-significant 1 bit of
 | `a'.  If `a' is zero, 32 is returned.
 *----------------------------------------------------------------------------*/

static int8 countLeadingZeros32( bits32 a )
{
    static const int8 countLeadingZerosHigh[] = {
        8, 7, 6, 6, 5, 5, 5, 5, 4, 4, 4, 4, 4, 4, 4, 4,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };
    int8 shiftCount;
	
    shiftCount = 0;
    if ( a < 0x10000 ) {
        shiftCount += 16;
        a <<= 16;
    }
    if ( a < 0x1000000 ) {
        shiftCount += 8;
        a <<= 8;
    }
    shiftCount += countLeadingZerosHigh[ a>>24 ];
    return shiftCount;
	
}

/*----------------------------------------------------------------------------
 | Takes an abstract floating-point value having sign `zSign', exponent `zExp',
 | and significand `zSig', and returns the proper single-precision floating-
 | point value corresponding to the abstract input.  This routine is just like
 | `roundAndPackFloat32' except that `zSig' does not have to be normalized.
 | Bit 31 of `zSig' must be zero, and `zExp' must be 1 less than the ``true''
 | floating-point exponent.
 *----------------------------------------------------------------------------*/

static float32 normalizeRoundAndPackFloat32( flag zSign, int16 zExp, bits32 zSig )
{
    int8 shiftCount;
	
    shiftCount = countLeadingZeros32( zSig ) - 1;
    return roundAndPackFloat32( zSign, zExp - shiftCount, zSig<<shiftCount );
	
}

/*----------------------------------------------------------------------------
 | Returns the result of subtracting the absolute values of the single-
 | precision floating-point values `a' and `b'.  If `zSign' is 1, the
 | difference is negated before being returned.  `zSign' is ignored if the
 | result is a NaN.  The subtraction is performed according to the IEC/IEEE
 | Standard for Binary Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/

static float32 subFloat32Sigs( float32 a, float32 b, flag zSign )
{
    int16 aExp, bExp, zExp;
    bits32 aSig, bSig, zSig;
    int16 expDiff;
	float32 float32_default_nan = 0xFFC00000;
	
    aSig = extractFloat32Frac( a );
    aExp = extractFloat32Exp( a );
    bSig = extractFloat32Frac( b );
    bExp = extractFloat32Exp( b );
    expDiff = aExp - bExp;
    aSig <<= 7;
    bSig <<= 7;
    if ( 0 < expDiff ) goto aExpBigger;
    if ( expDiff < 0 ) goto bExpBigger;
    if ( aExp == 0xFF ) {
        if ( aSig | bSig ) return propagateFloat32NaN( a, b );
        float_raise( float_flag_invalid );
        return float32_default_nan;
    }
    if ( aExp == 0 ) {
        aExp = 1;
        bExp = 1;
    }
    if ( bSig < aSig ) goto aBigger;
    if ( aSig < bSig ) goto bBigger;
    return packFloat32( float_rounding_mode == float_round_down, 0, 0 );
bExpBigger:
    if ( bExp == 0xFF ) {
        if ( bSig ) return propagateFloat32NaN( a, b );
        return packFloat32( zSign ^ 1, 0xFF, 0 );
    }
    if ( aExp == 0 ) {
        ++expDiff;
    }
    else {
        aSig |= 0x40000000;
    }
    shift32RightJamming( aSig, - expDiff, &aSig );
    bSig |= 0x40000000;
bBigger:
    zSig = bSig - aSig;
    zExp = bExp;
    zSign ^= 1;
    goto normalizeRoundAndPack;
aExpBigger:
    if ( aExp == 0xFF ) {
        if ( aSig ) return propagateFloat32NaN( a, b );
        return a;
    }
    if ( bExp == 0 ) {
        --expDiff;
    }
    else {
        bSig |= 0x40000000;
    }
    shift32RightJamming( bSig, expDiff, &bSig );
    aSig |= 0x40000000;
aBigger:
    zSig = aSig - bSig;
    zExp = aExp;
normalizeRoundAndPack:
    --zExp;
    return normalizeRoundAndPackFloat32( zSign, zExp, zSig );
	
}

/*----------------------------------------------------------------------------
 | Returns the result of adding the single-precision floating-point values `a'
 | and `b'.  The operation is performed according to the IEC/IEEE Standard for
 | Binary Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/

float32 float32_add( float32 a, float32 b )
{
    flag aSign, bSign;
	
    aSign = extractFloat32Sign( a );
    bSign = extractFloat32Sign( b );
    if ( aSign == bSign ) {
        return addFloat32Sigs( a, b, aSign );
    }
    else {
        return subFloat32Sigs( a, b, aSign );
    }
	
}

/*----------------------------------------------------------------------------
 | Returns the result of subtracting the single-precision floating-point values
 | `a' and `b'.  The operation is performed according to the IEC/IEEE Standard
 | for Binary Floating-Point Arithmetic.
 *----------------------------------------------------------------------------*/

float32 float32_sub( float32 a, float32 b )
{
    flag aSign, bSign;
	
    aSign = extractFloat32Sign( a );
    bSign = extractFloat32Sign( b );
    if ( aSign == bSign ) {
        return subFloat32Sigs( a, b, aSign );
    }
    else {
        return addFloat32Sigs( a, b, aSign );
    }
	
}

//fully comment  expo() , digits(), and getfraction()

unsigned int expo(unsigned int a, unsigned int b)
{									//computes a ^ b for unsigned int
	unsigned int result = 1;
	while (b>0) 
	{
		result *= a;
		b--;
	}
	return result;	
}

unsigned int digits(unsigned int in) //Copyright 2014 Eugene Rockey
{									//computes the number of decimal digits in a decimal number
	unsigned int count = 0;
	if (in == 0) return 0;
	while (in != 0) 
	{
		in /= 10;
		count ++;
	}
	return count;
}

unsigned int getfraction(unsigned int shifts, unsigned int fleadzeros, unsigned int fraction) //Copyright 2014 Eugene Rockey
{									//computes the 23-bit mantissa or fraction of the IEEE-754 32-bit unsigned int
	unsigned int out = 0;
	unsigned int temp = 0;
	unsigned int places = digits(fraction) + fleadzeros;
	while (shifts != 0 && fraction != 0) 
	{
		temp = fraction * 2;
		if (digits(temp) > places)
		{
			out = out | (1 << shifts);
			fraction = temp - expo(10,places);
		}
		else 
		{
			fraction = temp;
		}
		shifts --;
	}
	return out;
}

unsigned int ASCII_to_float32(char* in) //Copyright 2014 Eugene Rockey
{									//converts ASCII decimal numbers to IEEE-754 32-bit unsigned int
	unsigned int out = 0, index = 0, integer = 0, fraction = 0, biasedexp = 0, shifts = 0, fleadzeros = 0, ibits = 0;
	
	if (in[index] == '-')
	{
		out = 0x80000000;
		index++;
	}
	
	while (in[index] != '.') 
	{
		integer = (integer + (in[index] - 48)) * 10;
		index ++;
	}
	integer /= 10;	
	index ++;
	while (in[index] == '0') 
	{
		fleadzeros ++;
		index ++;
	}
	
	while (in[index] != 0) 
	{
		fraction = (fraction + (in[index] - 48)) * 10;
		index ++;
	}
	
	fraction /= 10;	
	
	if (integer == 0 && fraction == 0)
	{
		return 0;
	}	
	
	if (integer == 0 && fraction != 0)
	{
		unsigned int temp = 0;
		unsigned int scount = 0;
		temp = getfraction(22, fleadzeros, fraction);
		while (temp < 0x80000000) 
		{
			temp = temp << 1;
			scount += 1;
		}
		temp = temp << 1;
		temp = temp >> 9;
		out = out | temp;
		scount = scount + 1 - 9;
		biasedexp = 0;
		biasedexp = (127 - scount) * 0x800000;
		return out | biasedexp;
	}	
	
	if (integer != 0 && fraction == 0)
	{
		while (integer < 0x400000) 
		{
			integer *= 2;
			shifts ++;
		}
		ibits = 23 - shifts;
		biasedexp = (ibits + 126) * 0x800000;
		out = out | biasedexp;
		integer *= 2;
		integer = integer & 0x007FFFFF;
		return out | integer;		
	}
	
	if (integer != 0 && fraction != 0 ) 
	{
		while (integer < 0x400000) 
		{
			integer *= 2;
			shifts ++;
		}
	ibits = 23 - shifts;
	biasedexp = (ibits + 126) * 0x800000;
	out = out | biasedexp;
	integer *= 2;
	integer = integer & 0x007FFFFF;
	out = out | integer;
	return out | getfraction(shifts, fleadzeros, fraction);
	}
	
	//Default
	return 0;
}


char* float32_to_ASCII(float32 r)
{
	return "Engineer and fully comment this function.";
}


