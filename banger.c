#include "m_pd.h"

static char *version = "[banger] v.1 by Brian Voyer <brianvoyer@gmail.com>";

static t_class *banger_class;

typedef struct _banger
{
  t_object x_obj;
  int counter;
  float in1_last, in2_last;
  int out1max, out2max;
  t_inlet *inlet2;
  t_outlet *output1;
  t_outlet *output2;
} t_banger;

void *banger_new(void) 
{
    t_banger *x = (t_banger *)pd_new(banger_class);
    
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("in_one"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("in_two"));
    x->output1 = outlet_new(&x->x_obj, &s_float);
    x->output2 = outlet_new(&x->x_obj, &s_float);
    
    x->counter = 0;
    x->in1_last = 0;
    x->in2_last = 0;
    x->out1max = 0;
    x->out2max = 0;

    return (void *)x;
}

void makebang(t_banger *x)
{
  int temp = x->counter << 1;
  int in1 = (int)x->in1_last + (int)(temp * 0.5);
  int in2 = (int)x->in2_last + (int)(temp * 0.5);
  if(temp < 0)
    temp = ~temp;
  if((((in1 + in2) & in1) ^ in2) > ((in1 * in2) << 1))
    {
      x->out1max++;
      outlet_bang(x->output1);
      x->counter -= in1;
    }
  if(((in1 | in2) ^ temp) < ((in1 + in2) >> (temp % 2)))
    {
      x->out2max++;
      outlet_bang(x->output2);
      x->counter -= in2;
    }
  if(x->counter < -50)
    x->counter *= -1;
}

void banger_one(t_banger *x, t_floatarg f)
{
  x->counter = (x->counter + 1) % 256;
  x->in1_last = f;
  makebang(x);
};

void banger_two(t_banger *x, t_floatarg f)
{
  x->counter = (x->counter + 1) % 256;
  x->in2_last = f;
  makebang(x);
};

void banger_reset(t_banger *x)
{
  x->counter = 0;
}

void banger_setup(void) //sets up the actual class
{
  banger_class = class_new(gensym("banger"),(t_newmethod)banger_new, 0,
			   sizeof(t_banger), CLASS_DEFAULT, 0); 
  class_addbang(banger_class, banger_reset);
  class_addmethod(banger_class, (t_method)banger_one, gensym("in_one"), 
		  A_FLOAT, 0);
  class_addmethod(banger_class, (t_method)banger_two, gensym("in_two"), 
		  A_FLOAT, 0);
  post(version);
}

