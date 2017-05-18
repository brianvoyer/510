#ifndef COMPLEX_HELP_H
#define COMPLEX_HELP_H

/* Complex Number Functions */
float complexMult_r(float a, float b, float c, float d)
{
  return a*c - b*d;
}

float complexMult_i(float a, float b, float c, float d)
{
  return a*d + b*c;
}

#endif /*COMPLEX_HELP_H*/
