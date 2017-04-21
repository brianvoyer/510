#include "m_pd.h"
#include <math.h>
#include <string.h>

#define TWOPI 6.2831853072
#define MAXAMPS 144

static char *version = "[wavgen~] v.1 by Brian Voyer <brianvoyer@gmail.com>";

static t_class *wavgen_tilde_class;

typedef struct _wavgen_tilde 
{
  t_object obj; // the Pd object
  t_float x_f; // for internal conversion from float to signal

  long table_length; // length of wavetable
  char wave[20];
  int harmonic_count;
  float *amplitudes;
  t_symbol *waveform;
  float *wavetable; // wavetable
  float phase, si;
  long wavetable_bytes;

  float si_factor; // factor for generating the sampling increment

  // fix the clicks
  float *old_wavetable;
  short dirty;
  float xfade_duration;
  int xfade_samples;
  int xfade_countdown;
  // only implement one crossfade for simplicity

  float sr; // sampling rate
  float maximum_delay_time; // maximum delay time
  long delay_length; // length of the delay line in samples
  long delay_bytes; // length of delay line in bytes
  float *delay_line; // the delay line itself
  float *read_ptr; // read pointer into delay line
  float *write_ptr; // write pointer into delay line
  float delay_time; // current delay time
  float feedback; // feedback multiplier
  short delaytime_connected; // inlet connection status
  short feedback_connected; // inlet connection status
  float fdelay; // the fractional delay time
  long idelay; // the integer delay time
  float fraction; // the fractional difference between the fractional 
                  // and integer delay times
  float srms; //sampling rate as milliseconds
} t_wavgen_tilde;

void tableswitch(t_wavgen_tilde *x)
{
  float *wavetable = x->wavetable;
  float *old_wavetable = x->old_wavetable;
  memcpy(old_wavetable, wavetable, x->table_length * sizeof(float));
  x->dirty = 1;
}

void sinebasic(t_wavgen_tilde *x)
{
  tableswitch(x);
  for(int i = 0; i < x->table_length; i++)
    {
      x->wavetable[i] = sin(TWOPI * (float)i / (float)x->table_length);
    }
  x->dirty = 0;
  x->xfade_countdown = x->xfade_samples;
}

void sawtoothbasic(t_wavgen_tilde *x)
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

void squarebasic(t_wavgen_tilde *x)
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

void trianglebasic(t_wavgen_tilde *x)
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

void sineadd(t_wavgen_tilde *x, int harmonic, float amp)
{
  for(int i = 0; i < x->table_length; i++)
    {
      float phase = (float) harmonic * TWOPI / (float) x->table_length;
      x->wavetable[i] += amp * sin(phase * (float)i);
    }
}

void sawtoothadd(t_wavgen_tilde *x, int harmonic, float amp)
{
  for(int i = 0; i < x->table_length; i++)
    {
      float saw = 0;
      saw = harmonic * fmod( (double)i, x->table_length / harmonic) /
        (float)x->table_length;
      //post("%f", saw);
      // - 0.25 to remove the native DC offset
      x->wavetable[i] += amp * saw - 0.25;
    }
}

void triangleadd(t_wavgen_tilde *x, int harmonic, float amp)
{
  float period = (float)x->table_length / pow(2, harmonic);
  for(int i = 0; i < x->table_length; i++)
    {
      float permod = fmod(i, 2 * period);
      float tri = 0;
      if(permod < period)
        {
          tri = (2 * permod) / period - 1;
        }
      else
        {
          tri = 1 - 2*(permod - period) / period;
        }
      x->wavetable[i] += amp * tri;// - 0.25;
    }
}

void squareadd(t_wavgen_tilde *x, int harmonic, float amp)
{
  float period = (float)x->table_length / pow(2, harmonic);
  for(int i = 0; i < x->table_length; i++)
    {
      float permod = fmod(i, 2 * period);
      float sqr = -1.0;
      if(permod > period)
        {
          sqr = 1.0;
        }
      x->wavetable[i] += amp * sqr;
    }
}

