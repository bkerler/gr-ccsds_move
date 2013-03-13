#ifndef INCLUDED_CCSDS_MPSK_AMBIGUITY_RESOLVER_F_H
#define INCLUDED_CCSDS_MPSK_AMBIGUITY_RESOLVER_F_H

#include <ccsds_api.h>
#include <gr_block.h>
#include <string>

/*! \brief Verbosity level: Do not output anything */
#define CCSDS_AR_SOFT_OUTPUT_NONE 0

/*! \brief Verbosity level: Do output state change information */
#define CCSDS_AR_SOFT_OUTPUT_CHANGE 1

/*! \brief Verbosity level: Do output state information */
#define CCSDS_AR_SOFT_OUTPUT_STATE 2

/*! \brief Verbosity level: Do output debug information */
#define CCSDS_AR_SOFT_OUTPUT_DEBUG 3

/*! \brief Level of verbosity of this block.
 *
 *  \sa #CCSDS_AR_SOFT_OUTPUT_NONE
 *  \sa #CCSDS_AR_SOFT_OUTPUT_CHANGE
 *  \sa #CCSDS_AR_SOFT_OUTPUT_STATE
 *  \sa #CCSDS_AR_SOFT_OUTPUT_DEBUG
 */
#define CCSDS_AR_SOFT_VERBOSITY_LEVEL CCSDS_AR_SOFT_OUTPUT_STATE

class ccsds_mpsk_ambiguity_resolver_f;

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
typedef boost::shared_ptr<ccsds_mpsk_ambiguity_resolver_f> ccsds_mpsk_ambiguity_resolver_f_sptr;

/*!
 *  \brief Return a shared_ptr to a new instance of ccsds_mpsk_ambiguity_resolver_f
 *
 *  Create an instance of ccsds_mpsk_ambiguity_resolver_f and return it's shared_ptr.
 *
 *  \param M Modulation order. Determins how many parallel streams to watch.
 *  \param ASM String containing the attached sync marker to look for in
 *	hexadecimal representation. If asm_len is not a multiple of 8 (i.e. ASM
 *	is not a multiple of bytes), the ASM string should be tailed by zeros.
 *  \param asm_len Length of the ASM in bits
 *  \param threshold Number of ASM losses to enter full search again. If
 *	set to one, a full search is performed every time an ASM is lost.
 *  \param correlation_threshold Minimum normalized correlation between an incomming
 *	bit sequence and the ASM to consider them matching.
 *  \param frame_length Length of a frame (without ASM) in bytes.
 *  \param num_tail_syms Number of bits after the Frame that should be copied as
 *	well. Default is zero, so only the frame data is copied.
 *  \return Shared pointer to the created AR block
 */
CCSDS_API ccsds_mpsk_ambiguity_resolver_f_sptr ccsds_make_mpsk_ambiguity_resolver_f(const unsigned int M, std::string ASM, const unsigned int asm_len, const unsigned int threshold, const float correlation_threshold, const unsigned int frame_length, const unsigned int num_tail_syms=0);

/*!
 * \brief M-PSK soft bit ambiguity resolution and frame synchronization.
 *
 * \ingroup synchronization
 *
 * The input to this block is a stream of \c d_ldM grouped soft bits. The output
 * of this block is a asynchronous message containing a float vector with the
 * soft bits of a frame. The bytes passed to this block as ASM are cut off, but
 * any skipped bits will be in the output message.
 *
 * So if you pass the two ASM bytes 0x00 0xAA to this block followed by two
 * bytes frame data (FD1 and FD2), followed by another two bytes ASM and set
 * \c asm_skip_first_bits to zero the output will be a message containing FD1
 * and FD2. If \c asm_skip_first_bits is set to two, the output will be FD1, FD2
 * followed by the first two zero bits from the ASM.
 *
 * Using \c asm_skip_first_bits (setting it to nonzero values)
 *
 * The block will start in searchmode, looking for every possible bit offset
 * in all possible \c M ambiguities. Once an ASM is found it will enter
 * locked state where it will only check for the ASM at the expected position
 * and ambiguity. If the ASM is found a counter is increased up to
 * \c d_THRESHOLD. If an exprected ASM is not found the counter is decreased.
 * If the counter reaches zero the block goes into search mode again.
 */
class CCSDS_API ccsds_mpsk_ambiguity_resolver_f : public gr_block
{
private:
	friend CCSDS_API ccsds_mpsk_ambiguity_resolver_f_sptr ccsds_make_mpsk_ambiguity_resolver_f(const unsigned int M, std::string ASM, const unsigned int asm_len, const unsigned int threshold, const float correlation_threshold, const unsigned int frame_length, const unsigned int num_tail_syms);

