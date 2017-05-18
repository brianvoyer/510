/* Complex subtraction at audio rate */
#include "m_pd.h"  
#include "complex_help.h"

static t_class *isub_tilde_class;  

typedef struct _isub_tilde {  
  t_object  x_obj;
  t_sample f;
} t_isub_tilde;

t_int *isub_tilde_perform(t_int *w)  
{  
  t_isub_tilde *x = (t_isub_tilde *)(w[1]);  
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
      (*outR++) = cur1R - cur2R;
      (*outI++) = cur1I - cur2I;;
    }
  return (w+9);  
}  

void isub_tilde_dsp(t_isub_tilde *x, t_signal **sp)  
{
  dsp_add(isub_tilde_perform, 8, x,  
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, 
	  sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[0]->s_n);  
}  

void *isub_tilde_new(void)  
{  
  t_isub_tilde *x = (t_isub_tilde *)pd_new(isub_tilde_class);  

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  
  return (void *)x;  
}  

void isub_tilde_setup(void) {  
  isub_tilde_class = class_new(gensym("i-~"),  
			       (t_newmethod)isub_tilde_new,  
			       0,
			       sizeof(t_isub_tilde),  
			       CLASS_DEFAULT,  
			       0);  
			       
  class_addmethod(isub_tilde_class,(t_method)isub_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(isub_tilde_class, t_isub_tilde, f);  
  class_sethelpsymbol(isub_tilde_class, gensym("convlexlib"));
}
