#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ccsds_dll_cc.h>
#include <gr_io_signature.h>
#include <math.h>
#include <complex>
#include <cstdlib>
#include <fftw3.h>

#define DLL_DEBUG

ccsds_dll_cc_sptr ccsds_make_dll_cc(unsigned int osf, float gamma) {
    return ccsds_dll_cc_sptr (new ccsds_dll_cc(osf,gamma) );
}

ccsds_dll_cc::ccsds_dll_cc (unsigned int osf, float gamma)
  : gr_block ("ccsds_dll_cc",
	gr_make_io_signature (1, 1, sizeof (gr_complex)),
	//gr_make_io_signature3 (1, 3, sizeof (gr_complex), sizeof (float), sizeof (float)))
	gr_make_io_signature (1, 1, sizeof (gr_complex))), d_OSF(osf), d_OSF_HALF((float)osf/2.0f), d_GAMMA(gamma)
{

	// not initialized, we will init the other variables in general_work
	d_init = false;

	#ifdef DLL_DEBUG
		dbg_count = 0;

		dbg_file_o = fopen("debug_dll_orig.dat","w");
		dbg_file_i = fopen("debug_dll_intp.dat","w");
		dbg_file_s = fopen("debug_dll_symb.dat","w");
		dbg_file_t = fopen("debug_dll_tauh.dat","w");
		if(dbg_file_o == NULL || dbg_file_i == NULL || dbg_file_s == NULL || dbg_file_i == NULL) {
			fprintf(stderr,"ERROR DLL: can not open debug file\n");
			exit(EXIT_FAILURE);
			return;
		}
		fprintf(dbg_file_o, "#k,real(y_hat),imag(y_hat)\n");
		fprintf(dbg_file_s, "#k,real(y_hat),imag(y_hat)\n");
		fprintf(dbg_file_i, "#k,real(y_hat),imag(y_hat)\n");
		fprintf(dbg_file_t, "#k,e_tau,tau_hat,l,mu\n");
	#endif
}

ccsds_dll_cc::~ccsds_dll_cc () {
	#ifdef DLL_DEBUG
		fflush(dbg_file_o);
		fflush(dbg_file_s);
		fflush(dbg_file_i);
		fflush(dbg_file_t);

		fclose(dbg_file_o);
		fclose(dbg_file_s);
		fclose(dbg_file_i);
		fclose(dbg_file_t);
	#endif
}

float ccsds_dll_cc::get_frac(float value) {
	return std::fmod(value,1.0f);
}

int ccsds_dll_cc::get_int(float value) {
	return (int) std::floor(value);
}


void ccsds_dll_cc::forecast(int noutput_items,gr_vector_int &ninput_items_required){
	// basically decimating by a factor of d_OSF, but for interpolation we
	// need to see 2 more samples "into the future"
	ninput_items_required[0] = d_OSF * noutput_items + 4;
}

inline void ccsds_dll_cc::to_real(float *out, const gr_complex *in, const unsigned int num) {
	for(unsigned int i=0;i<num;i++) {
		out[i] = std::real(in[i]);
	}
	return;
}

inline void ccsds_dll_cc::to_imag(float *out, const gr_complex *in, const unsigned int num) {
	for(unsigned int i=0;i<num;i++) {
		out[i] = std::imag(in[i]);
	}
	return;
}

/*
	Gardner timing error detector for passband signals

	e(k) = re{ [ y(kT-T+tau[k-1]) - y(kT+tau[k]) ] y*(kT-T/2+tau[k-1]) }
	Mengali p. 431
*/
float ccsds_dll_cc::gardner(gr_complex previous, gr_complex intermediate, gr_complex current) {
	return std::real( (previous-current) * std::conj(intermediate) );
}

gr_complex ccsds_dll_cc::interpolate_cvalue(const gr_complex *y, float mu) {
	const unsigned int num = 4;
	float *real = 0;
	float *imag = 0;

	real = new float[num];
	imag = new float[num];

	to_real(real, y, num);
	to_imag(imag, y, num);

	gr_complex ret = gr_complex(interpolate_value(real, mu), interpolate_value(imag, mu));
	
	delete[] real;
	delete[] imag;


	return ret;
}

inline float ccsds_dll_cc::interpolate_value(const float *y, float mu) {
	float a,b,c,d;

	a = y[3] - y[2] - y[0] + y[1];
	b = y[0] - y[1] - a;
	c = y[2] - y[0];
	d = y[1];

	return(a*mu*mu*mu+b*mu*mu+c*mu+d);
}

