/* Complex multiplication at audio rate */
#include "m_pd.h"  
#include "complex_help.h"

static t_class *imult_tilde_class;  

typedef struct _imult_tilde {  
  t_object  x_obj;
  t_sample f;
} t_imult_tilde;

t_int *imult_tilde_perform(t_int *w)  
{  
  t_imult_tilde *x = (t_imult_tilde *)(w[1]);  
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
      (*outR++) = cur1R * cur2R - cur1I * cur2I;
      (*outI++) = cur1R * cur2I + cur1I * cur2R;
    }
  return (w+9);  
}  

void imult_tilde_dsp(t_imult_tilde *x, t_signal **sp)  
{
  dsp_add(imult_tilde_perform, 8, x,  
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, 
	  sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, sp[0]->s_n);  
}  

void *imult_tilde_new(void)  
{  
  t_imult_tilde *x = (t_imult_tilde *)pd_new(imult_tilde_class);  
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  
  return (void *)x;  
}  

void imult_tilde_setup(void) {  
  imult_tilde_class = class_new(gensym("i*~"),  
			       (t_newmethod)imult_tilde_new,  
			       0,
			       sizeof(t_imult_tilde),  
			       CLASS_DEFAULT,  
			       0);  
			       
  class_addmethod(imult_tilde_class,(t_method)imult_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(imult_tilde_class, t_imult_tilde, f);  
  class_sethelpsymbol(imult_tilde_class, gensym("convlexlib"));
}
