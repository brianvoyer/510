/* 
   Complex Arithmetic and Convolution Library
   By David Estes-Smargiassi and Brian Voyer
*/


#include "imult.h"
#include "idiv.h"
#include "iadd.h" 
#include "isub.h" 
// #include "cverb.h" /* Unfinished and non-functional as of May 17, 2017 */
#include "carith.c" 

void convlexlib_setup(void)
{

  imult_tilde_setup();
  idiv_tilde_setup();
  iadd_tilde_setup();
  isub_tilde_setup();
  //cverb_tilde_setup();
  carith_setup();

  
  post("convlexlib, the convolution and complex arithmetic library");
  post("v1.0 Copyright 2017 David Estes-Smargiassi and Brian Voyer");
}
