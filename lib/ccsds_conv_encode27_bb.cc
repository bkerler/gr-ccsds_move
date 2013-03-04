#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ccsds_conv_encode27_bb.h>
#include <gr_io_signature.h>
#include <stdio.h>

ccsds_conv_encode27_bb_sptr
ccsds_make_conv_encode27_bb (const unsigned char gen_poly_c1, const unsigned char gen_poly_c2, ccsds_conv_encode27_punct::punct_t puncturing_type) {
    return ccsds_conv_encode27_bb_sptr (new ccsds_conv_encode27_bb (gen_poly_c1, gen_poly_c2, puncturing_type));
}

float ccsds_conv_encode27_bb::get_rate(ccsds_conv_encode27_punct::punct_t puncturing_type) {
	float rate;
	switch(puncturing_type) {
		case ccsds_conv_encode27_punct::NONE:	 	rate = 1.0f/2.0f; break;
		case ccsds_conv_encode27_punct::ECSS_23:	rate = 2.0f/3.0f; break;
		case ccsds_conv_encode27_punct::ECSS_34:	rate = 3.0f/4.0f; break;
		case ccsds_conv_encode27_punct::ECSS_56:	rate = 5.0f/6.0f; break;
		case ccsds_conv_encode27_punct::ECSS_78:	rate = 7.0f/8.0f; break;
		default:			rate = 0.0f;    fprintf(stderr,"ERROR CONV ENCODE: unknown puncturing type.\n");
	}
	return rate;
}

ccsds_conv_encode27_bb::ccsds_conv_encode27_bb(const unsigned char gen_poly_c1, const unsigned char gen_poly_c2, ccsds_conv_encode27_punct::punct_t puncturing_type)
  : gr_block ("ccsds_conv_encode27_bb",
	gr_make_io_signature (1, 1, sizeof(unsigned char)),
	gr_make_io_signature (1, 1, sizeof(unsigned char))),
    ccsds_conv_encode27_punct_aux(gen_poly_c1, gen_poly_c2, puncturing_type),
    d_RATE(get_rate(puncturing_type))
{
	d_buffer_count = 0;
	d_buffer = 0x0000;
}

ccsds_conv_encode27_bb::~ccsds_conv_encode27_bb ()
{
	// nothing to do
}

void ccsds_conv_encode27_bb::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
	ninput_items_required[0] = std::ceil((noutput_items*8.0f-d_buffer_count) * d_RATE /8.0f);
	//printf("CONVE ENCODE forecast: need %d inputs for %d output. Still have %u bits in buffer\n",ninput_items_required[0], noutput_items, d_buffer_count);
	return;
}

int  ccsds_conv_encode27_bb::general_work (int       noutput_items,
			 gr_vector_int 		     &ninput_items,
                         gr_vector_const_void_star   &input_items,
                         gr_vector_void_star         &output_items)
{
	const unsigned char *in = (const unsigned char *) input_items[0];
	unsigned char *out = (unsigned char *) output_items[0];

	//printf("CONV ENCODE27: %d bytes requested, %d bytes given, %d bits in buffer\n",noutput_items, ninput_items[0], d_buffer_count);

	unsigned int num_in = 0;
	unsigned int num_out = 0;

	// is there still a full byte stored from last time?	
	if(d_buffer_count >= 8) {
		// do we have memory to output?
		if(noutput_items > 0) {
			// output buffer
			out[0] = uint8_t ((d_buffer >> 8) & 0x00FF);
			// increase counter
			num_out++;
			// shift still unused bits at MSB position in buffer
			d_buffer = d_buffer << 8;
			// update buffer counter
			d_buffer_count -= 8;
		} else {
			// no output memory
			fprintf(stderr,"ERROR CONV ENCODE: called general_work() but no output memory allocated .\n");
			return 0;
		}
	}

	// At this point d_buffer contains at most 7 bits.

	while((unsigned int)ninput_items[0] > num_in && (unsigned int)noutput_items > num_out) {
		// There is at least one output byte allocated and at least one
		// input byte available. Encode can create at most 16 new bits
		// so we will create one or two new output bits.

		uint16_t bits_enc;
		uint8_t num_bits_out;

		// read in new byte and encode it
		encode_punct(bits_enc, num_bits_out, in[num_in]);
		num_in++;

		// are there enough bits left for a full byte and is there still memory to output it?
		// this loop runs at least once since encode will create at least 8 bits.
		while(d_buffer_count + num_bits_out >= 8 && (unsigned int)noutput_items > num_out) {
			// yes, shift new bits to make place for the
			// d_buffer_count old bytes at the front

			// first make sure that the bits are left aligned
			uint16_t new_bits = bits_enc << (16-num_bits_out);
			// then shift back to the right to make space for the
			// data and make sure the left 8 bits are empty as we
			// want to cast to byte.
			new_bits = new_bits >> (d_buffer_count + 8);

			// add bits from buffer
			new_bits |= (d_buffer >> 8);

			// update bit count
			num_bits_out -= (8-d_buffer_count);

			// empty buffer
			d_buffer = 0;
			d_buffer_count = 0;

			// output new byte
			out[num_out] = (uint8_t) (new_bits & 0x00FF);
			num_out++;
		}

		// At this point d_buffer will be empty, because the above loop
		// will run at least once, which will empty the buffer.

		// are there a few bits left that did not fit into a full byte?
		if(num_bits_out > 0) {
			// align them to start at the MSB of a byte.
			d_buffer = bits_enc << (16-num_bits_out);
			d_buffer_count = num_bits_out;
		}
	}

	//printf("    %u bytes read, %u bytes written, %d bits in buffer\n",num_in,num_out,d_buffer_count);

	// consume input stream
	consume_each(num_in);

	// Tell runtime system how many output items we produced.
	return num_out;
}
