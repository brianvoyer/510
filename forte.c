/* [forte]: an external for organizing pitch data based on pitch sets.
 * Give up to 12 non-overlapping pitch sets. Then give a PC 0 (C, C#/Db, etc.).
 * If a MIDI note is within a pitch set, then output that note to the
 * corresponding outlet. If there is no corresponding outlet, send it to a
 * default outlet.
 *
 * e.g.,
 * set PC0 = C, sets = 0134, 28t, 59e, 6
 * (60->0), (61->0), (62->1), (63->0), (64->0), (65->2), (66->3),
 * (67->4 [DEFAULT]), (68->1), (69->2), (70->1), (71->2)
 *
 * On recieving a message "root %s":
 *    set an interal variable PC to be 0-11 based on what is sent
 * 
 */
#include "m_pd.h"
#include <string.h>

static char *version = "[forte] v.1 by Brian Voyer <brianvoyer@gmail.com>";

static t_class *forte_class;

typedef struct _forte
{
  t_object fte;
  char strs[MAXPDSTRING];
  int pcs[12];
  int root;
  int max_outlet;
  t_outlet *outlet[12];
} t_forte;

void forte_float(t_forte *x, t_floatarg f)
{
  int index = ((int)f + 12 - x->root) % 12;
  if(index < 12 && index > -1)
    {
      post("PC %d goes to outlet %d", index, x->pcs[index]);
      outlet_float(x->outlet[x->pcs[index]], f);
    }  
  else
    error("Invalid input");
}

void forte_update(t_forte *x, t_symbol *s, int argc, t_atom *argv)
{
  char temp[MAXPDSTRING];
  int foo = x->root;
  t_atom *bar = &argv[0];
  atom_string(bar, temp, sizeof(temp));
  if(strcmp(temp, "C") == 0 || strcmp(temp, "c") == 0)
    {
      x->root = 0;
      post("C set as root.");
    }  
  if(strcmp(temp, "C#") == 0 || strcmp(temp, "c#") == 0)
    {
      x->root = 1;
      post("C# set as root.");
    }  
  if(strcmp(temp, "Db") == 0 || strcmp(temp, "db") == 0)
    {
      x->root = 1;
      post("Db set as root.");
    }  
  if(strcmp(temp, "D") == 0 || strcmp(temp, "d") == 0)
    {
      x->root = 2;
      post("D set as root.");
    }  
  if(strcmp(temp, "D#") == 0 || strcmp(temp, "d#") == 0)
    {
      x->root = 3;
      post("D# set as root.");
    }  
  if(strcmp(temp, "Eb") == 0 || strcmp(temp, "eb") == 0)
    {
      x->root = 3;
      post("Eb set as root.");
    }  
  if(strcmp(temp, "E") == 0 || strcmp(temp, "e") == 0)
    {
      x->root = 4;
      post("E set as root.");
    }  
  if(strcmp(temp, "E#") == 0 || strcmp(temp, "e#") == 0)
    {
      x->root = 5;
      post("E# set as root.");
    }  
  if(strcmp(temp, "Fb") == 0 || strcmp(temp, "fb") == 0)
    {
      x->root = 4;
      post("Fb set as root.");
    }  
  if(strcmp(temp, "F") == 0 || strcmp(temp, "f") == 0)
    {
      x->root = 5;
      post("F set as root.");
    }  
  if(strcmp(temp, "F#") == 0 || strcmp(temp, "f#") == 0)
    {
      x->root = 6;
      post("F# set as root.");
    }  
  if(strcmp(temp, "Gb") == 0 || strcmp(temp, "gb") == 0)
    {
      x->root = 6;
      post("Gb set as root.");
    }  
  if(strcmp(temp, "G") == 0 || strcmp(temp, "g") == 0)
    {
      x->root = 7;
      post("G set as root.");
    }  
  if(strcmp(temp, "G#") == 0 || strcmp(temp, "g#") == 0)
    {
      x->root = 8;
      post("G# set as root.");
    }  
  if(strcmp(temp, "Ab") == 0 || strcmp(temp, "ab") == 0)
    {
      x->root = 8;
      post("Ab set as root.");
    }  
  if(strcmp(temp, "A") == 0 || strcmp(temp, "a") == 0)
    {
      x->root = 9;
      post("A set as root.");
    }  
  if(strcmp(temp, "A#") == 0 || strcmp(temp, "a#") == 0)
    {
      x->root = 10;
      post("A# set as root.");
    }  
  if(strcmp(temp, "Bb") == 0 || strcmp(temp, "bb") == 0)
    {
      x->root = 10;
      post("Bb set as root.");
    }  
  if(strcmp(temp, "B") == 0 || strcmp(temp, "b") == 0)
    {
      x->root = 11;
      post("B set as root.");
    }  
  if(strcmp(temp, "B#") == 0 || strcmp(temp, "b#") == 0)
    {
      x->root = 0;
      post("B# set as root.");
    }  
  if(strcmp(temp, "Cb") == 0 || strcmp(temp, "cb") == 0)
    {
      x->root = 11;
      post("Cb set as root.");
    }  
}

