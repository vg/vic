/************************************************************************
 *
 *  snr.c, part of tmn (TMN encoder)
 *
 *  Copyright (C) 1997  University of BC, Canada
 *
 *  Contacts: 
 *  Michael Gallant                   <mikeg@ee.ubc.ca>
 *  Guy Cote                          <guyc@ee.ubc.ca>
 *  Berna Erol                        <bernae@ee.ubc.ca>
 *
 *  UBC Image Processing Laboratory   http://www.ee.ubc.ca/image
 *  2356 Main Mall                    tel.: +1 604 822 4051
 *  Vancouver BC Canada V6T1Z4        fax.: +1 604 822 5949
 *
 *  Copyright (C) 1995, 1996  Telenor R&D, Norway
 *
 *  Contacts:
 *  Robert Danielsen                  <Robert.Danielsen@nta.no>
 *
 *  Telenor Research and Development  http://www.nta.no/brukere/DVC/
 *  P.O.Box 83                        tel.:   +47 63 84 84 00
 *  N-2007 Kjeller, Norway            fax.:   +47 63 81 00 76
 *
 ************************************************************************/
/* Disclaimer of Warranty
 * 
 * These software programs are available to the user without any license fee
 * or royalty on an "as is" basis. The University of British Columbia
 * disclaims any and all warranties, whether express, implied, or
 * statuary, including any implied warranties or merchantability or of
 * fitness for a particular purpose.  In no event shall the
 * copyright-holder be liable for any incidental, punitive, or
 * consequential damages of any kind whatsoever arising from the use of
 * these programs.
 * 
 * This disclaimer of warranty extends to the user of these programs and
 * user's customers, employees, agents, transferees, successors, and
 * assigns.
 * 
 * The University of British Columbia does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any
 * third-party patents.
 * 
 * Commercial implementations of H.263, including shareware, are subject to
 * royalty fees to patent holders.  Many of these patents are general
 * enough such that they are unavoidable regardless of implementation
 * design.
 * 
 */



#include"sim.h"

#include<math.h>

//SV-XXX: defined UNUSED() macro for unused variables
#ifndef UNUSED
#define UNUSED(x) (x) = (x)
#endif


/**********************************************************************
 *
 *	Name:        SNRcomp
 *	Description:        Compares two image files using SNR
 *                      No conversion to 422
 *
 *	Input:
 *	Returns:
 *	Side effects:
 *
 *	Date: 930711	Author: <klillevo@mailbox.jf.intel.com>
 *
 ***********************************************************************/



void ComputeSNR (PictImage * im1, PictImage * im2, Results * res, int pict_type, int write)
{
  FILE *out = NULL;
  int n;
  register int m;
  int quad, quad_Cr, quad_Cb, diff;
  PictImage *diff_image = NULL;
  /* Diff. image written to diff_filename */
  char *diff_filename = DEF_DIFFILENAME;

  UNUSED(pict_type); //SV-XXX: unused variable

  if (write)
  {
    out = fopen (diff_filename, "ab");
    diff_image = (PictImage *) malloc (sizeof (PictImage));
    diff_image->lum = (unsigned char *) malloc (sizeof (char) * sed_pels * sed_lines);
    diff_image->Cr  = (unsigned char *) malloc (sizeof (char) * sed_pels * sed_lines / 4);
    diff_image->Cb  = (unsigned char *) malloc (sizeof (char) * sed_pels * sed_lines / 4);
  }
  quad = 0;
  quad_Cr = quad_Cb = 0;
  /* Luminance */
  quad = 0;
  for (n = 0; n < sed_lines; n++)
    for (m = 0; m < sed_pels; m++)
    {
      diff = *(im1->lum + m + n * sed_pels) - *(im2->lum + m + n * sed_pels);
      if (write)
        *(diff_image->lum + m + n * sed_pels) = 10 * diff + 128;
      quad += diff * diff;
    }

  res->SNR_l = (float) quad / (float) (sed_pels * sed_lines);
  if (res->SNR_l)
  {
    res->SNR_l = (float) (255 * 255) / res->SNR_l;
    res->SNR_l = 10 * (float) log10 (res->SNR_l);
  } else
    res->SNR_l = (float) 99.99;

  /* Chrominance */
  for (n = 0; n < sed_lines / 2; n++)
    for (m = 0; m < sed_pels / 2; m++)
    {
      quad_Cr += (*(im1->Cr + m + n * sed_pels / 2) - *(im2->Cr + m + n * sed_pels / 2)) *
        (*(im1->Cr + m + n * sed_pels / 2) - *(im2->Cr + m + n * sed_pels / 2));
      quad_Cb += (*(im1->Cb + m + n * sed_pels / 2) - *(im2->Cb + m + n * sed_pels / 2)) *
        (*(im1->Cb + m + n * sed_pels / 2) - *(im2->Cb + m + n * sed_pels / 2));
      if (write)
      {
        *(diff_image->Cr + m + n * sed_pels / 2) =
          (*(im1->Cr + m + n * sed_pels / 2) - *(im2->Cr + m + n * sed_pels / 2)) * 10 + 128;
        *(diff_image->Cb + m + n * sed_pels / 2) =
          (*(im1->Cb + m + n * sed_pels / 2) - *(im2->Cb + m + n * sed_pels / 2)) * 10 + 128;
      }
    }

  res->SNR_Cr = (float) quad_Cr / (float) (sed_pels * sed_lines / 4);
  if (res->SNR_Cr)
  {
    res->SNR_Cr = (float) (255 * 255) / res->SNR_Cr;
    res->SNR_Cr = 10 * (float) log10 (res->SNR_Cr);
  } else
    res->SNR_Cr = (float) 99.99;

  res->SNR_Cb = (float) quad_Cb / (float) (sed_pels * sed_lines / 4);
  if (res->SNR_Cb)
  {
    res->SNR_Cb = (float) (255 * 255) / res->SNR_Cb;
    res->SNR_Cb = 10 * (float) log10 (res->SNR_Cb);
  } else
    res->SNR_Cb = (float) 99.99;

  if (write)
  {
    fwrite (diff_image->lum, sizeof (char), sed_pels * sed_lines, out);
    fwrite (diff_image->Cr, sizeof (char), sed_pels * sed_lines / 4, out);
    fwrite (diff_image->Cb, sizeof (char), sed_pels * sed_lines / 4, out);
    free (diff_image->lum);
    free (diff_image->Cr);
    free (diff_image->Cb);
    free (diff_image);
    fclose (out);
  }
  return;
}
