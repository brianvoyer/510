#include "m_pd.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define DEFAULT_DEC 2

/* Complex Number Functions */
float complexMult_r(float a, float b, float c, float d)
{
  return a*c - b*d;
}

float complexMult_i(float a, float b, float c, float d)
{
  return a*d + b*c;
}

float complexGetPhase(float a, float b)
{
  if(0 == a && 0 == b)
    return 0;
  else
    return atan2(b, a);
}

float complexGetAmp(float a, float b)
{
  return a*a + b*b;
}

char *version = "[groan~] by brian voyer <brianvoyer@gmail.com>";

static t_class *groan_tilde_class;

typedef struct _groan_tilde {
  t_object x_obj;
  t_sample f;
  int block_size;
  int numdec; // only integer multiples, just for convenience
  float phase;
  short cool; // makes it do cool stuff
} t_groan_tilde;

t_int *groan_tilde_perform(t_int *w)
{
  t_groan_tilde *x = (t_groan_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]);
  t_sample *in2 = (t_sample *)(w[3]);
  t_sample *out1 = (t_sample *)(w[4]);
  t_sample *out2 = (t_sample *)(w[5]);
  int n = (int)(w[6]);
  int i;
  float conv_r;
  float conv_i;
  float phase;
  t_sample in_i, in_r;

  for(i = 0; i < x->numdec; i++)
    {
      (*in1++);
      (*in2++);
      n--;
    }
  while (n--)
    {
      in_i = (*in2++);
      in_r = (*in1++);
      
      // randomize phase
      phase = x->phase * M_PI * 0.00555555555;
      //((float)rand() / (float)RAND_MAX) * 0.125 + PI*0.5;
      if(x->cool)
	{
	  conv_r = complexMult_r(in_r, in_i, cos(phase), sin(phase));
	  conv_i = complexMult_i(in_r, in_i, sin(phase), cos(phase));
	}      
      else
	{
	  conv_r = complexMult_r(in_r, in_i, cos(phase), sin(phase));
	  conv_i = complexMult_i(in_r, in_i, cos(phase), sin(phase));
	}      

      (*out1++) = conv_r;// * x->window[n];
      (*out2++) = conv_i;// * x->window[n];
    }
  for(i = 0; i < x->numdec; i++)
    {
      (*out1++) = 0;
      (*out2++) = 0;
    }
	
  return (w+7);
}

void groan_tilde_dsp(t_groan_tilde *x, t_signal **sp)
{
  dsp_add(groan_tilde_perform, 6, x, sp[0]->s_vec, sp[1]->s_vec,
          sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

void groan_tilde_pitch(t_groan_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      x->numdec = (int) atom_getfloat(&argv[0]);
      post("[groan~]: downtuning by %d", x->numdec);
    }
}

void groan_tilde_phase(t_groan_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc)
    {
      x->phase = atom_getfloat(&argv[0]);
      post("[groan~]: rephasing by %.2f", x->phase);
    }
}

void *groan_tilde_new(t_atom *s, int argc, t_atom *argv)
{
  post(version);
  t_groan_tilde *x = (t_groan_tilde *)pd_new(groan_tilde_class);
  srand(time(NULL));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  if(argc)
    x->cool = 1;
  else
    x->cool = 0;
  x->numdec = DEFAULT_DEC;
  x->phase = 0;
  
  return (void *)x;
}

void groan_tilde_setup(void)
{
  groan_tilde_class = class_new(gensym("groan~"),
				(t_newmethod)groan_tilde_new, 0,
				sizeof(t_groan_tilde),
				CLASS_DEFAULT, A_GIMME, 0);
  class_addmethod(groan_tilde_class,
                  (t_method)groan_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(groan_tilde_class,
                  (t_method)groan_tilde_pitch, gensym("downtune"), A_GIMME, 0);
  class_addmethod(groan_tilde_class,
                  (t_method)groan_tilde_phase, gensym("phase"), A_GIMME, 0);
  CLASS_MAINSIGNALIN(groan_tilde_class, t_groan_tilde, f);
}
