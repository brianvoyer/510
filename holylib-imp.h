// [imp~] - sIMPle ambisonic decoder
// for holylib (c) 2017 Brian Voyer
// for mono, stereo, quad, and hex
// ignoring elevation

static t_class *imp_tilde_class;

typedef struct _imp_tilde {
  t_object  x_obj;
  t_sample f;

  float *optcoeff;
  float *deccoeff;

  int order;

  int dectype;
} t_imp_tilde;

t_int *imp_tilde_mono(t_int *w)
{
  t_imp_tilde *x = (t_imp_tilde *)(w[1]);
  t_sample *in1     = (t_sample *)(w[2]); // W
  t_sample *in2     = (t_sample *)(w[3]); // Y
  t_sample *in3     = (t_sample *)(w[4]); // X
  t_sample *out1    = (t_sample *)(w[5]);
  int n = (int)(w[6]);
  t_sample sample1;
  t_sample outsample;

  while(n--)
    {
      sample1 = *in1++  * x->optcoeff[0];
      *out1++ = sample1 * x->deccoeff[0];
    }
  return (w+7);
}

t_int *imp_tilde_stereo(t_int *w)
{
  t_imp_tilde *x = (t_imp_tilde *)(w[1]);
  t_sample *in1     = (t_sample *)(w[2]); // W
  t_sample *in2     = (t_sample *)(w[3]); // Y
  t_sample *in3     = (t_sample *)(w[4]); // X
  t_sample *out1    = (t_sample *)(w[5]);
  t_sample *out2    = (t_sample *)(w[6]);
  int n = (int)(w[7]);
  t_sample insamples[3];
  t_sample outsamples[2];

  int i, j;

  while (n--)
    {
      outsamples[0] = 0;
      outsamples[1] = 0;

      insamples[0] = (*in1++) * x->optcoeff[0]; // W
      insamples[1] = (*in2++) * x->optcoeff[1]; // Y
      insamples[2] = (*in3++) * x->optcoeff[2]; // X

      // stereo decoder
      for (i = 0; i < 2; i++)
        {
          for (j = 0; j < 3; j++)
            {
              outsamples[i] += insamples[j] * x->deccoeff[3 * i + j];
              //post("deccoeff[%d, %d]: %.2f",i, j, x->deccoeff[3 * i + j]);
            }
        }
      (*out1++) = outsamples[0];
      (*out2++) = outsamples[1];
    }

  return (w+8);
}

t_int *imp_tilde_quad(t_int *w)
{
  t_imp_tilde *x = (t_imp_tilde *)(w[1]);
  t_sample *in1     = (t_sample *)(w[2]); // W
  t_sample *in2     = (t_sample *)(w[3]); // Y
  t_sample *in3     = (t_sample *)(w[4]); // X
  t_sample *out1    = (t_sample *)(w[5]);
  t_sample *out2    = (t_sample *)(w[6]);
  t_sample *out3    = (t_sample *)(w[7]);
  t_sample *out4    = (t_sample *)(w[8]);
  int n = (int)(w[9]);
  t_sample insamples[3];
  t_sample outsamples[4];

  int i, j;

  while (n--)
    {
      outsamples[0] = 0;
      outsamples[1] = 0;
      outsamples[2] = 0;
      outsamples[3] = 0;

      insamples[0] = *in1++ * x->optcoeff[0]; // W
      insamples[1] = *in2++ * x->optcoeff[1]; // Y
      insamples[2] = *in3++ * x->optcoeff[2]; // X

      // quad decoder
      for (i = 0; i < 4; i++)
        {
          for (j = 0; j < 3; j++)
            {
              outsamples[i] += insamples[j] * x->deccoeff[3 * i + j];
            }
        }

      *out1++ = outsamples[0];
      *out2++ = outsamples[1];
      *out3++ = outsamples[2];
      *out4++ = outsamples[3];
    }

  return (w+10);
}

