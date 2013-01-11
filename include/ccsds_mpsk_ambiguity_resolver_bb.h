#ifndef INCLUDED_CCSDS_MPSK_AMBIGUITY_RESOLVER_BB_H
#define INCLUDED_CCSDS_MPSK_AMBIGUITY_RESOLVER_BB_H

#include <ccsds_api.h>
#include <gr_block.h>
#include <string>

/*! \brief Verbosity level: Do not output anything */
#define CCSDS_AR_OUTPUT_NONE 0

/*! \brief Verbosity level: Do output state information */
#define CCSDS_AR_OUTPUT_STATE 1

/*! \brief Verbosity level: Do output debug information */
#define CCSDS_AR_OUTPUT_DEBUG 2

/*! \brief Level of verbosity of this block.
 *
 *  \sa #CCSDS_AR_OUTPUT_NONE
 *  \sa #CCSDS_AR_OUTPUT_STATE
 *  \sa #CCSDS_AR_OUTPUT_DEBUG
 */
#define CCSDS_AR_VERBOSITY_LEVEL CCSDS_AR_OUTPUT_STATE

class ccsds_mpsk_ambiguity_resolver_bb;

/*
 * We use boost::shared_ptr's instead of raw pointers for all access
 * to gr_blocks (and many other data structures).  The shared_ptr gets
 * us transparent reference counting, which greatly simplifies storage
 * management issues.  This is especially helpful in our hybrid
 * C++ / Python system.
 *
 * See http://www.boost.org/libs/smart_ptr/smart_ptr.htm
 *
 * As a convention, the _sptr suffix indicates a boost::shared_ptr
 */
typedef boost::shared_ptr<ccsds_mpsk_ambiguity_resolver_bb> ccsds_mpsk_ambiguity_resolver_bb_sptr;

/*!
 *  \brief Return a shared_ptr to a new instance of ccsds_mpsk_ambiguity_resolver_bb
 *
 *  Create an instance of ccsds_mpsk_ambiguity_resolver_bb and return it's shared_ptr.
 *
 *  \param M Modulation order. Determins how many parallel streams to watch.
 *  \param ASM String containing the attached sync marker to look for in
 *	hexadecimal representation. Must be of even length.
 *  \param threshold Number of ASM losses to enter full search again. If
 *	set to one, a full search is performed every time an ASM is lost.
 *  \param frame_length Length of a frame (without ASM) in bytes.
 *  \return Shared pointer to the created AR block
 */
CCSDS_API ccsds_mpsk_ambiguity_resolver_bb_sptr ccsds_make_mpsk_ambiguity_resolver_bb(const unsigned int M, std::string ASM, unsigned int threshold, const unsigned int frame_length);

/*!
 * \brief M-PSK Ambiguity resolution. Take M input streams and look for ASM on
 *	each one. Output the one that shows the ASM, i.e. that has the right
 *	phase ambiguity. ASM is preserved in the output stream.
 * \ingroup synchronization
 *
 * \todo Document final search and lock algorithms.
 */
class CCSDS_API ccsds_mpsk_ambiguity_resolver_bb : public gr_block
{
private:
	friend CCSDS_API ccsds_mpsk_ambiguity_resolver_bb_sptr ccsds_make_mpsk_ambiguity_resolver_bb(const unsigned int M, std::string ASM, unsigned int threshold, const unsigned int frame_length);

	/*!
	 * \brief Private constructor of the AR
	 *  \param M Modulation order. Determins how many parallel streams to
	 *	watch.
	 *  \param ASM String containing the attached sync marker to look for in
	 *	hexadecimal representation. Must be of even length.
	 *  \param threshold Number of ASM losses to enter full search again. If
	 *	set to one, a full search is performed every time an ASM is lost.
	 *  \param frame_length Length of a frame (without ASM) in bytes.
	 *
	 *  Constructs a AR block that searches for the ASM in the \c M input
	 *  streams and outputs the one that contains the ASM. If no ASM is
	 *  found, no output is given. This block preserves the ASM, as it is
	 *  still needed for frame synchronization after decoding.
	 */
	ccsds_mpsk_ambiguity_resolver_bb(const unsigned int M, std::string ASM, unsigned int threshold, const unsigned int frame_length);
	
	/*! \brief Enumerator for the two different states. */
	enum d_STATE {
		STATE_SEARCH=0,
		STATE_LOCK};

	/*! \brief State variable */
	d_STATE d_state;

	/*! \brief Modulation order */
	const unsigned int d_M;

	/*! \brief Mapping to translate from the gray coded bits to the binary
	 *	coded constellation index.
	 */
	const unsigned char *d_map_bits_to_index;

	/*! \brief Mapping to translate from the binary coded constellation
	 *	index to the gray coded bits.
	 */
	const unsigned char *d_map_index_to_bits;

	/*! \brief number of bits in an unpacked input bytecalculated as ld(d_M). */
	const unsigned int d_ldM;

	/*! \brief Storage for attached snc marker
	 *
	 *  \sa d_ASM_LEN
	 */
	unsigned char *d_ASM;

	/*! \brief Length of ASM in bytes 
	 *
	 *  \sa d_ASM
	 */
	const unsigned int d_ASM_LEN;

	/*! \brief Number of ASM losses to enter full search again. If
	 *	set to one, a full search is performed every time an ASM is lost.
	 */
	const unsigned int d_THRESHOLD;

	/*! \brief Maximum number of bit errors that may occur between a
	 *	sequence and the ASM to still consider the sequence as an ASM.
	 *	If set to zero, sequence must match the ASM exactly.
	 */
	const unsigned int d_BER_THRESHOLD;

