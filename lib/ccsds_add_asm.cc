#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ccsds_add_asm.h>
#include "ccsds_hexstring_to_binary.h"
#include <gr_io_signature.h>



ccsds_add_asm_sptr ccsds_make_add_asm(std::string ASM, const unsigned int frame_len) {
    return ccsds_add_asm_sptr (new ccsds_add_asm(ASM, frame_len) );
}

ccsds_add_asm::ccsds_add_asm(std::string ASM, const unsigned int frame_len)
  : gr_block ("ccsds_add_asm",
	gr_make_io_signature (0, 0, sizeof (unsigned char)),
	gr_make_io_signature (0, 0, sizeof (unsigned char))), d_ASM_LEN(std::floor(ASM.length()/2)), d_FRAME_LEN(frame_len)
{
	// Transfer ASM from Hex to bytes
	d_ASM = new unsigned char[d_ASM_LEN];
	ccsds_hexstring_to_binary(&ASM, d_ASM);

	// register output port
	message_port_register_out(pmt::mp("out"));

	// register input type
	message_port_register_in(pmt::mp("in"));
	set_msg_handler(pmt::mp("in"), boost::bind(&ccsds_add_asm::process_frame, this, _1));
}

ccsds_add_asm::~ccsds_add_asm () {
	delete[] d_ASM;
}

void ccsds_add_asm::process_frame(pmt::pmt_t msg_in) {
	if(!pmt::pmt_is_blob(msg_in)) {
		fprintf(stderr,"ERROR ADD ASM: expecting message of type blob, skipping.\n");
		return;
	}

	// Message is BLOB
	const unsigned char *data_in = (const unsigned char *) pmt::pmt_blob_data(msg_in);

	unsigned char *data_out = new unsigned char[d_ASM_LEN+d_FRAME_LEN];

	memcpy(&data_out[0]        , d_ASM  , d_ASM_LEN  *sizeof(unsigned char));
	memcpy(&data_out[d_ASM_LEN], data_in, d_FRAME_LEN*sizeof(unsigned char));

	pmt::pmt_t msg_out = pmt::pmt_make_blob(data_out, d_ASM_LEN+d_FRAME_LEN);

	message_port_pub( pmt::mp("out"), msg_out );

	delete[] data_out;
}