t_int *imp_tilde_hex(t_int *w)
{
  t_imp_tilde *x = (t_imp_tilde *)(w[1]);
  t_sample *in1     = (t_sample *)(w[2]); // W
  t_sample *in2     = (t_sample *)(w[3]); // Y
  t_sample *in3     = (t_sample *)(w[4]); // X
  t_sample *in4     = (t_sample *)(w[5]); // V
  t_sample *in5     = (t_sample *)(w[6]); // U
  t_sample *out1    = (t_sample *)(w[7]);
  t_sample *out2    = (t_sample *)(w[8]);
  t_sample *out3    = (t_sample *)(w[9]);
  t_sample *out4    = (t_sample *)(w[10]);
  t_sample *out5    = (t_sample *)(w[11]);
  t_sample *out6    = (t_sample *)(w[12]);
  int n = (int)(w[13]);

  t_sample insamples[5];
  t_sample outsamples[6];

  int i, j;

  while (n--)
    {
      outsamples[0] = 0;
      outsamples[1] = 0;
      outsamples[2] = 0;
      outsamples[3] = 0;
      outsamples[4] = 0;
      outsamples[5] = 0;

      insamples[0] = *in1++ * x->optcoeff[0]; // W
      insamples[1] = *in2++ * x->optcoeff[1]; // Y
      insamples[2] = *in3++ * x->optcoeff[2]; // X
      insamples[3] = *in4++ * x->optcoeff[3]; // V
      insamples[4] = *in5++ * x->optcoeff[4]; // U

      // hex decoder
      for (i = 0; i < 6; i++)
        {
          for (j = 0; j < 5; j++)
            {
              outsamples[i] += insamples[j] * x->deccoeff[5 * i + j];
            }
        }
      *out1++ = outsamples[0];
      *out2++ = outsamples[1];
      *out3++ = outsamples[2];
      *out4++ = outsamples[3];
      *out5++ = outsamples[4];
      *out6++ = outsamples[5];
    }

  return (w+14);
}

