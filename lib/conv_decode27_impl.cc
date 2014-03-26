#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <conv_decode27_impl.h>
#include <gnuradio/io_signature.h>
#include <stdio.h>
#include <boost/scoped_array.hpp>
#include "hexstring_to_binary.h"

namespace gr {
  namespace ccsds {
    
    unsigned int conv_decode27_impl::get_rate_in(conv_encode27_punct::punct_t puncturing_type) {
    	switch(puncturing_type) {
    		case conv_encode27_punct::NONE:		return 16; // 16 bits in, 1 byte out
    		case conv_encode27_punct::ECSS_23:	return 12; // 12 bits in, 1 byte out
    		case conv_encode27_punct::ECSS_34:	return 32; // 32 bits in, 3 byte out
    		case conv_encode27_punct::ECSS_56:	return 48; // 48 bits in, 5 byte out
    		case conv_encode27_punct::ECSS_78:	return 64; // 64 bits in, 7 byte out
    		default:			fprintf(stderr,"ERROR CONV DECODE27: invalid puncturing type %d\n",puncturing_type);
    						exit(EXIT_FAILURE);
    						return 0;
    	}
    }
    
    unsigned int conv_decode27_impl::get_rate_out(conv_encode27_punct::punct_t puncturing_type) {
    	switch(puncturing_type) {
    		case conv_encode27_punct::NONE:		return 1; // 16 bits in, 1 byte out
    		case conv_encode27_punct::ECSS_23:	return 1; // 12 bits in, 1 byte out
    		case conv_encode27_punct::ECSS_34:	return 3; // 32 bits in, 3 byte out
    		case conv_encode27_punct::ECSS_56:	return 5; // 48 bits in, 5 byte out
    		case conv_encode27_punct::ECSS_78:	return 7; // 64 bits in, 7 byte out
    		default:			fprintf(stderr,"ERROR CONV DECODE27: invalid puncturing type %d\n",puncturing_type);
    						exit(EXIT_FAILURE);
    						return 0;
    	}
    }
    
    unsigned int conv_decode27_impl::get_num_inputs_req(conv_encode27_punct::punct_t puncturing_type, const unsigned int num_out) {
    
    	const unsigned int pattern_len = conv_encode27_punct::get_pattern_len(puncturing_type);
    	boost::shared_ptr<bool[]> pattern = conv_encode27_punct::get_pattern(puncturing_type);
    
    	unsigned int count_in = 0;
    	unsigned int count_pattern = 0;
    
    	// go through every output bit and determine if it will be generated by
    	// the input stream or the depuncturing.
    	for(unsigned int count_out=0;count_out<num_out;count_out++) {
    		// true means bit is not dropped, so we need the input bit
    		// false means bi is dropped, so we can retrieve it again (erasure symbol) without an input bit
    		count_in += pattern[count_pattern] ? 1 : 0;
    
    		// update wrapping counter for the pattern.
    		count_pattern = (count_pattern+1) % pattern_len;
    	}
    
    	return count_in;
    }
    
    
    int conv_decode27_impl::get_start_state(std::string ASM) {
    
    	// get length of ASM
    	const int asm_len = ASM.length() / 2;
    
    	// Get ASM bytes out of string
    	boost::scoped_array<unsigned char> tmp(new unsigned char[asm_len]);
    	hexstring_to_binary(&ASM, tmp.get());
    
    	// We need the last 6 bits of the ASM
    	return (int)(tmp[asm_len-1] & 0x3F); // 0x3F = 0011 1111
    }
    
    unsigned int conv_decode27_impl::get_term_state(std::string ASM) {
    
    	// get length of ASM
    	const int asm_len = ASM.length() / 2;
    
    	// Get ASM bytes out of string
    	boost::scoped_array<unsigned char> tmp(new unsigned char[asm_len]);
    	hexstring_to_binary(&ASM, tmp.get());
    
    	// We need the first 6 bits of the ASM
    	return (unsigned int) ( (tmp[0] >> 2)  & 0x3F ); // 0x3F = 0011 1111
    }
    
