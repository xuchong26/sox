
/*
 * July 5, 1991
 * Copyright 1991 Lance Norskog And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained. 
 * Lance Norskog And Sundry Contributors are not responsible for 
 * the consequences of using this software.
 */

/*
 * Sound Tools Low-Pass effect file.
 *
 * Algorithm:  Recursive single pole lowpass filter
 *
 * Reference: The Scientist and Engineer's Guide to Digital Signal Processing
 *
 * 	output[N] = input[N] * A + output[N-1] * B
 *
 * 	X = exp(-2.0 * pi * Fc)
 * 	A = 1 - X
 * 	B = X
 * 	Fc = cutoff freq / sample rate
 *
 * Mimics an RC low-pass filter:   
 *
 *     ---/\/\/\/\----------->
 *                    |
 *                   --- C
 *                   ---
 *                    |
 *                    |
 *                    V
 *
 */

#include <math.h>
#include "st.h"

/* Private data for Lowpass effect */
typedef struct lowpstuff {
	float	cutoff;
	double	A, B;
	double	outm1;
} *lowp_t;

/*
 * Process options
 */
int st_lowp_getopts(effp, n, argv) 
eff_t effp;
int n;
char **argv;
{
	lowp_t lowp = (lowp_t) effp->priv;

	if ((n < 1) || !sscanf(argv[0], "%f", &lowp->cutoff))
	{
		st_fail("Usage: lowp cutoff");
		return (ST_EOF);
	}
	return (ST_SUCCESS);
}

/*
 * Prepare processing.
 */
int st_lowp_start(effp)
eff_t effp;
{
	lowp_t lowp = (lowp_t) effp->priv;
	if (lowp->cutoff > effp->ininfo.rate / 2)
	{
		st_fail("Lowpass: cutoff must be < sample rate / 2 (Nyquest rate)\n");
		return (ST_EOF);
	}

	lowp->B = exp((-2.0 * M_PI * (lowp->cutoff / effp->ininfo.rate)));
	lowp->A = 1 - lowp->B;
	lowp->outm1 = 0.0;
	return (ST_SUCCESS);
}

/*
 * Processed signed long samples from ibuf to obuf.
 * Return number of samples processed.
 */

int st_lowp_flow(effp, ibuf, obuf, isamp, osamp)
eff_t effp;
LONG *ibuf, *obuf;
LONG *isamp, *osamp;
{
	lowp_t lowp = (lowp_t) effp->priv;
	int len, done;
	double d;
	LONG l;

	len = ((*isamp > *osamp) ? *osamp : *isamp);

	for(done = 0; done < len; done++) {
		l = *ibuf++;
		d = lowp->A * l + lowp->B * lowp->outm1;
		if (d < -2147483647L)
		    d = -2147483647L;
		else if (d > 2147483647L)
		    d = 2147483647L;
		lowp->outm1 = l;
		*obuf++ = d;
	}
	*isamp = len;
	*osamp = len;
	return (ST_SUCCESS);
}

/*
 * Do anything required when you stop reading samples.  
 * Don't close input file! 
 */
int st_lowp_stop(effp)
eff_t effp;
{
	/* nothing to do */
    return (ST_SUCCESS);
}

