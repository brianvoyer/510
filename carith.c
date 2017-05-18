/* complex arithmetic: do complex arithmetic on message rate data *
 * using info from x_arithmetic.c in the PD vanilla source        */

#include "m_pd.h"
#include <stdio.h>
#include <math.h>

typedef struct _carbin
{
  t_object x_obj;
  t_float x_r1, x_r2;
  t_float x_i1, x_i2;
  t_outlet *x_out2;
} t_carbin;

//:  +, -, *, / ----------------------------- */

static void *carbin_new(t_class *floatclass)
{
  t_carbin *x = (t_carbin *)pd_new(floatclass);
  outlet_new(&x->x_obj, &s_float);
  x->x_out2 = outlet_new(&x->x_obj, &s_float);
  floatinlet_new(&x->x_obj, &x->x_i1);
  floatinlet_new(&x->x_obj, &x->x_r2);
  floatinlet_new(&x->x_obj, &x->x_i2);
  x->x_r1 = 0;
  x->x_i1 = 0;
  x->x_r2 = 0;
  x->x_i2 = 0;
  return (x);
}

void *carbin_free(t_carbin *x)
{
  free(x->x_out2);
}

/* --------------------- addition ------------------------------- */

static t_class *carbin_plus_class;

static void *carbin_plus_new()
{
  return (carbin_new(carbin_plus_class));
}

static void carbin_plus_bang(t_carbin *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_r1 + x->x_r2);
  outlet_float(x->x_out2, x->x_i1 + x->x_i2);
}

static void carbin_plus_float(t_carbin *x, t_float f)
{
  x->x_r1 = f;
  outlet_float(x->x_obj.ob_outlet, x->x_r1 + x->x_r2);
  outlet_float(x->x_out2, x->x_i1 + x->x_i2);
}

/* --------------------- subtraction ------------------------------- */

static t_class *carbin_minus_class;

static void *carbin_minus_new()
{
  return (carbin_new(carbin_minus_class));
}

static void carbin_minus_bang(t_carbin *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_r1 - x->x_r2);
  outlet_float(x->x_out2, x->x_i1 - x->x_i2);
}

static void carbin_minus_float(t_carbin *x, t_float f)
{
  x->x_r1 = f;
  outlet_float(x->x_obj.ob_outlet, x->x_r1 - x->x_r2);
  outlet_float(x->x_out2, x->x_i1 - x->x_i2);
}

/* --------------------- multiplication ------------------------------- */

static t_class *carbin_times_class;

static void *carbin_times_new()
{
  return (carbin_new(carbin_times_class));
}

static void carbin_times_bang(t_carbin *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_r1 * x->x_r2 - x->x_i1 * x->x_i2);
  outlet_float(x->x_out2, x->x_r1 * x->x_i2 + x->x_i1 * x->x_r2);
}

static void carbin_times_float(t_carbin *x, t_float f)
{
  x->x_r1 = f;
  outlet_float(x->x_obj.ob_outlet, x->x_r1 * x->x_r2 - x->x_i1 * x->x_i2);
  outlet_float(x->x_out2, x->x_r1 * x->x_i2 + x->x_i1 * x->x_r2);
}

/* --------------------- division ------------------------------- */

static t_class *carbin_div_class;

static void *carbin_div_new()
{
  return (carbin_new(carbin_div_class));
}

static void carbin_div_bang(t_carbin *x)
{
  if(x->x_r2 == 0 && x->x_i2 == 0)
    {
      outlet_float(x->x_obj.ob_outlet, 0);
      outlet_float(x->x_out2, 0);
      error("[i/]: Cannot divide by 0");
    }
  else
    {
      float tmp = 1/(x->x_r2 * x->x_r2 + x->x_i2 * x->x_i2);
      outlet_float(x->x_obj.ob_outlet, 
		   (x->x_r1 * x->x_r2 + x->x_i1 * x->x_i2) * tmp);
      outlet_float(x->x_out2, (x->x_i1 * x->x_r2 - x->x_r1 * x->x_i2) * tmp);
    }
}

static void carbin_div_float(t_carbin *x, t_float f)
{
  x->x_r1 = f;
  if(x->x_r2 == 0 && x->x_i2 == 0)
    {
      outlet_float(x->x_obj.ob_outlet, 0);
      outlet_float(x->x_out2, 0);
      error("[i/]: Cannot divide by 0");
    }
  else
    {
      float tmp = 1/((x->x_r2 * x->x_r2) + (x->x_i2 * x->x_i2));
      outlet_float(x->x_obj.ob_outlet, 
		   (x->x_r1 * x->x_r2 + x->x_i1 * x->x_i2) * tmp);
      outlet_float(x->x_out2, (x->x_i1 * x->x_r2 - x->x_r1 * x->x_i2) * tmp);
    }
}

void carith_setup(void)
{
  t_symbol *carbin_sym = gensym("carith");
  
  carbin_plus_class = class_new(gensym("i+"), 
				(t_newmethod)carbin_plus_new, 0,
				sizeof(t_carbin), CLASS_DEFAULT, 0);
  class_addbang(carbin_plus_class, carbin_plus_bang);
  class_addfloat(carbin_plus_class, (t_method)carbin_plus_float);
  class_sethelpsymbol(carbin_plus_class, carbin_sym);
  
  carbin_minus_class = class_new(gensym("i-"),
				 (t_newmethod)carbin_minus_new, 0,
				 sizeof(t_carbin), CLASS_DEFAULT, 0);
  class_addbang(carbin_minus_class, carbin_minus_bang);
  class_addfloat(carbin_minus_class, (t_method)carbin_minus_float);
  class_sethelpsymbol(carbin_minus_class, carbin_sym);
  
  carbin_times_class = class_new(gensym("i*"),
				 (t_newmethod)carbin_times_new, 0,
				 sizeof(t_carbin), CLASS_DEFAULT, 0);
  class_addbang(carbin_times_class, carbin_times_bang);
  class_addfloat(carbin_times_class, (t_method)carbin_times_float);
  class_sethelpsymbol(carbin_times_class, carbin_sym);
  
  carbin_div_class = class_new(gensym("i/"),
			       (t_newmethod)carbin_div_new, 0,
			       sizeof(t_carbin), CLASS_DEFAULT, 0);
  class_addbang(carbin_div_class, carbin_div_bang);
  class_addfloat(carbin_div_class, (t_method)carbin_div_float);
  class_sethelpsymbol(carbin_div_class, carbin_sym);
}
