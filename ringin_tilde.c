#include "m_pd.h"

static t_class *ringin_tilde_class;

typedef struct _ringin_tilde {
  t_object  x_obj;
  t_sample f;

  t_inlet *x_in2;
  t_outlet*x_out;
} t_ringin_tilde;

t_int *ringin_tilde_perform(t_int *w)
{
  t_ringin_tilde *x = (t_ringin_tilde *)(w[1]);
  t_sample  *in1 =    (t_sample *)(w[2]);
  t_sample  *in2 =    (t_sample *)(w[3]);
  t_sample  *out =    (t_sample *)(w[4]);
  int          n =           (int)(w[5]);

  while (n--) *out++ = (*in1++)*(*in2++);

  return (w+6);
}

void ringin_tilde_dsp(t_ringin_tilde *x, t_signal **sp)
{
  dsp_add(ringin_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void ringin_tilde_free(t_ringin_tilde *x)
{
  inlet_free(x->x_in2);
  outlet_free(x->x_out);
}

void *ringin_tilde_new(void)
{
  t_ringin_tilde *x = (t_ringin_tilde *)pd_new(ringin_tilde_class);

  x->x_in2=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->x_out=outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void ringin_tilde_setup(void) {
  ringin_tilde_class = class_new(gensym("ringin~"),
                              (t_newmethod)ringin_tilde_new,
                              0, sizeof(t_ringin_tilde),
                              CLASS_DEFAULT, 0);

  class_addmethod(ringin_tilde_class,
                  (t_method)ringin_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(ringin_tilde_class, t_ringin_tilde, f);
}