	/*! \brief Length of a frame (without ASM) in bytes. */
	const unsigned int d_FRAME_LEN;

	
	/*! \brief Counter variable on how many ASMs have been observed */
	unsigned int d_count;

	/*! \brief Index of the input stream that the AR is locked in. If in
	 *	search state, this variable may contain anything.
	 *
	 *  \sa d_state
	 */
	unsigned int d_locked_on_stream;
	

	/*! \brief Byte index of the next expected ASM if locked. If in search
	 *	state this variable may contain anything.
	 *
	 *  \sa d_state
	 *  \sa d_offset_bits
	 */
	unsigned int d_offset_bytes;

	/*! \brief Bit index of the next expected ASM in the byte indicated by
	 *	d_offset_byte if in lock state. If in search state this
	 *	variable may contain anything.
	 *
	 *  \sa d_state
	 *  \sa d_offset_bytes
	 */
	unsigned int d_offset_bits;

	/*! \brief Number of bits the first sample that belong to a old byte
	 *	which's samples have already been consumed.
	 */
	unsigned int d_samp_skip_bits;

	/*! \brief Extract an array of packed bytes from the samples of unpacked
	 *	bits and change it's change ambiguity.
	 *
	 *  \param in_unpacked Array of samples (unpacked bits) to process.
	 *  \param bytes_offset Index of the first requested byte if the
	 *	complete packed stream was available in a single array.
	 *  \param bytes_req Number of bytes that should be produced.
	 *  \param ambiguity The number of symbols by which the received
	 *	constellation was rotated before going into the detector.
	 *
	 *  \sa d_offset_bits
	 *
	 *  Assuming the input samples (unpacked bits) would be converted into
	 *  one huge array of packed bytes. This function gives access to the
	 *  \c bytes_req elements of this array starting at index \c bytes_offset,
	 *  given the complete original input sequence \c in_unpacked.
	 *  Additionally the byte and sample stream can be shifted by a few bits
	 *  is the last run of \ref general_work did not end on a boundary of a
	 *  sample but only on a byte boundary. This shift is stored in
	 *  \c d_offset_bits.
	 *
	 *  The input bits are converted from gray code to binary code to see
	 *  which constellation point was received, increase it's index by
	 *  \c ambiguity_offset and convert the binary index back to gray coded
	 *  bits.
	 */
	unsigned char * get_packed_bytes(const unsigned char *in_unpacked, const unsigned int bytes_offset, const unsigned int bytes_req, unsigned int ambiguity);

	/*! \brief Checks if stream matches an ASM at given bit offset.
	 *
	 *  \param stream Array of bytes to check ASM against. If \c offset_bits is zero
	 *	\c stream must have at least \c d_ASM_LEN elements, otherwise one element
	 *	more.
	 *  \param offset_bits Number of bits to ignore in the first byte of the stream.
	 *  \return \a true if stream is matching the ASM, \a false if not.
	 *
	 *  \sa d_BER_THRESHOLD
	 *
	 *  The stream is considered to match if \c d_BER_THRESHOLD or less bits
	 *  differ.
	 */
	bool check_for_asm(const unsigned char* stream, unsigned int offset_bits);

	/*! \brief Searches for ASM in the given stream.
	 *
	 *  \param stream Array of bytes in which to search for the ASM.
	 *  \param stream_len Length of the stream. The last \c d_ASM_LEN bytes of this
	 *	stream are not searched (as the ASM would only fit in partially).
	 *  \param *offset_bytes Pointer to an unsigned int where the offset bytes between
	 *	start of stream and the found ASM should be stored, if a match is found.
	 *  \param *offset_bits Pointer to an unsigned int where the offset bits between
	 *	start of stream and the found ASM should be stored, if a match is found.
	 *  \return \a true if ASM is found, \a false otherwise.
	 */
	bool search_asm(const unsigned char* stream, const unsigned int stream_len, unsigned int *offset_bytes, unsigned int *offset_bits);

	/*! \brief Copy bitstream from a byte array with a bit offset into an
	 *	aligned byte array.
	 *
	 *  \param stream_in Array of bytes to copy. First bit to copy is
	 *	located in the first element of this array. If \c offset_bit is
	 *	zero this array must contain at least \c len elements, if
	 *	\c offset_bit is greater thatn zero, this array must contain
	 *	at least \c len+1 elements.
	 *  \param stream_out Array in which the resulting bytes should be
	 *	copied. Memory for at least \c len elements must be allocated.
	 *  \param len Number of bytes to copy.
	 *  \param offset_bits Number of bits to skip in the first byte, starting
	 *	with the MSB. If offset_bit is zero this function copies the
	 *	first \c len bytes from \c stream_in to \c stream_out by using
	 *	\c memcpy. Otherwise the copy operation is done manually using
	 *	bitwise operations.
	 *
	 *  
	 *  The first \c offset_bits starting with the MSB from the first
	 *  element of \c stream_in are skipped, afterwards all bits from
	 *  \c stream_in are copied to \c stream_out until \c len bytes have
	 *  been copied to \c stream_out.
	 */
	void copy_stream(const unsigned char * stream_in, unsigned char * stream_out, const unsigned int len, const unsigned int offset_bits);
public:
	/*! \brief Public deconstructor of the AR */	
	~ccsds_mpsk_ambiguity_resolver_bb ();  // public destructor

	void forecast(int noutput_items,gr_vector_int &ninput_items_required);

	int  general_work (int                     noutput_items,
                                gr_vector_int               &ninput_items,
                                gr_vector_const_void_star   &input_items,
                                gr_vector_void_star         &output_items);};

#endif /* INCLUDED_CCSDS_MPSK_AMBIGUITY_RESOLVER_BB_H */
