#include <m_pd.h>
#include <math.h>
#include <string.h>

#define TWOPI 6.2831853072

#define MAXAMPS 144

static char *version = "[simplesine~] v.1 by Brian Voyer <brianvoyer@gmail.com>";

static t_class *simplesine_class;

typedef struct _simplesine
{
  t_object obj;
  t_sample x_f;

  long table_length; // length of wavetable
  char wave[20];
  int harmonic_count;
  float *amplitudes;
  t_symbol *waveform;
  float *wavetable; // wavetable
  float sr, phase, si;
  long wavetable_bytes;

  float si_factor; // factor for generating the sampling increment

  // fix the clicks
  float *old_wavetable;
  short dirty;
  float xfade_duration;
  int xfade_samples;
  int xfade_countdown;
  // only implement one crossfade for simplicity

} t_simplesine;

void tableswitch(t_simplesine *x)
{
  float *wavetable = x->wavetable;
  float *old_wavetable = x->old_wavetable;
  memcpy(old_wavetable, wavetable, x->table_length * sizeof(float));
  x->dirty = 1;
}

void sinebasic(t_simplesine *x)
{
  tableswitch(x);
  for(int i = 0; i < x->table_length; i++)
    {
      x->wavetable[i] = sin(TWOPI * (float)i / (float)x->table_length);
    }
  x->dirty = 0;
  x->xfade_countdown = x->xfade_samples;
}

void sawtoothbasic(t_simplesine *x)
{
  tableswitch(x);
  for(int i = 0; i < x->table_length; i++)
    {
      float saw = 0;
      saw = 2*((float)i / (float)x->table_length) - 1;
      //post("%f", saw);
      x->wavetable[i] = saw;
    }
  /*implement normalization*/
  float max = 0.0, rescale;

  for(int j = 0; j < x->table_length; j++)
    {
      if(max < fabs(x->wavetable[j]))
        {
          max = fabs(x->wavetable[j]);
          // post("%f", max);
        }
    }

  if(max == 0.0) //avoid divide by zero
    {
      return;
    }

  rescale = 1.0 / max ;

  for(int k = 0; k < x->table_length; k++)
    {
      x->wavetable[k] *= rescale ;
    }

  x->dirty = 0;
  x->xfade_countdown = x->xfade_samples;
}

void squarebasic(t_simplesine *x)
{
  tableswitch(x);
  for(int i = 0; i < x->table_length; i++)
    {
      float sqr = 0;
      if(i > x->table_length/2)
        {
          sqr = 1.0;
        }
      else
        {
          sqr = -1.0;
        }
      //post("%f", sqr);
      x->wavetable[i] = sqr;
    }
  /*implement normalization*/
  float max = 0.0, rescale;

  for(int j = 0; j < x->table_length; j++)
    {
      if(max < fabs(x->wavetable[j]))
        {
          max = fabs(x->wavetable[j]);
          // post("%f", max);
        }
    }

  if(max == 0.0) //avoid divide by zero
    {
      return;
    }

  rescale = 1.0 / max ;

  for(int k = 0; k < x->table_length; k++)
    {
      x->wavetable[k] *= rescale ;
    }

  x->dirty = 0;
  x->xfade_countdown = x->xfade_samples;
}

void trianglebasic(t_simplesine *x)
{
  tableswitch(x);
  for(int i = 0; i < x->table_length; i++)
    {
      float tri = 0;
      if(i < x->table_length/2)
        {
          tri = (float)i / ((float)x->table_length);
        }
      else
        {
          tri = (1 - ((float)i / ((float)x->table_length)));
          // 0.5 + = cheap way to 'normalize'
        }
      //post("%f", tri);
      x->wavetable[i] = tri - 0.25;
    }

  /*implement normalization*/
  float max = 0.0, rescale;

  for(int j = 0; j < x->table_length; j++)
    {
      if(max < fabs(x->wavetable[j]))
        {
          max = fabs(x->wavetable[j]);
          // post("%f", max);
        }
    }

  if(max == 0.0) //avoid divide by zero
    {
      return;
    }

  rescale = 1.0 / max ;

  for(int k = 0; k < x->table_length; k++)
    {
      x->wavetable[k] *= rescale ;
    }
  x->dirty = 0;
  x->xfade_countdown = x->xfade_samples;
}

