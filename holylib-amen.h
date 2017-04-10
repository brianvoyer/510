// [amen~] AMbisonic ENcoder 
//for holylib (c) 2017 Brian Voyer

// ignoring elevation--assume listener is always on the same horizontal plane
// as the speakers,

static t_class *amen_tilde_class;

typedef struct _amen_tilde
{
  t_object  x_obj;
  t_sample f;

  t_inlet *x_in2;
  t_outlet *x_out1;
  t_outlet *x_out2;
  t_outlet *x_out3;
  t_outlet *x_out4;
  t_outlet *x_out5;
  t_outlet *x_out6;
  t_outlet *x_out7;

  int order;
} t_amen_tilde;

t_int *amen_tilde_perform1(t_int *w)
{
  t_amen_tilde *x = (t_amen_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]); // audio signal
  t_sample *in2 = (t_sample *)(w[3]); // azimuth
  t_sample *out1 = (t_sample *)(w[4]);
  t_sample *out2 = (t_sample *)(w[5]);
  t_sample *out3 = (t_sample *)(w[6]);
  int n = (int)(w[7]);

  t_sample sample1, sample2;

  while (n--)
    {
      sample1 = *in1++;
      sample2 = *in2++ * DEGTORAD;

      *out1++ = sample1 * 0.707; // W, sqrt(2)/2
      *out2++ = sample1 * cosf(sample2); // Y
      *out3++ = sample1 * sinf(sample2); // X
    }

  return (w+8);
}

t_int *amen_tilde_perform2(t_int *w)
{
  t_amen_tilde *x = (t_amen_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]); // audio signal
  t_sample *in2 = (t_sample *)(w[3]); // azimuth
  t_sample *out1 = (t_sample *)(w[4]);
  t_sample *out2 = (t_sample *)(w[5]);
  t_sample *out3 = (t_sample *)(w[6]);
  t_sample *out4 = (t_sample *)(w[7]);
  t_sample *out5 = (t_sample *)(w[8]);
  int n = (int)(w[9]);

  t_sample sample1, sample2;

  while (n--)
    {
      sample1 = *in1++;
      sample2 = *in2++ * DEGTORAD;

      *out1++ = sample1 * 0.707; // W, sqrt(2)/2
      *out2++ = sample1 * cosf(sample2); // Y
      *out3++ = sample1 * sinf(sample2); // X
      *out4++ = sample1 * cosf(2 * sample2); // V
      *out5++ = sample1 * sinf(2 * sample2); // U
    }

  return (w+10);
}

t_int *amen_tilde_perform3(t_int *w)
{
  t_amen_tilde *x = (t_amen_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]); // audio signal
  t_sample *in2 = (t_sample *)(w[3]); // azimuth
  t_sample *out1 = (t_sample *)(w[4]);
  t_sample *out2 = (t_sample *)(w[5]);
  t_sample *out3 = (t_sample *)(w[6]);
  t_sample *out4 = (t_sample *)(w[7]);
  t_sample *out5 = (t_sample *)(w[8]);
  t_sample *out6 = (t_sample *)(w[9]);
  t_sample *out7 = (t_sample *)(w[10]);
  int n = (int)(w[11]);

  t_sample sample1, sample2;

  while (n--)
    {
      sample1 = *in1++;
      sample2 = *in2++ * DEGTORAD;

      *out1++ = sample1 * 0.707; // W, sqrt(2)/2
      *out2++ = sample1 * cosf(sample2); // Y
      *out3++ = sample1 * sinf(sample2); // X
      *out4++ = sample1 * cosf(2 * sample2); // V
      *out5++ = sample1 * sinf(2 * sample2); // U
      *out6++ = sample1 * cosf(3 * sample2); // Q
      *out7++ = sample1 * sinf(3 * sample2); // P
    }

  return (w+12);
}

void amen_tilde_dsp(t_amen_tilde *x, t_signal **sp)
{
  if(x->order <= 1)
    {
      dsp_add(amen_tilde_perform1, 7, x, sp[0]->s_vec, sp[1]->s_vec,
	      sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[0]->s_n);
    }
  else if(x->order == 2)
    {
      dsp_add(amen_tilde_perform2, 9, x, sp[0]->s_vec, sp[1]->s_vec,
	      sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec,
	      sp[6]->s_vec, sp[0]->s_n);
    }
  else if(x->order >= 3)
    {
      dsp_add(amen_tilde_perform3, 11, x, sp[0]->s_vec, sp[1]->s_vec,
	      sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec,
	      sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, sp[0]->s_n);
    }
}

void amen_tilde_free(t_amen_tilde *x)
{
  inlet_free(x->x_in2);
  outlet_free(&x->x_out1);
  outlet_free(&x->x_out2);
  outlet_free(&x->x_out3);
  if(x->order == 2)
    {
      outlet_free(&x->x_out4);
      outlet_free(&x->x_out5);
    }
  if(x->order >= 3)
    {
      outlet_free(&x->x_out6);
      outlet_free(&x->x_out7);
    }
}

void *amen_tilde_new(t_floatarg order)
{
  t_amen_tilde *x = (t_amen_tilde *)pd_new(amen_tilde_class);

  x->order = 0;
  x->order = order;

  x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->x_out1 = outlet_new(&x->x_obj, &s_signal);
  x->x_out2 = outlet_new(&x->x_obj, &s_signal);
  x->x_out3 = outlet_new(&x->x_obj, &s_signal);
  if(x->order == 2)
    {
      x->x_out4 = outlet_new(&x->x_obj, &s_signal);
      x->x_out5 = outlet_new(&x->x_obj, &s_signal);
    }
  else if(x->order >= 3)
    {
      x->x_out4 = outlet_new(&x->x_obj, &s_signal);
      x->x_out5 = outlet_new(&x->x_obj, &s_signal);
      x->x_out6 = outlet_new(&x->x_obj, &s_signal);
      x->x_out7 = outlet_new(&x->x_obj, &s_signal);
    }


  return (void *)x;
}

void amen_tilde_setup(void)
{
  amen_tilde_class = class_new(gensym("amen~"),
                               (t_newmethod)amen_tilde_new,
                               (t_newmethod)amen_tilde_free,
                               sizeof(t_amen_tilde),
                               CLASS_DEFAULT, A_DEFFLOAT, 0);

  class_addmethod(amen_tilde_class,
                  (t_method)amen_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(amen_tilde_class, t_amen_tilde, f);
  class_sethelpsymbol(amen_tilde_class, gensym("holylib-help.pd"));
}