    int conv_decode27_impl::convert_poly(const uint8_t in) {
    
    	// start with empty byte
    	uint8_t ret = 0x00;
    
    	// copy 7 bits that store the polynom information
    	for(unsigned int i=0;i<7;i++) {
    
    		// shift ret's LSB to the next higher position
    		ret = ret << 1;
    
    		// copy i-th in bit (counted from LSB) to ret's LSB
    		ret |= 0x01 & (in>>i);
    
    
    	}
    
    	// take care of the sign
    	if(in & 0x80) {
    		return -(int)ret;
    	} else {
    		return (int)ret;
    	}
    }
    
    conv_decode27::sptr
    conv_decode27::make (const unsigned char gen_poly_c1, const unsigned char gen_poly_c2, conv_encode27_punct::punct_t puncturing_type, const unsigned int block_len, std::string ASM) {
        return gnuradio::get_initial_sptr (new conv_decode27_impl (gen_poly_c1, gen_poly_c2, puncturing_type, block_len, ASM));
    }
    
    conv_decode27_impl::conv_decode27_impl(const unsigned char gen_poly_c1, const unsigned char gen_poly_c2, conv_encode27_punct::punct_t puncturing_type, const unsigned int block_len, std::string ASM)  
    : gr::sync_block ("conv_decode27",
    	gr::io_signature::make (0, 0, sizeof(unsigned char)),
    	gr::io_signature::make (0, 0, sizeof(unsigned char))),
    	d_PUNCT_TYPE(puncturing_type),
    	d_RATE_NUM_IN(get_rate_in(d_PUNCT_TYPE)),
    	d_RATE_NUM_OUT(get_rate_out(d_PUNCT_TYPE)),
    	d_BLOCK_NUM_BITS_IN(get_num_inputs_req(d_PUNCT_TYPE, 2*(block_len + 6))),
    	d_BLOCK_NUM_BITS_IN_UNPUNC(2*(block_len+6)),
    	d_BLOCK_NUM_BITS_OUT(block_len),
    	d_START_STATE(get_start_state(ASM)),
    	d_TERM_STATE(get_term_state(ASM)),
    	d_PATTERN_LEN(conv_encode27_punct::get_pattern_len(d_PUNCT_TYPE))
    {
    	// Allocate buffer memory to store a complete depunctured soft byte sequence plus the 12 tail symbols
    	d_buffer = new unsigned char[d_BLOCK_NUM_BITS_IN_UNPUNC];
    
    	d_punct_pattern = conv_encode27_punct::get_pattern(d_PUNCT_TYPE);
    
    	// Set CPU mode of libfec
    	Cpu_mode = SSE2;
    
    	// Translate polynomials
    	int polys[2] = {convert_poly(gen_poly_c1),convert_poly(gen_poly_c2)};
    	// set polynomials
    	FEC_FUNC_VITERBI27_SET_POLYNOMIAL(polys);
    
    	// output one byte per iteration
    	d_viterbi = FEC_FUNC_VITERBI27_CREATE((int)d_BLOCK_NUM_BITS_OUT);
    
    	if(d_viterbi == NULL) {
    		fprintf(stderr,"ERROR CONV DECODE27: Unable to create viterbi decoder\n");
    		exit(EXIT_FAILURE);
    		return;
    	}
    
    	// register output port
    	message_port_register_out(pmt::mp("out"));
    
    	// register input type
    	message_port_register_in(pmt::mp("in"));
    	set_msg_handler(pmt::mp("in"), boost::bind(&conv_decode27_impl::process_message, this, _1));
    }
    
    conv_decode27_impl::~conv_decode27_impl(void) {
    
    	// free viterbi decoder resources
    	FEC_FUNC_VITERBI27_DELETE(d_viterbi);
    
    	delete[] d_buffer;
    }
    
