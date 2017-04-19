#include "m_pd.h"
#include <math.h>

static t_class *cleaner_tilde_class;

static char *version = "[cleaner~] v.1 by Brian Voyer <brianvoyer@gmail.com>";

typedef struct cleaner_tilde {
  t_object x_obj;

  t_sample f;

  float *mag;
  float *phase;
  long vecsize;
} t_cleaner_tilde;

t_int *cleaner_tilde_perform(t_int *w)
{
  t_cleaner_tilde *x = (t_cleaner_tilde *) (w[1]);
  t_sample *real_in = (t_sample *) (w[2]);
  t_sample *imag_in = (t_sample *) (w[3]);
  t_sample *threshmult = (t_sample *)(w[4]);
  t_sample *multiplier = (t_sample *)(w[5]);
  t_sample *real_out = (t_sample *)(w[6]);
  t_sample *imag_out = (t_sample *)(w[7]);
  int n = (int)(w[8]) / 2; // block size

  float *phase = x->phase;
  float *mag = x->mag;
  
  float maxamp = 0.0;
  float threshold;
  float mult;
  int i;
  float a, b; // variables for polar/cartesian conversion
  
  
  // convert from polar to complex
  for(i = 0; i <= n; i++)
    {
      a = (i == n ? real_in[1] : real_in[i]);
      b = (i == 0 || i == n ? 0.0 : imag_in[i]);
      mag[i] = hypot(a, b);
      phase[i] = -atan2(b, a);
    }

  for(i = 0; i < n; i++)
    {
      if(maxamp < mag[i])
	maxamp = mag[i];
    }
  threshold = *threshmult * maxamp;
  mult = *multiplier;
  
  for(i = 0; i < n; i++)
    {
      if(mag[i] < threshold)
	mag[i] *= mult;
    }
  // reconvert from polar to complex
  for(i = 0; i <= n; i++)
    {
      real_out[i] = mag[i] * cos(phase[i]);
      if(i != n)
	imag_out[i] = -mag[i] * sin(phase[i]);
      else
	imag_out[i] = 0.0;
    }
  
  return (w+9);
}

void cleaner_tilde_dsp(t_cleaner_tilde *x, t_signal **sp)
{
  long size;

  if(x->vecsize != sp[0]->s_n)
    {
      x->vecsize = sp[0]->s_n;
      size = x->vecsize * sizeof(float);
      if(x->mag = NULL)
	{
	  x->phase = (float *) malloc(size);
	  x->mag =  (float *) malloc(size);
	}
      else
	{
	  free(x->phase);
	  free(x->mag);
	  x->phase = (float *) malloc(size);
	  x->mag =  (float *) malloc(size);
	}
    }

  dsp_add(cleaner_tilde_perform, 8, x, sp[0]->s_vec, sp[1]->s_vec, 
	  sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[0]->s_n);
}

void *cleaner_tilde_new(t_floatarg f)
{
  t_cleaner_tilde *x = (t_cleaner_tilde *)pd_new(cleaner_tilde_class);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);

  x->vecsize = 0;
  x->mag = NULL;

  return (void *)x;
}

void cleaner_tilde_free(t_cleaner_tilde *x)
{
  if(x->mag != NULL)
    {
      free(x->mag);
      free(x->phase);
    }
}

void cleaner_tilde_setup(void)
{
  cleaner_tilde_class = class_new(gensym("cleaner~"), 
				  (t_newmethod)cleaner_tilde_new,
				  (t_newmethod)cleaner_tilde_free,
				  sizeof(t_cleaner_tilde), 
				  CLASS_DEFAULT, A_DEFFLOAT, 0); 
  class_addmethod(cleaner_tilde_class, (t_method)cleaner_tilde_dsp, 
		  gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(cleaner_tilde_class, t_cleaner_tilde, f);
  post(version);
}