void sineadd(t_simplesine *x, int harmonic, float amp)
{
  for(int i = 0; i < x->table_length; i++)
    {
      float phase = (float) harmonic * TWOPI / (float) x->table_length;
      x->wavetable[i] += amp * sin(phase * (float)i);
    }
}

void *simplesine_new(t_symbol *s, int argc, t_atom *argv)
{
  float init_freq;
  init_freq = 440;
  t_simplesine *x = (t_simplesine *) pd_new(simplesine_class);
  outlet_new(&x->obj, gensym("signal"));
  x->table_length = 1048576; //8192; //try 1048576
  x->wavetable_bytes = x->table_length * sizeof(float);
  x->wavetable = (float *) getbytes(x->wavetable_bytes);
  x->old_wavetable = (float *) getbytes(x->wavetable_bytes);
  x->amplitudes = (float *) getbytes(MAXAMPS * sizeof(float));

  x->phase = 0;
  x->sr = sys_getsr();
  x->si_factor = (float) x->table_length / x->sr;
  x->si = init_freq * x->si_factor ;

  x->dirty = 0;
  x->xfade_countdown = 0;
  x->xfade_duration = 100.;
  x->xfade_samples = x->xfade_duration * x->sr / 1000.0;

  x->waveform = atom_getsymbolarg(0, argc, argv);

  /* Branch to the appropriate method to initialize the waveform */

  if (x->waveform == gensym("sine"))
    {
      sinebasic(x);
    }
  else if (x->waveform == gensym("triangle"))
    {
      trianglebasic(x);
    }
  else if (x->waveform == gensym("square"))
    {
      squarebasic(x);
    }
  else if (x->waveform == gensym("sawtooth"))
    {
      sawtoothbasic(x);
    }
  else
    {
      error("%s is not a legal waveform - using sine wave instead",
            x->waveform->s_name);
      sinebasic(x);
    }

  return (void *)x;
}

void simplesine_build_waveform(t_simplesine *x)
{
  // build waveform only uses sines
  float rescale;
  int i;
  float max;
  float *wavetable = x->wavetable;
  float *amplitudes = x->amplitudes;
  int partial_count = x->harmonic_count + 1;
  int table_length = x->table_length;

  if(partial_count < 1)
    {
      error("No harmonics specified, waveform not created");
      return;
    }
  max = 0.0;
  for(i = 0; i < partial_count; i++)
    {
      max += fabs(amplitudes[i]);
    }
  if(!max)
    {
      error("All zero function, waveform not created");
      return;
    }

  // for(i = 0; i < MAXAMPS; i++)
  //   {
  //     if(amplitudes[i])
  //     post("%d: %f", i, amplitudes[i]);
  //   }

  tableswitch(x);

  for(i = 0; i < table_length; i++)
    {
      wavetable[i] = amplitudes[0];
    }
  for(i = 1; i < partial_count; i++)
    {
      sineadd(x, i, amplitudes[i]);
    }
  max = 0.0;
  for(i = 0; i < (int)(table_length * 0.5); i++)
    {
      if(max < fabs(wavetable[i]))
        {
          max = fabs(wavetable[i]);
        }
    }
  if(max == 0)
    {
      error("simplesine~: weird all zero error -- exiting");
      return;
    }
  rescale = 1.0 / max;
  for(i = 0; i < table_length; i++)
    {
      wavetable[i] *= rescale;
    }
  x->dirty = 0;
  x->xfade_countdown = x->xfade_samples;
}

void simplesine_list(t_simplesine *x, t_symbol *msg, short argc,
                     t_atom *argv)
{
  short i;
  int harmonic_count = 0;
  float *amplitudes = x->amplitudes;
  for(i = 0; i < MAXAMPS; i++)
    {
      amplitudes[i] = 0;
    }
  for(i = 0; i < argc; i++)
    {
      amplitudes[harmonic_count++] = atom_getfloat(argv + i);
    }
  x->harmonic_count = harmonic_count;
  simplesine_build_waveform(x);
}

