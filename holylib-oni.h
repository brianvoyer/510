// [oni] Optimized coefficient geNeratIon
//for holylib (c) 2017 Brian Voyer

// ignoring elevation--assume listener is always on the same horizontal plane
// as the speakers,

static t_class *oni_class;

typedef struct _oni
{
  t_object  x_obj;
  t_sample f;

  t_inlet *x_in2;

  t_outlet *output;

  float* coeffs;
  float* optcoeff;

  int order;
} t_oni;

// basically, generate a list of 2N+1 coefficients: s1 = 1/(1+N)
// then coeff 1 is s1 * sqrt(2) * 0.5, followed by:
// coeff(2n) = s1 * sinf(n*s2)
// coeff(2n+1) = s1 * cosf(n*s2)

// maybe do this as message rate then in the decoder do it too? idk

void oni_generate(t_oni *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *listOut;
  x->order = (int)x->order;
  int totallength = 2 * x->order + 1;
  int outletnum = argc;
  int i, j;
  float unitary = 1 / ((float) x->order + 1);
  //post("unitary: %f", unitary);
  x->coeffs = malloc(outletnum * totallength * sizeof(float));
  
  for (i = 0; i < outletnum; i++)
    {
      //post("on speaker %d at angle %f", i, atom_getfloat(&argv[i]));
      float currentangle = atom_getfloat(&argv[i]) * DEGTORAD;
      x->coeffs[i * totallength] = unitary * RTWOTWO; 
      for (j = 1; j < totallength; j++)
	{
	  if (j % 2 == 0)
	    {
	      x->coeffs[i * totallength + j] 
		= unitary * sinf( (j / 2) * currentangle) * x->optcoeff[j];
	    }
	  else
	    {
	      x->coeffs[i * totallength + j] 
		= unitary * cosf( ((j / 2) + 1) * currentangle) * x->optcoeff[j];
	    }
	}
    }
  //for (i = 0; i < outletnum * totallength; i++)
  //  post("%f", x->coeffs[i]);
  //post("done");
  listOut = (t_atom *)t_getbytes(outletnum * totallength * sizeof(t_atom));
  for (i = 0; i < outletnum * totallength; i++)
    {
      if(fabs(x->coeffs[i]) < 0.00001)
	x->coeffs[i] = 0;
      SETFLOAT(listOut+i, x->coeffs[i]);
    }
  outlet_list(x->output, 0, outletnum * totallength, listOut);
  t_freebytes(listOut, outletnum * totallength * sizeof(t_atom));
}

void oni_free(t_oni *x)
{
  outlet_free(&x->output);
  t_freebytes(&x->optcoeff, x->order * sizeof(float));
}

void *oni_new(t_symbol *s, int argc, t_atom *argv)
{
  t_oni *x = (t_oni *)pd_new(oni_class);

  int inphase;
  char sel_mode[MAXPDSTRING];

  if (argc != 2)
    {
      error("[oni~]: usage: [imp~ (in/out) ORDER].");
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
	    post("[oni]: defaulting to \"in-phase\"");
	  inphase = 1;
	}
    }

  x->order = atom_getfloat(&argv[1]);
  //x->order = atom_string(&argv[0], sel_mode, MAXPDSTRING);;

  int i;
  int j = 1;
  x->optcoeff = (t_atom *)t_getbytes((2 * x->order + 1) * sizeof(float)); 
  if(inphase)
    {
      int nbsqr = pow(factorial(x->order), 2);
      x->optcoeff[0] = 1; // N!^2 / (N! * N!) = 1
      
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
  x->output = outlet_new(&x->x_obj, gensym("list")); //or &s_list

  return (void *)x;
}

void oni_setup(void)
{
  oni_class = class_new(gensym("oni"),
			   (t_newmethod)oni_new,
			   (t_newmethod)oni_free,
			   sizeof(t_oni),
			   CLASS_DEFAULT, A_GIMME, 0);
  
  class_addlist(oni_class, oni_generate);

  class_sethelpsymbol(oni_class, gensym("holylib-help.pd"));
}
