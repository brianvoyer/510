#include "m_pd.h"

#define DEGTORAD 0.01745329

static t_class *amen_tilde_class;

static char *version = "[amen~] v.1: AMbisonic ENcoder";
static char *author  = "by Brian Voyer <brianvoyer@gmail.com>";

typedef struct _amen_tilde
{
  t_object  x_obj;
  t_sample f;

  t_inlet *x_in2;
  t_inlet *x_in3;
  t_outlet *x_out1;
  t_outlet *x_out2;
  t_outlet *x_out3;
  t_outlet *x_out4;
} t_amen_tilde;

t_int *amen_tilde_perform(t_int *w)
{
  t_amen_tilde *x = (t_amen_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]); // audio signal
  t_sample *in2 = (t_sample *)(w[3]); // azimuth
  t_sample *in3 = (t_sample *)(w[4]); // elevation
  t_sample *out1 = (t_sample *)(w[5]);
  t_sample *out2 = (t_sample *)(w[6]);
  t_sample *out3 = (t_sample *)(w[7]);
  t_sample *out4 = (t_sample *)(w[8]);
  int n = (int)(w[9]);

  t_sample sample1, sample2, sample3;

  float kPl = 2; // black magic

  while (n--)
    {
      sample1 = *in1++;
      sample2 = *in2++ * DEGTORAD;
      sample3 = (*in3++ - 0.5) * kPl;

      *out1++ = sample1 * 0.707; // W, sqrt(2)/2
      *out2++ = sample1 * cosf(sample2); // X
      *out3++ = sample1 * sinf(sample2); // Y
      *out4++ = sample1 * sinf(sample3); // Z
    }

  return (w+10);
}

void amen_tilde_dsp(t_amen_tilde *x, t_signal **sp)
{
  dsp_add(amen_tilde_perform, 9, x, sp[0]->s_vec, sp[1]->s_vec,
          sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec,
          sp[6]->s_vec, sp[0]->s_n);
}

void amen_tilde_free(t_amen_tilde *x)
{
  inlet_free(x->x_in2);
  inlet_free(x->x_in3);
  outlet_free(x->x_out1);
  outlet_free(x->x_out2);
  outlet_free(x->x_out3);
  outlet_free(x->x_out4);
}

void *amen_tilde_new(void)
{
  t_amen_tilde *x = (t_amen_tilde *)pd_new(amen_tilde_class);

  x->x_in2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->x_in3=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);

  x->x_out1=outlet_new(&x->x_obj, &s_signal);
  x->x_out2=outlet_new(&x->x_obj, &s_signal);
  x->x_out3=outlet_new(&x->x_obj, &s_signal);
  x->x_out4=outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void amen_tilde_setup(void)
{
  amen_tilde_class = class_new(gensym("amen~"),
                               (t_newmethod)amen_tilde_new,
                               (t_newmethod)amen_tilde_free,
                               sizeof(t_amen_tilde),
                               CLASS_DEFAULT, 0);

  class_addmethod(amen_tilde_class,
                  (t_method)amen_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(amen_tilde_class, t_amen_tilde, f);
  post(version);
  post(author);
}
