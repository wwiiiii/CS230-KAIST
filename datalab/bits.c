/* 
 * CS:APP Data Lab 
 * 
 * <Á¤ÅÂ¿µ p20150699>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitNor - ~(x|y) using only ~ and & 
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitNor(int x, int y) {
	
	return (~x) & (~y);  
  
}

/* 
 * copyLSB - set all bits of result to least significant bit of x
 *   Example: copyLSB(5) = 0xFFFFFFFF, copyLSB(6) = 0x00000000
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */ 
int copyLSB(int x) {

	//arithmetic shift
	return (x<<31)>>31;

}

/* 
 * isEqual - return 1 if x == y, and 0 otherwise 
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
	
	//if x == y => x^y is 0
    return !( (~(x^y)) + 1);

}

/* 
 * bitMask - Generate a mask consisting of all 1's 
 *   lowbit and highbit
 *   Examples: bitMask(5,3) = 0x38
 *   Assume 0 <= lowbit <= 31, and 0 <= highbit <= 31
 *   If lowbit > highbit, then mask should be all 0's
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int bitMask(int highbit, int lowbit) {

	//make right blank by shift 0xFFFFFFFF left
	//make left  blank by shift (1<<31)    right(use arithmetic shift)
	//then xor both!
	int rightBlanked = (~0)<<lowbit;
    int isHighGreater = (~((highbit+(~lowbit)+1)>>31));
    int leftFilled = ((1<<31)>>(32+(~highbit)))<<1;         //<<1's purpose : prevent shift of negative count
    return( rightBlanked ^ leftFilled) & isHighGreater;
  

}

/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
    int a1 = 0x55;     //01010101		  *4
    int a2 = 0x33;     //00110011         *4
    int a3 = 0x0F;     //00001111         *4
    int a4 = 0xFF;     //0000000011111111 *2
    int a5 = 0xFF;     //0000..001111..11
    a1 = a1 + (a1 << 8);
    a1 = a1 + (a1 << 16);
    a2 = a2 + (a2 << 8);
    a2 = a2 + (a2 << 16);
    a3 = a3 + (a3 << 8);
    a3 = a3 + (a3 << 16);
    a4 = a4 + (a4 << 16);
    a5 = a5 + (a5 << 8);
    x = (x & a1) + ( (x>>1)  &a1);
    x = (x & a2) + ( (x>>2)  &a2);
    x = (x & a3) + ( (x>>4)  &a3);
    x = (x & a4) + ( (x>>8)  &a4);
    x = (x & a5) + ( (x>>16) &a5);
  
    return x;

}

/* 
 * TMax - return maximum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
 
  return (1 << 31) ^ (~0) ;

}

/* 
 * isNonNegative - return 1 if x >= 0, return 0 otherwise 
 *   Example: isNonNegative(-1) = 0.  isNonNegative(0) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 3
 */
int isNonNegative(int x) {
	//arithmetic shift
	return (x >> 31) + 1;

}

/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
	/*
	 *	(!((x+y)>>31 +1))	if x,y>0 and overflow => 1 / else => 0
	 *	(!((x+y)>>31))		if x,y<0 and overflow => 1 / else => 0
	 *	no OverFlow when xy<=0
	 *	(!(x>>31 | y>>31))	if x,y>0 => 1		 / else => 0
	 *	(x>>31 & y>>31)		if x,y<0 => 0xffffff / else => 0
	 */
    int SumShifted = (x + y) >> 31;
    int xshifted = x>>31;
    int yshifted = y>>31;
	return !(((!(SumShifted + 1)) & (!(xshifted | yshifted))) | ((!SumShifted)&(xshifted & yshifted)));

}

