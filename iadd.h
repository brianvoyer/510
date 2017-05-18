/* Complex addition at audio rate */
#include "m_pd.h"  
#include "complex_help.h"

static t_class *iadd_tilde_class;  

typedef struct _iadd_tilde {  
  t_object  x_obj;
  t_sample f;
} t_iadd_tilde;

t_int *iadd_tilde_perform(t_int *w)  
{  
  t_iadd_tilde *x = (t_iadd_tilde *)(w[1]);  
  t_sample   *in1R =      (t_sample *)(w[2]);
  t_sample   *in1I =      (t_sample *)(w[3]);
  t_sample   *in2R =      (t_sample *)(w[4]); 
  t_sample   *in2I =      (t_sample *)(w[5]); 
  t_sample   *outR =      (t_sample *)(w[6]);  
  t_sample   *outI =      (t_sample *)(w[7]);  
  int           n  =             (int)(w[8]);  
  t_sample cur1R, cur1I, cur2R, cur2I;
  
  while(n--)
    {
      cur1R = (*in1R++);
      cur1I = (*in1I++);
      cur2R = (*in2R++);
      cur2I = (*in2I++);
      (*outR++) = cur1R + cur2R;
      (*outI++) = cur1I + cur2I;;
    }
  return (w+9);  
}  

void iadd_tilde_dsp(t_iadd_tilde *x, t_signal **sp)  
{
  dsp_add(iadd_tilde_perform, 8, x,  
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, 
	  sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[0]->s_n);  
}  

void *iadd_tilde_new(void)  
{  
  t_iadd_tilde *x = (t_iadd_tilde *)pd_new(iadd_tilde_class);  
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  
  return (void *)x;  
}  

void iadd_tilde_setup(void) {  
  iadd_tilde_class = class_new(gensym("i+~"),  
			       (t_newmethod)iadd_tilde_new,  
			       0,
			       sizeof(t_iadd_tilde),  
			       CLASS_DEFAULT,  
			       0);  
			       
  class_addmethod(iadd_tilde_class,(t_method)iadd_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(iadd_tilde_class, t_iadd_tilde, f);  
  class_sethelpsymbol(iadd_tilde_class, gensym("convlexlib"));
}
