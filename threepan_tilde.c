#include "m_pd.h"
#include <math.h>
#include <string.h>

#define PI 3.14159265

static t_class *threepan_tilde_class;

static char *version = "[threepan~] v.1 by Brian Voyer <brianvoyer@gmail.com>";

typedef struct threepan_tilde {
  t_object x_obj;

  t_sample f_pan;
  t_sample f;

  short pan_mode; //0 is linear, 1 is power, 2 is cosine

  t_inlet *x_in2;
  t_outlet *x_out1;
  t_outlet *x_out2;
} t_threepan_tilde;

void threepan_tilde_update(t_threepan_tilde *x, t_symbol *s, int argc, 
			   t_atom *argv)
{
  char temp[MAXPDSTRING];
  int foo = x->pan_mode;
  t_atom *bar = &argv[0];
  atom_string(bar, temp, sizeof(temp));
  if(strcmp(temp, "0") == 0 || strcmp(temp, "lin") == 0)
    {
      x->pan_mode = 0;
      post("Performing linear panning.");
    }  
  else if(strcmp(temp, "1") == 0 || strcmp(temp, "pow") == 0)
    {
      x->pan_mode = 1;
      post("Performing equal power panning.");
    }  
  else if(strcmp(temp, "2") == 0 || strcmp(temp, "cos") == 0)
    {
      x->pan_mode = 2;
      post("Performing cosine panning.");
    }  
  else
    error("Invalid mode, mode unchanged");
}

t_int *threepan_tilde_perform(t_int *w)
{
  t_threepan_tilde *x = (t_threepan_tilde *)(w[1]);
  t_sample *into = (t_sample *)(w[2]);
  t_sample *out1 = (t_sample *)(w[3]);
  t_sample *out2 = (t_sample *)(w[4]);
  int n = (int)(w[5]); // block size
  
  t_sample f_pan, in;

  if(x->f_pan < 0)
    f_pan = 0.0;
  else if(x->f_pan > 1)
    f_pan = 1.0;
  else
    f_pan = x->f_pan;
  
  while (n--) 
    {
      in = *into++;
      if(x->pan_mode == 0)
	{
	  // linear panning
	  *out1++ = in * (1-f_pan);
	  *out2++ = in * f_pan;
	}
      else if(x->pan_mode == 1)
	{
	  // equal power panning
	  *out1++ = in * sqrt(1-f_pan);
	  *out2++ = in * sqrt(f_pan);
	}
      else if(x->pan_mode == 2)
	{
	  // cosine panning
	  *out1++ = in * cos(2 * PI * (0.25 * f_pan - 0.5));
	  *out2++ = in * cos(2 * PI * (0.25 * f_pan - 0.25));
	}
    }

  return (w+6);
}

void threepan_tilde_dsp(t_threepan_tilde *x, t_signal **sp)
{
  // black magic follows as far as I can tell
  dsp_add(threepan_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void threepan_tilde_free(t_threepan_tilde *x)
{
  inlet_free(x->x_in2);
  outlet_free(x->x_out1);
  outlet_free(x->x_out2);
}

void *threepan_tilde_new(t_floatarg f)
{
  t_threepan_tilde *x = (t_threepan_tilde *)pd_new(threepan_tilde_class);

  x->f_pan = f;

  x->pan_mode = 0;

  floatinlet_new(&x->x_obj, &x->f_pan);

  x->x_out1 = outlet_new(&x->x_obj, &s_signal);
  x->x_out2 = outlet_new(&x->x_obj, &s_signal);
  
  return (void *)x;
}

void threepan_tilde_setup(void)
{
  threepan_tilde_class = class_new(gensym("threepan~"), 
				   (t_newmethod)threepan_tilde_new, 
				   threepan_tilde_free, 
				   sizeof(t_threepan_tilde), 
				   CLASS_DEFAULT, A_DEFFLOAT, 0); 
  class_addmethod(threepan_tilde_class, (t_method)threepan_tilde_dsp, 
		  gensym("dsp"), 0);
  class_addmethod(threepan_tilde_class, threepan_tilde_update, gensym("mode"),
		  A_GIMME, 0);
  CLASS_MAINSIGNALIN(threepan_tilde_class, t_threepan_tilde, f);
  post(version);
}