	/*!
	 * \brief Private constructor of the AR
	 *  \param M Modulation order. Determins how many parallel streams to
	 *	watch.
	 *  \param ASM String containing the attached sync marker to look for in
	 *	hexadecimal representation. Must be of even length.
	 *  \param asm_len Length of the ASM in bits
	 *  \param threshold Number of ASM losses to enter full search again. If
	 *	set to one, a full search is performed every time an ASM is lost.
	 *  \param correlation_threshold Minimum normalized correlation between an incomming
	 *	bit sequence and the ASM to consider them matching.
	 *  \param num_tail_syms Number of bits after the Frame that should be copied as
	 *	well. Default is zero, so only the frame data is copied.
	 *  \param frame_length Length of a frame (without ASM) in bytes.
	 *
	 *  Constructs a AR block that searches for the ASM in the \c M input
	 *  streams and outputs the one that contains the ASM. If no ASM is
	 *  found, no output is given. This block preserves the ASM, as it is
	 *  still needed for frame synchronization after decoding.
	 */
	ccsds_mpsk_ambiguity_resolver_f(const unsigned int M, std::string ASM, const unsigned int asm_len, const unsigned int threshold, const float correlation_threshold, const unsigned int frame_length, const unsigned int num_tail_syms=0);
	
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

	/*! \brief Number of ASM losses to enter full search again. If
	 *	set to one, a full search is performed every time an ASM is lost.
	 */
	const unsigned int d_THRESHOLD;

	const float d_CORRELATION_THRESHOLD;

	/*! \brief Length of a frame (without ASM) in bytes. */
	//const unsigned int d_FRAME_LEN;

	/*! \brief Length of a frame (without ASM) in bits. */
	const unsigned int d_FRAME_LEN_BITS;

	float *d_ASM;

	/*! \brief Length of an ASM in bits. */
	const unsigned int d_ASM_LEN_BITS;

	/*! \brief Number of symbols/bits that should be copied as well after a frame */
	const unsigned int d_NUM_TAIL_SYMS;

	const unsigned int d_SEARCH_LEN_MIN;

	const unsigned int d_SEARCH_LEN_MAX;

	/*! \brief Counter variable on how many ASMs have been observed */
	unsigned int d_count;

	/*! \brief buffer to strore message in */
	pmt::pmt_t d_msg_buffer;

	/*! \brief Number of elements that are already in buffer */
	unsigned int d_msg_buffer_count;

	/*! \brief Flag whether the buffer is beeing filled right now, or not.
	 *	If not and we are in state LOCK, check for the existence of an
	 *	ASM at the current position.
	 */
	bool d_msg_buffer_fill;

	#if CCSDS_AR_SOFT_VERBOSITY_LEVEL >= CCSDS_AR_SOFT_OUTPUT_DEBUG
		/*! \brief File pointer for debugging. */
		FILE *dbg_file_in;

		/*! \brief File pointer for debugging. */
		FILE *dbg_file_in_hard;

		/*! \brief File pointer for debugging. */
		FILE *dbg_file_out;
	#endif
	#if CCSDS_AR_SOFT_VERBOSITY_LEVEL >= CCSDS_AR_SOFT_OUTPUT_CHANGE
		/*! \brief Counter for debugging. */
		unsigned long dbg_count;
	#endif

	/*! \brief Index of the input stream that the AR is locked in. If in
	 *	search state, this variable may contain anything.
	 *
	 *  \sa d_state
	 */
	unsigned int d_locked_on_stream;
	

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

	/*!
	 *  \brief Returns the number of unpacked symbols (as float) that relate
	 *	to the given number of packed bytes.
	 *
	 *  \param packed Number of packed bytes.
	 *  \return Number of unpacked bytes (symbols) that relate to \c packed.
	 */
	float from_packed_to_unpacked(unsigned int packed);

	/*!
	 *  \brief Ensures that \c d_offset_bits is confined within [0,d_ldM).
	 *	If not \c d_offset_bytes is increased accordingly.
	 *
	 *  \sa d_offset_bytes
	 *  \sa d_offset_bits
	 */
	void confine_offsets(void);

	/*!
	 *  \brief Change ambiguity of incomming symbol stream.
	 *
	 *  \param out Symbol array where to put the new ambiguity in. Memory
	 *	for at least \c num elements must be allocated.
	 *  \param in Symbol array which's abmiguity is to be changed. Must
	 *	contain at least \c num elements.
	 *  \param num_bits Number of elements (single bits) that should be converted.
	 *	Must be a multiple of \c d_ldM, otherwise the last bits will be
	 *	ignored.
	 *  \param ambiguity Ambiguity that should be calculated. Must be in the
	 *	range of [0,d_M), where 0 will just copy \c num elements from
	 *	\c in to \c out.
	 */
	void convert_ambiguity(float *out, const float *in, const unsigned int num_bits, unsigned int ambiguity);

	float check_for_ASM(const float *in, const unsigned int offset);
	float search_for_ASM(const float *in, const unsigned int search_len, unsigned int *offset);

	unsigned char convert_to_char(const float *in);

	unsigned int get_lower_mul(const unsigned int n);
	unsigned int get_upper_mul(const unsigned int n);

public:
	/*! \brief Public deconstructor of the AR */	
	~ccsds_mpsk_ambiguity_resolver_f ();  // public destructor

	void forecast(int noutput_items,gr_vector_int &ninput_items_required);

	bool stop(void);

	int  general_work (int  noutput_items,
			        gr_vector_int               &ninput_items,
			        gr_vector_const_void_star   &input_items,
			        gr_vector_void_star         &output_items);
};

#endif /* INCLUDED_CCSDS_MPSK_AMBIGUITY_RESOLVER_F_H */
