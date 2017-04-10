/*
 * holylib, an ambisonics library for PD
 *
 * - an ambisonic encoder capable of 1st through 3rd order encoding
 *
 * - include the ability to apply in-phase optimization coefficients
 *
 * - an ambisonic decoder configured to support at least three 2D/3D
 *   loudspeaker configurations of your choice
 *
 * - adhere to IEM / Pd Distribution conventions when packaging your
 *   final submission prior to compression
 *
 * Bonus points: 
 *
 * - include a message-rate external to generate a list optimization
 *   coefficients for in-phase
 *
 * - include a message-rate external to generate a list of
 *   coefficients for inputted speaker angles
 */

#include "m_pd.h"
#include <string.h>
#include <stdlib.h>

#define DEGTORAD 0.017453292519943295769236907684886127134428718885417254560
// pi / 180

#define RTWOTWO 0.707106781186547524400844362104849039284835937688474036588
// sqrt(2)/2

long long factorial(int input)
{
  // you may ask why I chose to declare this as an array rather than doing the
  // factorial itself programatically. It boils down to three reasons: 
  // a)  computing the factorial of an arbitrary number is processor-intensive,
  //     and table lookups are O(1)
  // b)  only up to 20! fits in long longs, so it was not difficult to 
  //     hard-code these
  // c)  every other implementation I tried failed for some reason at 13!
  long long factorialarray[21] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880,
				  3628800, 39916800, 479001600, 6227020800, 
				  87178291200, 1307674368000, 20922789888000, 
				  355687428096000, 6402373705728000, 
				  121645100408832000, 2432902008176640000};
  if(input < 0)
    {
      error("Cannot take factorial of negative number");
      return 1;
    }
  else if(input > 21)
    {
      error("Integer overflow");
      return 1;
    }
  else
    return factorialarray[input];
}

static char *version = "holylib v. 1.0: \"(un)holy\" Ambisonics Library";
static char *author  = "(c) Brian Voyer 2017 <brianvoyer@gmail.com>";

// [amen~] AMbisonic ENcoder
#include "holylib-amen.h"

// [deamen~] - ambisonic decoder (general version)
//#include "holylib-deamen.h"

// [imp~] - ambisonic decoder (weaker version)
#include "holylib-imp.h"

// [lucifer] - Location CoeFficient generatoR
//#include "holylib-lucifer.h"
// deprecated -- included in [oni]

// [oni] - Optimized coefficient geNeratIon
#include "holylib-oni.h"

void holylib_setup(void)
{
  amen_tilde_setup();
  //  deamen_tilde_setup();
  imp_tilde_setup();
  //  lucifer_setup();
  oni_setup();
  post(version);
  post(author);
}
