#include "m_pd.h"

#define DEGTORAD 0.01745329

static t_class *amde_tilde_class;

static char *version = "[amde~] v.1: AMbisonic DEcoder";
static char *author  = "by Brian Voyer <brianvoyer@gmail.com>";

typedef struct _amde_tilde {
  t_object  x_obj;
  t_sample f;

  t_inlet *x_in2;
  t_inlet *x_in3;
  t_inlet *x_in4;
  t_outlet*x_out1;
  t_outlet*x_out2;
} t_amde_tilde;

t_int *amde_tilde_perform(t_int *w)
{
  t_amde_tilde *x = (t_amde_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]); // W
  t_sample *in2 = (t_sample *)(w[3]); // X
  t_sample *in3 = (t_sample *)(w[4]); // Y
  t_sample *in4 = (t_sample *)(w[5]); // Z
  t_sample *out1 = (t_sample *)(w[6]);
  t_sample *out2 = (t_sample *)(w[7]);
  int n = (int)(w[8]);
  t_sample sample1, sample2, sample3, sample4;

  while (n--)
    {
      sample1 = *in1++; // W
      sample2 = *in2++; // X
      sample3 = *in3++; // Y
      sample4 = *in4++; // Z

      // basic decoder
      *out1++ = sample1 * 0.707 + sample3*0.5;
      *out2++ = sample2 * 0.707 + sample3*0.5;
    }

  return (w+9);
}

void amde_tilde_dsp(t_amde_tilde *x, t_signal **sp)
{
  dsp_add(amde_tilde_perform, 8, x, sp[0]->s_vec, sp[1]->s_vec,
          sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec,
          sp[0]->s_n);
}

void amde_tilde_free(t_amde_tilde *x)
{
  inlet_free(x->x_in2);
  inlet_free(x->x_in3);
  inlet_free(x->x_in4);
  outlet_free(x->x_out1);
  outlet_free(x->x_out2);
}

void *amde_tilde_new(void)
{
  t_amde_tilde *x = (t_amde_tilde *)pd_new(amde_tilde_class);

  x->x_in2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->x_in3=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->x_in4=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  x->x_out1=outlet_new(&x->x_obj, &s_signal);
  x->x_out2=outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void amde_tilde_setup(void) {
  amde_tilde_class = class_new(gensym("amde~"),
                               (t_newmethod)amde_tilde_new,
                               (t_newmethod)amde_tilde_free,
                               sizeof(t_amde_tilde),
                               CLASS_DEFAULT, 0);

  class_addmethod(amde_tilde_class,
                  (t_method)amde_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(amde_tilde_class, t_amde_tilde, f);
  post(version);
  post(author);
}