t_int *simplesine_perform(t_int *w)
{
  t_simplesine *x = (t_simplesine *) (w[1]);
  float *frequency = (t_float *)(w[2]);
  float *out = (t_float *)(w[3]);
  int n = w[4];

  long iphase;
  float si_factor = x->si_factor;
  float si = x->si;
  float phase = x->phase;
  long table_length = x->table_length;
  float *wavetable = x->wavetable;
  float *old_wavetable = x->old_wavetable;

  while (n--)
    {
      si = *frequency++ * si_factor;
      iphase = floor(phase); //truncate phase

      if(x->xfade_countdown)
	{
	  float fraction = 0.25 * TWOPI * (float)x->xfade_countdown / 
	    (float)x->xfade_samples;
	  *out++ = sin(fraction) * old_wavetable[iphase] + cos(fraction) * 
	    wavetable[iphase];
	  --x->xfade_countdown;
	}
      else if(x->dirty)
	{
	  *out++ = old_wavetable[iphase];
	}
      else
	{
	  *out++ = wavetable[iphase];
	}
      phase += si;

      while(phase >= table_length)
        {
          phase -= table_length;
        }
      while(phase < 0)
        {
          phase += table_length;
        }
      x->phase = phase;
    }
  return w + 5;
}


void simplesine_dsp(t_simplesine *x, t_signal **sp)
{
  x->si *= x->sr / sp[0]->s_sr;
  x->sr = sp[0]->s_sr;
  x->si_factor = (float) x->table_length / x->sr;

  /* add routine to dsp tree */
  dsp_add(simplesine_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}


void simplesine_free(t_simplesine *x)
{
  t_freebytes(x->wavetable, x->wavetable_bytes);
}


void simplesine_tilde_setup(void)
{
  simplesine_class = class_new(gensym("simplesine~"),
                               (t_newmethod)simplesine_new,
                               (t_method)simplesine_free, sizeof(t_simplesine),
                               CLASS_DEFAULT, A_GIMME, 0);
  class_addmethod(simplesine_class, (t_method)simplesine_dsp, gensym("dsp"),0);
  CLASS_MAINSIGNALIN(simplesine_class, t_simplesine, x_f);

  class_addmethod(simplesine_class, (t_method)sinebasic, gensym("sine"),0);
  class_addmethod(simplesine_class, (t_method)trianglebasic,
                  gensym("triangle"),0);
  class_addmethod(simplesine_class, (t_method)sawtoothbasic,
                  gensym("sawtooth"),0);
  class_addmethod(simplesine_class, (t_method)squarebasic, gensym("square"),0);

  // additive synthesis method
  class_addmethod(simplesine_class, (t_method)simplesine_list, gensym("list"),
                  A_GIMME, 0);

  post(version);
}

// // all for additive synthesis with non-sine waves, decided not to implement
// // it, hence the comment
// void sawtoothadd(t_simplesine *x, int harmonic, float amp)
// {
//   for(int i = 0; i < x->table_length; i++)
//     {
//       float saw = (float)harmonic * i / (float)x->table_length;
//       //post("%f", saw);
//       saw *= amp;
//       x->wavetable[i] += saw;
//     }
// }
//
// void squareadd(t_simplesine *x, int harmonic, float amp)
// {
//   for(int i = 0; i < x->table_length; i++)
//     {
//       float sqr = 0;
//       if((float) harmonic * i > x->table_length/2)
//         {
//           sqr = 1.0;
//         }
//       else
//         {
//           sqr = 0.0;
//         }
//       //post("%f", sqr);
//       sqr *= amp;      
//       int in = (i * harmonic) % x->table_length;
//       x->wavetable[i] += sqr;
//     }
// }
//
// void triangleadd(t_simplesine *x, int harmonic, float amp)
// {
//   for(int i = 0; i < x->table_length; i++)
//     {
//       float tri = 0;
//       if((float) harmonic * i < x->table_length/2)
//         {
//           tri = (float) harmonic * (float)i / ((float)x->table_length);
//         }
//       else
//         {
//           tri = (1 - ((float) harmonic * (float)i / ((float)x->table_length)));
//           // 0.5 + = cheap way to 'normalize'
//         }
//       //post("%f", tri);
//       tri *= amp;
//       x->wavetable[i] += tri;
//     }
//   /*implement normalization*/
//   float max = 0.0, rescale;
//
//   for(int j = 0; j < x->table_length; j++)
//     {
//       if(max < fabs(x->wavetable[j]))
//         {
//           max = fabs(x->wavetable[j]);
//           // post("%f", max);
//         }
//     }
//
//   if(max == 0.0) //avoid divide by zero
//     {
//       return;
//     }
//
//   rescale = 1.0 / max ;
//
//   for(int k = 0; k < x->table_length; k++)
//     {
//       x->wavetable[k] *= rescale ;
//     }
// }