void wavgen_tilde_build_waveform(t_wavgen_tilde *x)
{
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
  for(i = 0; i < table_length; i++)
    {
      wavetable[i] = amplitudes[0];
    }
  for(i = 1; i < partial_count; i++)
    {
      if(x->waveform == gensym("sin"))
        {
          sineadd(x, i, amplitudes[i]);
        }
      else if(x->waveform == gensym("tri"))
        {
          triangleadd(x, i, amplitudes[i]);
        }
      else if(x->waveform == gensym("sqr"))
        {
          squareadd(x, i, amplitudes[i]);
        }
      else if(x->waveform == gensym("saw"))
        {
          sawtoothadd(x, i, amplitudes[i]);
        }
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
      error("[wavgen~]: weird all zero error -- exiting");
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

void wavgen_tilde_list(t_wavgen_tilde *x, t_symbol *msg, short argc,
                     t_atom *argv)
{
  post("No waveform given, default to sine...");
  tableswitch(x);
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
  x->waveform = gensym("sin");
  wavgen_tilde_build_waveform(x);
}

void wavgen_tilde_sin(t_wavgen_tilde *x, t_symbol *msg, short argc,
                    t_atom *argv)
{
  tableswitch(x);
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
  x->waveform = gensym("sin");
  wavgen_tilde_build_waveform(x);
}

void wavgen_tilde_tri(t_wavgen_tilde *x, t_symbol *msg, short argc,
                    t_atom *argv)
{
  tableswitch(x);
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
  x->waveform = gensym("tri");
  wavgen_tilde_build_waveform(x);
}

void wavgen_tilde_saw(t_wavgen_tilde *x, t_symbol *msg, short argc,
                    t_atom *argv)
{
  tableswitch(x);
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
  x->waveform = gensym("saw");
  wavgen_tilde_build_waveform(x);
}

void wavgen_tilde_sqr(t_wavgen_tilde *x, t_symbol *msg, short argc,
                    t_atom *argv)
{
  tableswitch(x);
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
  x->waveform = gensym("sqr");
  wavgen_tilde_build_waveform(x);
}

void *wavgen_tilde_new(t_symbol *s, short argc, t_atom *argv)
{
  float init_freq = 440.0;
  float delmax = 250.0, deltime = 100.0, feedback = 0.1;

  t_wavgen_tilde *x = (t_wavgen_tilde *) pd_new(wavgen_tilde_class);
  inlet_new(&x->obj, &x->obj.ob_pd, gensym("signal"), gensym("signal"));
  inlet_new(&x->obj, &x->obj.ob_pd, gensym("signal"), gensym("signal"));
  outlet_new(&x->obj, gensym("signal"));

  x->table_length = 1048576; //8192; //try 1048576
  x->wavetable_bytes = x->table_length * sizeof(float);
  x->wavetable = (float *) getbytes(x->wavetable_bytes);
  x->old_wavetable = (float *) getbytes(x->wavetable_bytes);
  x->amplitudes = (float *) getbytes(MAXAMPS * sizeof(float));

  x->phase = 0;
  x->sr = sys_getsr();
  x->si_factor = (float) x->table_length / x->sr;
  x->si = init_freq * x->si_factor;

  x->dirty = 0;
  x->xfade_countdown = 0;
  x->xfade_duration = 500.;
  x->xfade_samples = x->xfade_duration * x->sr / 1000.0;

  x->waveform = atom_getsymbolarg(0, argc, argv);

  /* Read user parameters */

  if(delmax <= 0)
    {
      delmax = 250.0;
    }

  x->maximum_delay_time = delmax * 0.001;

  x->delay_time = deltime;
  if(x->delay_time > delmax || x->delay_time <= 0.0){
    error("illegal delay time: %f, reset to 1 ms", x->delay_time);
    x->delay_time = 1.0;
  }

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

  // force memory initialization in the DSP method by setting the
  // sampling rate to zero
  x->sr = 0.0;
  x->feedback = feedback;
  return x;
}

/* The free memory routine */

void wavgen_tilde_free(t_wavgen_tilde *x)
{
  t_freebytes(x->wavetable, x->wavetable_bytes);
  t_freebytes(x->old_wavetable, x->wavetable_bytes);
  t_freebytes(x->delay_line, x->delay_bytes);
}

/* The perform routine */

t_int *wavgen_tilde_perform(t_int *w)
{
  t_wavgen_tilde *x = (t_wavgen_tilde *) (w[1]);
  t_float *frequency = (t_float *) (w[2]);
  t_float *delaytime = (t_float *) (w[3]);
  t_float *feedback = (t_float *) (w[4]);
  t_float *output = (t_float *) (w[5]);
  t_int n = w[6];

  float sr = x->sr;
  float *delay_line = x->delay_line;
  float  *read_ptr  = x->read_ptr;
  float  *write_ptr = x->write_ptr;
  long delay_length = x->delay_length;
  float *endmem = delay_line + delay_length;
  short delaytime_connected = x->delaytime_connected;
  short feedback_connected = x->feedback_connected;
  float delaytime_float = x->delay_time;
  float feedback_float = x->feedback;
  float fraction;
  float fdelay;
  float samp1, samp2;
  long idelay;
  float srms = sr / 1000.0;
  float out_sample, feedback_sample, new_sample;

  // xfade
  long iphase;
  float si_factor = x->si_factor;
  float si = x->si;
  float phase = x->phase;
  long table_length = x->table_length;
  float *wavetable = x->wavetable;
  float *old_wavetable = x->old_wavetable;

  while(n--)
    {
      si = *frequency++ * si_factor;
      iphase = floor(phase); // truncate phase

      fdelay = *delaytime++ * srms;
      while(fdelay > delay_length)
        {
          fdelay -= delay_length;
        }
      while(fdelay < 0)
        {
          fdelay += delay_length;
        }
      idelay = (int)fdelay;
      fraction = fdelay - idelay;
      read_ptr = write_ptr - idelay;
      while(read_ptr < delay_line)
        {
          read_ptr += delay_length;
        }
      samp1 = *read_ptr++;
      if(read_ptr == endmem)
        {
          read_ptr = delay_line;
        }
      samp2 = *read_ptr;
      out_sample = samp1 + fraction * (samp2-samp1);
      feedback_sample = out_sample * *feedback++;
      if(fabs(feedback_sample)  < 0.000001)
        {
          feedback_sample = 0.0;
        }

      if(x->xfade_countdown)
	{
	  float fract = 0.25 * TWOPI * (float)x->xfade_countdown /
	    (float)x->xfade_samples;
	  new_sample = sin(fract) * old_wavetable[iphase] + cos(fract) *
	    wavetable[iphase];
	  --x->xfade_countdown;
	}
      else if(x->dirty)
	{
	  new_sample = old_wavetable[iphase];
	}
      else
	{
	  new_sample = wavetable[iphase];
	}
      *write_ptr++ = new_sample + feedback_sample;
      *output++ = out_sample;
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

      if(write_ptr == endmem)
        {
          write_ptr = delay_line;
        }
    }
  x->write_ptr = write_ptr;
  return w + 7;
}

void wavgen_tilde_dsp(t_wavgen_tilde *x, t_signal **sp, short *count)
{
  int i;
  int oldbytesize = x->delay_bytes;
  /* Exit if the sampling rate is zero */

  if(!sp[0]->s_sr)
    {
      return;
    }
  
  /* Store the states of signal inlet connections */
  
  x->delaytime_connected = 1;
  x->feedback_connected = 1;
  
  /* Reset the delayline if the sampling rate has changed */
  
  if(x->sr != sp[0]->s_sr)
    {
      x->sr = sp[0]->s_sr;
      x->delay_length = x->sr * x->maximum_delay_time + 1;
      x->delay_bytes = x->delay_length * sizeof(float);
      
      if(x->delay_line == NULL)
        {
          x->delay_line = (float *) getbytes(x->delay_bytes);
        }
      
      else 
	{
	  x->delay_line = (float *) resizebytes((void *)x->delay_line, 
						oldbytesize, x->delay_bytes);
	}
      if(x->delay_line == NULL)
        {
          error("[wavgen~]: cannot realloc %d bytes of memory", 
		x->delay_bytes);
          return;
        }
      
      // Clear the delay line
      
      for(i = 0; i < x->delay_length; i++)
        {
          x->delay_line[i] = 0.0;
        }
      
      // Assign the write pointer to the start of the delay line
      
      x->write_ptr = x->delay_line;
    }
  dsp_add(wavgen_tilde_perform, 6, x, sp[0]->s_vec, sp[1]->s_vec, 
	  sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

void wavgen_tilde_setup(void)
{
  wavgen_tilde_class = class_new(gensym("wavgen~"),
				 (t_newmethod)wavgen_tilde_new,
				 (t_method)wavgen_tilde_free, 
				 sizeof(t_wavgen_tilde),
				 CLASS_DEFAULT, A_GIMME, 0);
  
  class_addmethod(wavgen_tilde_class, (t_method)wavgen_tilde_dsp, 
		  gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(wavgen_tilde_class, t_wavgen_tilde, x_f);

  // change the waveform
  class_addmethod(wavgen_tilde_class, (t_method)sinebasic, gensym("sine"), 0);
  class_addmethod(wavgen_tilde_class, (t_method)trianglebasic,
                  gensym("triangle"), 0);
  class_addmethod(wavgen_tilde_class, (t_method)sawtoothbasic,
                  gensym("sawtooth"), 0);
  class_addmethod(wavgen_tilde_class, (t_method)squarebasic, gensym("square"),
                  0);

  // additive synthesis methods
  class_addmethod(wavgen_tilde_class, (t_method)wavgen_tilde_list, 
		  gensym("list"), A_GIMME, 0);
  class_addmethod(wavgen_tilde_class, (t_method)wavgen_tilde_sin, 
		  gensym("sin"), A_GIMME, 0);
  class_addmethod(wavgen_tilde_class, (t_method)wavgen_tilde_tri, 
		  gensym("tri"), A_GIMME, 0);
  class_addmethod(wavgen_tilde_class, (t_method)wavgen_tilde_saw, 
		  gensym("saw"), A_GIMME, 0);
  class_addmethod(wavgen_tilde_class, (t_method)wavgen_tilde_sqr, 
		  gensym("sqr"), A_GIMME, 0);

  post(version);
}