    void conv_decode27_impl::process_message(pmt::pmt_t msg_in) {
    	// check for EOF
    	if(pmt::is_eof_object(msg_in)) {
    		message_port_pub( pmt::mp("out"), pmt::PMT_EOF );
    		return;
    	}
    
    	// check that input is a pair value
    	if(!pmt::is_pair(msg_in)) {
    		fprintf(stderr,"WARNING CONV DECODE27: expecting message of type pair, skipping.\n");
    		return;
    	}
    
    	const pmt::pmt_t hdr = pmt::car(msg_in);
    	const pmt::pmt_t msg = pmt::cdr(msg_in);
    
    	// check that input header is a dictionary
    	if(!pmt::is_dict(hdr)) {
    		fprintf(stderr,"WARNING CONV DECODE27: expecting message header of type dict, skipping.\n");
    		return;
    	}
    
    	// check that input data is a float vector
    	if(!pmt::is_f32vector(msg)) {
    		fprintf(stderr,"WARNING CONV DECODE27: expecting message data of type f32vector, skipping.\n");
    		return;
    	}
    
    	// check that input has the expected length
    	if(pmt::length(msg) != d_BLOCK_NUM_BITS_IN) {
    		fprintf(stderr,"WARNING CONV DECODE27: expecting message of %u floats, got %u, skipping.\n",d_BLOCK_NUM_BITS_IN,pmt::length(msg));
    		return;
    	}
    
    	// check that we output full bytes
    	if(d_BLOCK_NUM_BITS_OUT % 8 != 0) {
    		fprintf(stderr,"WARNING CONV DECODE27: Number of output bits %u is not a multiple of 8, so data can't be put into packed bytes.\n",d_BLOCK_NUM_BITS_OUT);
    		return;
    	}
    
    	//
    	// Message contains d_BLOCK_NUM_BITS_IN floats and will generate an
    	// integer multiple of 8 bits output
    	//
    
    	// Fill buffer with unpunctured and converted bytes
    	size_t num_softbits = d_BLOCK_NUM_BITS_IN;
    	unpuncture_and_convert(d_buffer, pmt::f32vector_elements(msg, num_softbits));
    
    	/// init decoder
    	FEC_FUNC_VITERBI27_INIT(d_viterbi, d_START_STATE);
    
    	// fill in data
    	FEC_FUNC_VITERBI27_UPDATE_BLK(d_viterbi, d_buffer, d_BLOCK_NUM_BITS_OUT+6);
    
    	// allocate memory for the decoded bits (will be put into packed bytes
    	// by the library and we checked that the number of bits is a multiple
    	// of 8 above.
    	unsigned char *data_dec = new unsigned char[d_BLOCK_NUM_BITS_OUT/8];
    
    	// Let the decoder decode the corrected byte sequence
    	FEC_FUNC_VITERBI27_CHAINBACK(d_viterbi, data_dec, d_BLOCK_NUM_BITS_OUT, d_TERM_STATE);
    
    	// Create new message data containing the decoded bytes as BLOB
    	pmt::pmt_t msg_out_data = pmt::make_blob(data_dec, d_BLOCK_NUM_BITS_OUT/8);
    						
    	// Construct the new message using the received header
    	pmt::pmt_t msg_out = pmt::cons(hdr, msg_out_data);
    
    	// Post message on output port
    	message_port_pub( pmt::mp("out"), msg_out );
    
    	// Free buffer of decoded data
    	delete[] data_dec;
    
    	return;
    }
    
    inline unsigned char convert_softbit_fb(const float softbit) {
    	const float new_val = round(softbit * 127.5f + 127.5);
    	return (unsigned char) std::max(0.0f, std::min(255.0f, new_val));
    }
    
    void conv_decode27_impl::unpuncture_and_convert(unsigned char *out, const float *in) {
    	
    	if(d_PUNCT_TYPE == conv_encode27_punct::NONE) {
    		// No unpuncturing, just conversion
    
    		for(unsigned int i=0;i<d_BLOCK_NUM_BITS_IN_UNPUNC;i++) {
    			out[i] = convert_softbit_fb(in[i]);
    		}
    
    		return;
    
    	} else {
    		// Unpuncturing and conversion
    
    		unsigned int punct_count = 0;
    		unsigned int in_count = 0;
    
    		for(unsigned int i=0;i<d_BLOCK_NUM_BITS_IN_UNPUNC;i++) {
    
    			// bit (true), or erasure (false)
    			const bool action = d_punct_pattern[punct_count];
    
    			out[i]    = action ? convert_softbit_fb(in[in_count]) : 128u;
    			in_count += action ? 1u : 0u;
    
    			punct_count = (punct_count+1) % d_PATTERN_LEN;
    		}
    
    		return;
    	}
    }
    
    int conv_decode27_impl::work(int /*noutput_items*/, gr_vector_const_void_star& /*input_items*/, gr_vector_void_star& /*output_items*/) {
    	return 0; // nothing to do
    }

  } // namespace ccsds
} // namespace gr
