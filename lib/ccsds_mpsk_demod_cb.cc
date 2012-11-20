#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ccsds_mpsk_demod_cb.h>
#include <gr_io_signature.h>
#include <stdio.h>
#include <math.h>

ccsds_mpsk_demod_cb_sptr ccsds_make_mpsk_demod_cb (unsigned int M)
{
    return ccsds_mpsk_demod_cb_sptr (new ccsds_mpsk_demod_cb (M));
}

ccsds_mpsk_demod_cb::ccsds_mpsk_demod_cb (unsigned int M)
  : gr_block ("ccsds_mpsk_demod_cb",
	gr_make_io_signature (1, 1, sizeof (gr_complex)),
	gr_make_io_signature (1, 1, sizeof (char)))
{
	d_M = M;

	if(M==2) {
		detect_symbol = &ccsds_mpsk_demod_cb::detect_bpsk_symbol;
	} else if (M==4) {
		detect_symbol = &ccsds_mpsk_demod_cb::detect_qpsk_symbol;
	} else {
		detect_symbol = &ccsds_mpsk_demod_cb::detect_mpsk_symbol;
	}


	// create phasor for rotation between the symbols
	const gr_complex diff_phasor = gr_complex(cos(2*M_PI/d_M),sin(2*M_PI/d_M));
	// initial phasor is on the real axis, except for QPSK (according to ECSS standard)
	gr_complex phasor = (d_M!=4) ? gr_complex(1.0f,0.0f) : gr_complex(1.0f,1.0f);
	// make sure we handle only with real phasors
	phasor = phasor / std::abs(phasor);

	// allocate memory for the constellation points
	
	d_constellation = (gr_complex *)malloc(d_M * sizeof(gr_complex));
	if (d_constellation == 0) {
		printf("ERROR: Out of memory\n");
		return;
	}

	// go through all constellation points
	for(unsigned int i=0;i<d_M;i++) {
		unsigned int i_gray = (i >> 1) ^ i; // From: http://en.wikipedia.org/wiki/Grey_code#Converting_to_and_from_Gray_code

		// check if the next access is valid (not true for e.g M=3)
		if(i_gray >= d_M) {
			fprintf(stderr,"ERROR: modulation order M=%d not supported. M needs to be a power of two.\n",d_M);
			exit(EXIT_FAILURE);
			return;
		}		
		// store constellation point
		d_constellation[i_gray] = phasor;
		// rotate to next point
		phasor *= diff_phasor;
	}
}

ccsds_mpsk_demod_cb::~ccsds_mpsk_demod_cb ()
{
	// free memory allocated for the constellation
	free(d_constellation);
}

char ccsds_mpsk_demod_cb::detect_bpsk_symbol(gr_complex symbol) {
	if(std::real(symbol) > 0) {
		return 0;
	} else {
		return 1;
	}
}

char ccsds_mpsk_demod_cb::detect_qpsk_symbol(gr_complex symbol) {
	const float re = std::real(symbol);
	const float im = std::imag(symbol);

	if(im>re) {
		if(re>-im) {
			return 0;
		} else { // re <= -im
			return 2;
		}
	} else { // im <= re)
		if(re>-im) {
			return 1;
		} else { // re <= -im
			return 3;
		}
	}
}

// Brute Force per symbol
char ccsds_mpsk_demod_cb::detect_mpsk_symbol(gr_complex symbol) {
	float min_dist = INFINITY;
	float dist;
	char min_byte = 0;

	for(char byte=0;byte<d_M;byte++) {
		dist = std::abs(d_constellation[byte]-symbol);
		min_dist = (dist >= min_dist) ? min_dist : dist;
		min_byte = (dist >= min_dist) ? min_byte : byte;
	}

	return min_byte;
}

int  ccsds_mpsk_demod_cb::general_work (int                     noutput_items,
                                gr_vector_int               &ninput_items,
                                gr_vector_const_void_star   &input_items,
                                gr_vector_void_star         &output_items)
{
	const gr_complex *in = (const gr_complex *) input_items[0];
	char *out = (char *) output_items[0];

	// counter
	unsigned int i;
	for(i=0;i<noutput_items && i<ninput_items[0];i++) {
		out[i] = (this->*detect_symbol)(in[i]);
	}

	consume_each(i);

	// Tell runtime system how many output items we produced.
	return i;
}
