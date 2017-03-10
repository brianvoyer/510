/*
 * holylib, an ambisonics library for PD
 */

#include "m_pd.h"

#define DEGTORAD 0.01745329

static char *version = "holylib v. 0.1: Ambisonics Library";
static char *author  = "(c) Brian Voyer 2017 <brianvoyer@gmail.com>";

// [amen~] AMbisonic ENcoder
#include "holylib-amen.h"

// [deamen~] - ambisonic decoder
#include "holylib-deamen.h"

void holylib_setup(void)
{
  amen_tilde_setup();
  deamen_tilde_setup();
  post(version);
  post(author);
}
