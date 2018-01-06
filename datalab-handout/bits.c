/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
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
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  return ~((~x)|(~y));
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56(0x00000056)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  int temp = x >> (n<<3);
  temp = temp & 0xFF;
  return temp;

}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432//OxF8765432(算数右移)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int temp  = (1<<31);//0x80000000
  temp = ((temp>>n)<<1); //temp >> (n-1)
  temp = (~temp)&(x>>n);
  return temp;
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  //x = (x&0x55555555) + ((x>>1)&0x55555555); //2个相邻的位,相加,数值保存到这两二位bit.以此类推
  //x = (x&0x33333333) + ((x>>2)&0x33333333); 
  //x = (x&0x0f0f0f0f) + ((x>>4)&0x0f0f0f0f);
  //x = (x&0x00ff00ff) + ((x>>8)&0x00ff00ff);
  //x = (x&0x0000ffff) + ((x>>16)&0x0000ffff);
  int tmpMask1 = (0x55|0x55<<8);
  int mask1 = (tmpMask1 | (tmpMask1<<16));
  int tmpMask2 = (0x33|0x33<<8);
  int mask2 = (tmpMask2 | (tmpMask2<<16)); 
  int tmpMask3 = (0x0f|0x0f<<8);
  int mask3 = (tmpMask3 | (tmpMask3<<16));
  int mask4 = (0xff | 0xff<<16);
  int mask5 = (0xff | 0xff<<8);
  int count = 0;
  count = (x&mask1) + ((x>>1)&mask1);
  count = (count&mask2) + ((count>>2)&mask2);
  count = (count&mask3) + ((count>>4)&mask3);
  count = (count&mask4) + ((count>>8)&mask4);
  count = (count&mask5) + ((count>>16)&mask5);
  return count;
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  int tmp = ~x+1;//tmp=-x;
  tmp = x|tmp;
  tmp = tmp>>31;//0xffffffff 0x00000000
  return tmp+1;
}
/* 
 * tmin - return minimum two's complement integer //整数二进制补码
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  return 1<<31;//0xffffffff
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  int shiftNum = 32+(~n+1);//32-n
  x = x^((x<<shiftNum)>>shiftNum);//取后n位bits异或
  return !x;
}
/* 
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero(向零取舍)
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
    //x为正趋向零取舍,x为负数不然
    int sign = x>>31;
    //x为负,要加偏移量
    int mask = (1<<n) + (~0);//2^n-1(?)
    int bias = sign & mask;
    return (x+bias)>>n;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  return (~x)+1;
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  int sign = x>>31;
  return !(sign | (!x));//!((x >> 31) | (!x))
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int signx = (x>>31)&0x1; //0x0 or 0x1
  int signy = (y>>31)&0x1;
  int sing = signx&(signx^signy);     //保证异号
  int tmp = x+((~y)+1);
  tmp =((tmp>>31)&0x1)&(!(signx^signy));    //保证同号
  return sing | tmp | (!(x^y));
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4        即得到由多少位二进制表示即可
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  int bitsNum = 0;
  bitsNum = (!!(x>>16))<<4;
  bitsNum = bitsNum + ((!!(x>>(bitsNum+8)))<<3);
  bitsNum = bitsNum + ((!!(x>>(bitsNum+4)))<<2);
  bitsNum = bitsNum + ((!!(x>>(bitsNum+2)))<<1);
  bitsNum = bitsNum + (!!(x>>(bitsNum+1)));
  //for non zero bitsNumber, it should add 0
  //for zero bitsNumber, it should subtract 1
  //还要考虑x = 0;
  bitsNum = bitsNum + (!!bitsNum) + (~0) + !(1^x);
  return bitsNum;
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   返回和浮点数参数-f相等的二进制:计算-f,float(1+8+23)
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   参数和返回结果都是无符号整数，但是可以解释成单精度浮点数的二进制表示
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
 unsigned temp = uf&0x7fffffff; //符号位置0
 unsigned result = uf^0x80000000;//符号位取反
 if(temp>0x7f800000)//NAN
     result = uf;
 return result;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   返回int x的浮点数的二进制形式;不懂!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  unsigned shiftLeft = 0;
  unsigned afterShift, tmp, flag;
  unsigned abs_x = x;  //int-->unsigned
  unsigned sign = 0;
  if(x == 0)   //x=0时
      return 0;
  if(x<0)     //x<0,int补码形式表示的,float直接表示的
  {
    sign = 0x80000000;
    abs_x = -x;  //int-->unsigned
  }
  afterShift = abs_x;
  while(1)
  {
    tmp = afterShift; 
    afterShift<<=1;
    shiftLeft++;
    if(tmp & 0x80000000) break;
  }
  if((afterShift & 0x01ff)>0x0100)
      flag = 1;
  else if((afterShift & 0x03ff)==0x0300)
      flag = 1;
  else 
      flag = 0;
  return sign+(afterShift>>9)+((159-shiftLeft)<<23)+flag;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   返回 以unsinged表示的浮点数二进制的二倍的二进制unsigned型
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  int result = uf;
  if((result&0x7f800000)==0)//阶码全零
      result = (((result&0x007fffff)<<1) | (result&0x80000000)); //尾码左移一位,
  else if ((result&0x7f800000)!=0x7f800000) //阶码不全为零 
      result = result+0x00800000; //阶码加1
  return result;
}