void *forte_new(t_atom *s, int argc, t_atom *argv)
{
  t_forte *x = (t_forte *)pd_new(forte_class);
  x->root = 0;
  x->max_outlet = argc;
  for(int i = 0; i < 12; i++)
    {
      x->pcs[i] = x->max_outlet;
    }
  for(int i = 0; i < argc; i++)
    {
      t_atom *temp = &argv[i];
      atom_string(temp, x->strs, sizeof(x->strs));
      int arglen = strlen(x->strs);
      
      for(int j = 0; j < arglen; j++)
	{
	  // this is messy, but I can't seem to abstract it out?
	  if(x->strs[j] == '0') 
	    {
	      if(x->pcs[0] == x->max_outlet)
		x->pcs[0] = i;
	      else
		error("PC %d already assigned", 0);
	    }
	  else if(x->strs[j] == '1') 
	    {
	      if(x->pcs[1] == x->max_outlet)
		x->pcs[1] = i;
	      else
		error("PC %d already assigned", 1);
	    }
	  else if(x->strs[j] == '2') 
	    {
	      if(x->pcs[2] == x->max_outlet)
		x->pcs[2] = i;
	      else
		error("PC %d already assigned", 2);
	    }
	  else if(x->strs[j] == '3') 
	    {
	      if(x->pcs[3] == x->max_outlet)
		x->pcs[3] = i;
	      else
		error("PC %d already assigned", 3);
	    }
	  else if(x->strs[j] == '4') 
	    {
	      if(x->pcs[4] == x->max_outlet)
		x->pcs[4] = i;
	      else
		error("PC %d already assigned", 4);
	    }
	  else if(x->strs[j] == '5') 
	    {
	      if(x->pcs[5] == x->max_outlet)
		x->pcs[5] = i;
	      else
		error("PC %d already assigned", 5);
	    }
	  else if(x->strs[j] == '6') 
	    {
	      if(x->pcs[6] == x->max_outlet)
		x->pcs[6] = i;
	      else
		error("PC %d already assigned", 6);
	    }
	  else if(x->strs[j] == '7') 
	    {
	      if(x->pcs[7] == x->max_outlet)
		x->pcs[7] = i;
	      else
		error("PC %d already assigned", 7);
	    }
	  else if(x->strs[j] == '8') 
	    {
	      if(x->pcs[8] == x->max_outlet)
		x->pcs[8] = i;
	      else
		error("PC %d already assigned", 8);
	    }
	  else if(x->strs[j] == '9') 
	    {
	      if(x->pcs[9] == x->max_outlet)
		x->pcs[9] = i;
	      else
		error("PC %d already assigned", 9);
	    }
	  else if(x->strs[j] == 't') 
	    {
	      if(x->pcs[10] == x->max_outlet)
		x->pcs[10] = i;
	      else
		error("PC %d already assigned", 10);
	    }
	  else if(x->strs[j] == 'e') 
	    {
	      if(x->pcs[11] == x->max_outlet)
		x->pcs[11] = i;
	      else
		error("PC %d already assigned", 11);
	    }
	  else
	    error("Invalid input, ignoring...");
	  
	  // ignore anything else
	}
      //post("");
    }
  // end of pc assignment loop
  // create argc + 1 outlets
  for(int i = 0; i < x->max_outlet + 1; i++)
    {
      x->outlet[i] = outlet_new(&x->fte, &s_float);
    }
  return (void *)x;
}

void forte_setup(void)
{
  forte_class = class_new(gensym("forte"), (t_newmethod)forte_new, 0, 
			  sizeof(t_forte), CLASS_DEFAULT, A_GIMME, 0);
  class_addfloat(forte_class, forte_float);
  class_addmethod(forte_class, forte_update, gensym("root"), A_GIMME, 0);
  post(version);
}