int  ccsds_dll_cc::general_work (int                     noutput_items,
                                gr_vector_int               &ninput_items,
                                gr_vector_const_void_star   &input_items,
                                gr_vector_void_star         &output_items)
{
	const gr_complex *in = (const gr_complex *) input_items[0];
	gr_complex *out = (gr_complex *) output_items[0];
	//float *freq = (float *) output_items[1];
	
	// how many samples can we process?
	unsigned int num = (d_OSF*noutput_items > (unsigned int) ninput_items[0] ) ? ninput_items[0] : noutput_items*d_OSF;
	
	// how many samples to output, when we want to have 2*d_OSF spare
	// samples at the end and one at the front, to ensure the interpolator
	// never has to extrapolate
	//int num_out = std::floor((num-1)/d_OSF) -2;

	// not enough samples for one iteration
	if(num < d_OSF+4) {
		consume_each(0);
		return 0;
	}

	if(!d_init) {
		d_tau_hat = 0.0f;
		d_l = 1;
		d_mu = 0.0f;
		
		#ifdef DLL_DEBUG
			// debug initial parameters
			fprintf(dbg_file_t, "%d,%2.10f,%2.10f,%d,%2.10f\n",0, 0.0f, d_tau_hat, d_l%d_OSF, d_mu);
		#endif

		// retrieve first symbol
		// mu is zero, so take the sample directly
		d_last_interp[PREV] = in[1];

		// update indices
		d_l += get_int(d_OSF_HALF);
		d_mu = get_frac(d_mu + d_OSF_HALF);

		#ifdef DLL_DEBUG
			// debug parameters, updated mu and d_l
			fprintf(dbg_file_t, "%d,%2.10f,%2.10f,%d,%2.10f\n",d_l, 0.0f, d_tau_hat, d_l%d_OSF, d_mu);
		#endif

		// retrieve intermediate sample
		d_last_interp[INTM] = interpolate_cvalue(&in[d_l-1], d_mu);

		// update indices
		d_l += get_int(d_mu + d_OSF_HALF);
		d_mu = get_frac(d_mu + d_OSF_HALF);

		#ifdef DLL_DEBUG
			// debug parameters, update new values
			fprintf(dbg_file_t, "%d,%2.10f,%2.10f,%d,%2.10f\n",d_l, 0.0f, d_tau_hat, d_l%d_OSF, d_mu);
		#endif

		// retrieve second symbol
		d_last_interp[CURR] = interpolate_cvalue(&in[d_l-1], d_mu);

		// update tau_hat
		d_gamma_eps = gardner(d_last_interp[PREV], d_last_interp[INTM], d_last_interp[CURR]);
		if(d_gamma_eps != 0.0f) {
			d_gamma_eps *= d_GAMMA;
		}

		d_tau_hat += d_gamma_eps/(float)d_OSF;
		// update indices
		d_l += get_int(d_mu + d_OSF_HALF + d_gamma_eps);
		d_mu = get_frac(d_mu + d_OSF_HALF + d_gamma_eps);

		#ifdef DLL_DEBUG
			// debug interpolated symbols
			fprintf(dbg_file_i, "%f,%f,%f\n",(float)1.0f           ,std::real(d_last_interp[PREV]),std::imag(d_last_interp[PREV]));
			fprintf(dbg_file_i, "%f,%f,%f\n",(float)1.0f+d_OSF_HALF,std::real(d_last_interp[INTM]),std::imag(d_last_interp[INTM]));
			fprintf(dbg_file_i, "%f,%f,%f\n",(float)1.0f+d_OSF     ,std::real(d_last_interp[CURR]),std::imag(d_last_interp[CURR]));

			fprintf(dbg_file_s, "%f,%f,%f\n",(float)1.0f           ,std::real(d_last_interp[PREV]),std::imag(d_last_interp[PREV]));
			fprintf(dbg_file_s, "%f,%f,%f\n",(float)1.0f+d_OSF     ,std::real(d_last_interp[CURR]),std::imag(d_last_interp[CURR]));
			
			// debug input samples
			for(unsigned int i=0;i<d_l-1;i++) {
				fprintf(dbg_file_o, "%f,%f,%f\n",(float)(dbg_count++),std::real(in[i]),std::imag(in[i]));
			}

			// debug
			fprintf(dbg_file_t, "%f,%2.10f,%2.10f,%d,%2.10f\n",(float)d_OSF+d_OSF_HALF, d_gamma_eps, d_tau_hat, d_l%d_OSF, d_mu);
		#endif

		// initialization done
		d_init = true;

		// output the two first symbols
		out[0] = d_last_interp[PREV];
		out[1] = d_last_interp[CURR];

		// Tell runtime how many input samples we used
		// next intermediate sample will have basepoint index d_l and
		// we nedd one previous sample for interpolation
		consume_each(d_l-1);

		// next block will contain exactly one sample prior to the
		// basepoint sample
		d_l = 1;

		// Tell runtime system how many output items we produced
		return 2;
	} // end init

	// number of samples missing for next intermediate sample
	int missing = 0;
	
	// number of output symbols
	unsigned int num_out = 0;
	while(true) {
		// drop last two interpolants and shift buffer
		d_last_interp[PREV] = d_last_interp[CURR];

		// retrieve intermediate sample
		d_last_interp[INTM] = interpolate_cvalue(&in[d_l-1], d_mu);
		#ifdef DLL_DEBUG
			// debug intermediate sample
			fprintf(dbg_file_i, "%f,%f,%f\n",(float)(dbg_count+d_l+d_mu),std::real(d_last_interp[INTM]),std::imag(d_last_interp[INTM]));
		#endif

		// update indices
		d_l += get_int(d_mu + d_OSF_HALF);
		d_mu = get_frac(d_mu + d_OSF_HALF);

		#ifdef DLL_DEBUG
			// debug parameters for next symbol
			fprintf(dbg_file_t, "%d,%2.10f,%2.10f,%d,%2.10f\n",dbg_count+d_l, d_gamma_eps, d_tau_hat, d_l%d_OSF, d_mu);
		#endif

		// retrieve second symbol
		d_last_interp[CURR] = interpolate_cvalue(&in[d_l-1], d_mu);
		#ifdef DLL_DEBUG
			// debug next symbol
			fprintf(dbg_file_i, "%f,%f,%f\n",(float)(dbg_count+d_l+d_mu),std::real(d_last_interp[CURR]),std::imag(d_last_interp[CURR]));
			fprintf(dbg_file_s, "%f,%f,%f\n",(float)(dbg_count+d_l+d_mu),std::real(d_last_interp[CURR]),std::imag(d_last_interp[CURR]));
		#endif

		// output the new symbol
		out[num_out] = d_last_interp[CURR];
		num_out++;

		// update tau_hat
		d_gamma_eps = d_GAMMA * gardner(d_last_interp[PREV], d_last_interp[INTM], d_last_interp[CURR]);
		d_tau_hat += d_gamma_eps;

		// update indices
		d_l += get_int(d_mu + d_OSF_HALF + d_gamma_eps);
		d_mu = get_frac(d_mu + d_OSF_HALF + d_gamma_eps);

		#ifdef DLL_DEBUG
			// debug parameters for next intermediate sample			
			fprintf(dbg_file_t, "%d,%2.10f,%2.10f,%d,%2.10f\n",dbg_count+d_l, d_gamma_eps, d_tau_hat, d_l%d_OSF, d_mu);
		#endif

		// see if we have enough samples left with the new  tau estimate
		// d_l 		: index of next intermediate symbol
		// get_int(...) : advance to next symbol
		// 3		: additional samples for interpolation
		missing = d_l + get_int(d_mu + d_OSF_HALF) + 3 - num;
		missing = (missing > 0) ? missing : 0;
		if( missing > 0) {
			break;
		}
	}

	#ifdef DLL_DEBUG
		// debug original incomming samples
		for(unsigned int i=0;i<d_l-1-missing;i++) {
			fprintf(dbg_file_o, "%f,%f,%f\n",(float)(dbg_count++),std::real(in[i]),std::imag(in[i]));
		}
	#endif

	// Tell runtime how many input samples we used
	// try to get rid of as many samples as possible, so next call there is
	// only one sample before the basepoint index left (d_l-1). But do not
	// consume more samples than we have (missing).
	consume_each(d_l-1-missing);

	// next block will contain exactly one sample prior to the basepoint
	// sample or $missing$ more, if we could not consume the full last block
	// above
	d_l = 1+missing;

	// Tell runtime system how many output items we produced
	return num_out;
}
