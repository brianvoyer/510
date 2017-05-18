/* 
   Convolution for reverb
   real-time vs non-real-time
   Non-functional as of May 17, 2017
*/
#include "m_pd.h"  
#include "complex_help.h"

#define MAX_NUM_BLOCKS 200

static t_class *cverb_tilde_class;  

typedef struct _cverb_tilde {  
  t_object  x_obj;
  t_sample f;

  t_sample *impulseR;
  t_sample *impulseI;
  int impulseLen; /* length of buffer (padded to block-lengths) */
  t_sample *bufferR; /* is of length impulseLen  */
  t_sample *bufferI;
  int numBlocksInBuffer; /* the number of block lengths in buffer */
  int blockLen; /* expected block length (length of buffer) */
  short recordFlag; /* Record flag */
  short impulseIsInit; /* Flag to ensure we don't try to read */
} t_cverb_tilde;

t_int *cverb_tilde_perform(t_int *w)  
{  
  t_cverb_tilde *x = (t_cverb_tilde *)(w[1]);  
  t_sample   *inR  =      (t_sample *)(w[2]);  
  t_sample   *inI  =      (t_sample *)(w[3]);
  t_sample   *outR =      (t_sample *)(w[4]);  
  t_sample   *outI =      (t_sample *)(w[5]);  
  int           n  =             (int)(w[6]);  
  t_sample curR, curI, tmpR, tmpI; 
  int i, ind;
  int N = n;

  if(x->recordFlag)
    { /* Record input into impulse buffer and extend other buffer */
      post("here >>> %d <<<", x->numBlocksInBuffer);
      
      x->numBlocksInBuffer++;
      if(x->numBlocksInBuffer >= MAX_NUM_BLOCKS)
	{
	  error("[cverb~]: HIT MAX NUM BLOCKS");
	  x->recordFlag = 0;
	}
      // reallocate buffers to correct size
      x->impulseLen += x->blockLen;
      x->impulseR = realloc(x->impulseR, x->impulseLen);
      x->impulseI = realloc(x->impulseI, x->impulseLen);
      x->bufferR = realloc(x->bufferR, x->impulseLen);
      x->bufferI = realloc(x->bufferI, x->impulseLen);
      if(x->impulseR == NULL || x->impulseI == NULL ||
	 x->bufferR  == NULL || x->bufferI  == NULL )
	{
	  error("[cverb~]: Reallocation Failed! Out of memory!");
	  error("Get that shit checked out! Please go to http://downloadmoreram.com.totallylegit.ru");
	  /* If I were nicer, I would cleanly free everything */
	}
      
      while(n--)
	{
	  //post("INSIDE %d", n);
	  curR = (*inR++);
	  curI = (*inI++);

	  ind = (x->impulseLen - x->blockLen) + (N-(n+1));
	  if(ind >= x->impulseLen)
	    {
	      error("Impulse Length:%d",x->impulseLen);
	      error("attempted index:%d",ind);
	    }
	  x->impulseR[ind] = curR;
	  x->impulseI[ind] = curI;

	  x->bufferR[ind] = 0;
	  x->bufferI[ind] = 0;
	  
	  (*outR++) = 0;
	  (*outI++) = 0;
	}
      x->impulseIsInit = 1;
    }
  else if(x->impulseIsInit)
    { /* Run convolution */
 
      while(n--)
	{
	  curR = (*inR++);
	  curI = (*inI++);
	  (*outR++) = curR;//0;
	  (*outI++) = curI;//0;
	}
      
      /*
      while (n--)
	{
	  curR = (*inR++);
	  curI = (*inI++);
	  
	  for( i=0; i<x->numBlocksInBuffer; i++ )
	    {
	      // complex mult against relative impulse pair
	      ind = i*N + (N-n);
	      tmpR = complexMult_r(curR, curI, x->impulseR[ind], x->impulseI[ind])
		+ x->bufferR[ind];
	      tmpI = complexMult_i(curR, curI, x->impulseR[ind], x->impulseI[ind])
		+ x->bufferI[ind];

	      
	      if( i==0 )
		{
		  // if i=0, then add to relative buffer and move to output
		  (*outR++) = tmpR;
		  (*outI++) = tmpI;
		}
	      else
		{
		  // else, add to relative buffer and move one block back in buffer
		  x->bufferR[ind - i*N] = tmpR;
		  x->bufferI[ind - i*N] = tmpI;
		}	      
	    }  
	}
      */
    }
  else
    { /* Disregard input, ouput 0s */
      while(n--)
	{
	  curR = (*inR++);
	  curI = (*inI++);
	  (*outR++) = curR;//0;
	  (*outI++) = curI;//0;
	}
    }

  return (w+7);  
}  

void cverb_tilde_dsp(t_cverb_tilde *x, t_signal **sp)  
{
  if(x->blockLen == 0)
    {
      x->blockLen = sp[0]->s_n;
    }
  else if(x->blockLen != sp[0]->s_n)
    {
      error("[cverb~]: Block size changed, please re-record impulse.");
      free(x->impulseR);
      free(x->impulseI);
      free(x->bufferR);
      free(x->bufferI);
      x->blockLen = sp[0]->s_n;
      x->impulseLen = 0;
      x->recordFlag = 0;
      x->impulseIsInit = 0;
      x->numBlocksInBuffer = 0;
    }
  
  dsp_add(cverb_tilde_perform, 6, x,  
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);  
}  

void cverb_tilde_record(t_cverb_tilde *x)
{
  //x->impulseIsInit = 0;
  x->numBlocksInBuffer = 0;
  x->recordFlag = 1;
  post("[cverb~]: Hit record!");
}

void cverb_tilde_stop(t_cverb_tilde *x)
{
  post("[cverb~]: Hit stop!");
  x->recordFlag = 0;
}


void cverb_tilde_free(t_cverb_tilde *x)  
{  
  if(x->impulseIsInit)
    {
      free(x->impulseR);
      free(x->impulseI);
      free(x->bufferR);
      free(x->bufferI);
    }
}  

void *cverb_tilde_new(void)  
{  
  t_cverb_tilde *x = (t_cverb_tilde *)pd_new(cverb_tilde_class);  

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  
  x->blockLen = 0;
  x->impulseLen = 0;
  x->recordFlag = 0;
  x->impulseIsInit = 0;
  
  return (void *)x;  
}  

void cverb_tilde_setup(void) {  
  cverb_tilde_class = class_new(gensym("cverb~"),  
			       (t_newmethod)cverb_tilde_new,  
			       (t_newmethod)cverb_tilde_free, 
			       sizeof(t_cverb_tilde),  
			       CLASS_DEFAULT,  
			       0);  
			       
  class_addmethod(cverb_tilde_class,(t_method)cverb_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(cverb_tilde_class,(t_method)cverb_tilde_record, gensym("record"), 0);
  class_addmethod(cverb_tilde_class,(t_method)cverb_tilde_stop, gensym("stop"), 0);
  CLASS_MAINSIGNALIN(cverb_tilde_class, t_cverb_tilde, f);  
}