void imp_tilde_dsp(t_imp_tilde *x, t_signal **sp)
{
  if(x->dectype == 1) // mono
    {
      dsp_add(imp_tilde_mono, 6, x, sp[0]->s_vec, sp[1]->s_vec,
              sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
    }
  else if(x->dectype == 2) // stereo
    {
      dsp_add(imp_tilde_stereo, 7, x, sp[0]->s_vec, sp[1]->s_vec,
              sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[0]->s_n);
    }
  else if(x->dectype == 3) // quad
    {
      dsp_add(imp_tilde_quad, 9, x, sp[0]->s_vec, sp[1]->s_vec,
              sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec,
              sp[6]->s_vec, sp[0]->s_n);
    }
  else if(x->dectype == 4) // hex
    {
      dsp_add(imp_tilde_hex, 13, x, sp[0]->s_vec, sp[1]->s_vec,
              sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec,
              sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, sp[9]->s_vec,
              sp[10]->s_vec, sp[0]->s_n);
    }
}

void imp_tilde_free(t_imp_tilde *x)
{
  free(x->optcoeff);
  free(x->deccoeff);
}

void *imp_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  int i;
  t_imp_tilde *x = (t_imp_tilde *)pd_new(imp_tilde_class);
  
  int inphase;

  float *angles;

  x->dectype = 0;
  int inletnum = 2; // one inlet for free
  int outletnum = 1;
  char sel_mode[MAXPDSTRING];
  if (argc != 2)
    {
      error("[imp~]: usage: [imp~ (in/out) (mono/stereo/quad/hex)].");
      error("See holylib-help.pd");
    }
  else
    {
      atom_string(&argv[0], sel_mode, MAXPDSTRING);
      if (strcmp(sel_mode, "out") == 0)
	{
	  inphase = 0;
	}
      else
	{
	  if(strcmp(sel_mode, "in") != 0)
	    post("[imp~]: defaulting to \"in-phase\"");
	  inphase = 1;
	}
      atom_string(&argv[1], sel_mode, MAXPDSTRING);
      // mono is 3 in, 1 out
      // stereo is 3 in, 2 out
      // quad is 3 in, 4 out
      // hex is 5 in, 6 out
      if (strcmp(sel_mode, "mono") == 0)
	{
	  post("Selected mono!");
	  x->dectype = 1;
	  x->order = 1;
	  x->optcoeff = (float *)malloc(3 * sizeof(float));
	  x->deccoeff = (float *)malloc(sizeof(float));
	  // this is just happening, no generation required
	  x->optcoeff[0] = 1;
	  x->deccoeff[0] = 0.707;
	}
      else if (strcmp(sel_mode, "quad") == 0)
	{
	  post("Selected quad!");
	  x->dectype = 3;
	  outletnum = 4;
	  x->order = 1;
	  x->optcoeff = (float *)malloc(3 * sizeof(float));
	  x->deccoeff = (float *)malloc(12 * sizeof(float));
	  angles = (float *)malloc(outletnum * sizeof(float));
	  angles[0] = 315;
	  angles[1] = 225;
	  angles[2] = 135;
	  angles[3] = 45;
	}
      else if (strcmp(sel_mode, "hex") == 0)
	{
	  post("Selected hex!");
	  x->dectype = 4;
	  inletnum = 4; // one for free
	  outletnum = 6;
	  angles = (float *)malloc(outletnum * sizeof(float));
	  x->order = 2;
	  x->optcoeff = (float *)malloc(5 * sizeof(float));
	  x->deccoeff = (float *)malloc(30 * sizeof(float));
	  angles[0] = 330;
	  angles[1] = 270;
	  angles[2] = 210;
	  angles[3] = 150;
	  angles[4] = 90;
	  angles[5] = 30;
	}
      else
	{
	  if (strcmp(sel_mode, "stereo") == 0)
	    post("Selected stereo!");
	  // if not set, default to stereo
	  else
	    post("Didn't select stereo, but doing it anyway!");
	  x->dectype = 2;
	  outletnum = 2;
	  x->order = 1;
	  x->optcoeff = (float *)malloc(3 * sizeof(float));
	  x->deccoeff = (float *)malloc(6 * sizeof(float));
	  angles = (float *)malloc(outletnum * sizeof(float));
	  angles[0] = 270;
	  angles[1] = 90;
	}
      
      x->order = (x->order <= 1) ? 1: (x->order == 2) ? 2 : 3;  
      int j;
      float unitary = 1 / ((float)x->order + 1);
      for (i = 0; i < outletnum; i++)
	{
	  float currentangle = angles[i] * DEGTORAD;
	  x->deccoeff[i * inletnum] = unitary * RTWOTWO;
	  for (j = 1; j < inletnum; j++)
	    {
	      if (j % 2 == 0)
		{
		  x->deccoeff[i * inletnum + j]
		    = unitary * sinf( (j / 2) * currentangle);
		}
	      else
		{
		  x->deccoeff[i * inletnum + j]
		    = unitary * cosf( ((j / 2) + 1) * currentangle);
		}
	    }
	}
      if (inphase)
	{
	  // generate optimization coefficients here:
	  x->optcoeff[0] = 1; // N!^2/N!^2
	  j = 1;
	  int nbsqr = pow(factorial(x->order), 2);
	  float ocoeff;
	  for (i = 1; i < 2 * x->order + 1; i += 2)
	    {
	      ocoeff = ((float) nbsqr) /
		((float)(factorial(x->order + j) * factorial(x->order - j)));
	      x->optcoeff[i] = ocoeff;
	      x->optcoeff[i+1] = ocoeff;
	      j++;
	    }
	}
      else
	{
	  for (i = 1; i < 2 * x->order + 1; i++)
	    x->optcoeff[i] = 1;
	}
      for (i = 0; i < inletnum; i++)
	{
	  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	}
      for (i = 0; i < outletnum; i++)
	{
	  outlet_new(&x->x_obj, &s_signal);
	}
    }
  return (void *)x;
}

void imp_tilde_setup(void)
{
  imp_tilde_class = class_new(gensym("imp~"),
                                 (t_newmethod)imp_tilde_new,
                                 (t_newmethod)imp_tilde_free,
                                 sizeof(t_imp_tilde),
                                 CLASS_DEFAULT, A_GIMME, 0);

  class_addmethod(imp_tilde_class,
                  (t_method)imp_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(imp_tilde_class, t_imp_tilde, f);
  class_sethelpsymbol(imp_tilde_class, gensym("holylib-help.pd"));
}