/* 
 * rempwr2 - Compute x%(2^n), for 0 <= n <= 30
 *   Negative arguments should yield negative remainders
 *   Examples: rempwr2(15,2) = 3, rempwr2(-35,3) = -3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int rempwr2(int x, int n) {

   int ret, isNegPos;
   ret       = x + ( ~( (x>>n) << n) ) + 1;
   isNegPos  = (x>>31) & ( ~( (ret + (~0) ) >> 31) );   //x is minus, then ret should be minus or zero
   ret       = ret + ( ( 1 + (~(1<<n)) ) & isNegPos);	//if(x<0 and ret>0) make ret neg.
   return ret;
  
}

/* 
 * isLess - if x < y  then return 1, else return 0 
 *   Example: isLess(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLess(int x, int y) {

    //if   (x,y) != (+,-) 
    //and ((x,y) == (-,+) or x-y<0 )
    //no overflow when xy>=0
    int xshifted = x>>31;
    int yshifted = y>>31;
    int isPosNeg = (!xshifted) & yshifted;
    int isNegPos = xshifted & (!yshifted);
    int DirectCompare = ((x+(~y)+1)>>31);

    return (!isPosNeg) & (isNegPos | DirectCompare) & 1;
}

/* 
 * absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {

	//if x<0 => x>>31 == 0xffffffff
	int isNeg = x >> 31;
	int isPos = ~isNeg;
	int xRevSign = (~x) + 1;
	return ( (isPos&x) | (isNeg&(xRevSign) ) );
}

/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int isPower2(int x) { 

    //  x & (x-1) == 0 means x is consisted of only 1 
	//	!(x>>31|!x) is 1 only if x>0, else 0
	int isOnlyOneBit = !( x & +(x + (~0)) );
	int isPos        = !(x >> 31 | !x);
    return isOnlyOneBit & isPos;
}

/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
    
    if(0xFF000002<=(uf<<1)){if((uf<<1)<=0xFFFFFFFE)return uf;}   // if NaN, return uf
	return (uf + 0x80000000);                                    // makes msb reversed
    
}

/* 
 * float_half - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */

unsigned float_half(unsigned uf) {
 
    int exp, ret;
    if(0xFF000002<=(uf<<1)){if((uf<<1)<=0xFFFFFFFE)return uf;}	//case : NaN
    if((uf<<1) == 0xff000000) return uf;						//case : infinity

    exp = uf&(0x7fffffff);	//extract exp part from uf
    exp = exp>>23;
    if(exp <= 1) {			//have to handle frac part
        ret =  ( (uf>>1) & 0x3fffffff) + ( (uf>>31) <<31);
        return ret + (1 & uf & ret);//round to even
    }
    exp -= 1;	//divide by 2
    exp = exp<<23;
    uf = uf & 0x807fffff; //clear former exp part
    uf += exp;			  //fill with new exp part
    return uf;

}


/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floatine point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {

		int x_abs, retFracBackup, retFracDiff ,retFracPivot;
		int retExp =0, retFrac=0, retSign= 0;
		if(x==0) return 0;						//no msb
		if(x==0x80000000) return 0xcf000000;	//abs(0x80000000) is overflow
		if(x<0) 
		{
				x = -x;
				retSign = 0x80000000;
		}
		x_abs = x;
		while(x>=2)
		{
				retExp++; x = x>>1;
		} // 2^retExp <= x < 2^(retExp+1)
		retFrac = x_abs - (1<<retExp);	//
		if(retExp<=23)	//no need to consider rounding
		{
			retFrac = retFrac << (23-retExp);//set digit
			retFrac = retFrac & 0x7fffff;
		}
		else 
		{
			retFracBackup = retFrac;
			retFrac = retFrac >> (retExp-23);//set digit
			retFracDiff = retFracBackup - (retFrac << (retExp-23) );
			retFracPivot = 1<<(retExp-24);//pivot of decision of rounding
			retFrac = retFrac&0x7fffff;
			if (retFracDiff > retFracPivot) retFrac++; //vanished part > 0.5 => rounding up
			else if (retFracDiff == retFracPivot) retFrac += (retFrac&1);//vanished part == 0.5 => rounding to even
		}
		retExp+=127;//normalize
		return retSign + (retExp<<23) + (retFrac);

}




