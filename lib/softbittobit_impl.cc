/* -*- c++ -*- */
/* 
 * Copyright 2014 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "softbittobit_impl.h"

namespace gr {
  namespace ccsds {

    softbittobit::sptr
    softbittobit::make()
    {
      return gnuradio::get_initial_sptr
        (new softbittobit_impl());
    }

    /*
     * The private constructor
     */
    softbittobit_impl::softbittobit_impl()
      : gr::sync_block("softbittobit",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(char)))
    {}

    /*
     * Our virtual destructor.
     */
    softbittobit_impl::~softbittobit_impl()
    {
    }

    int
    softbittobit_impl::work(int noutput_items,
			  gr_vector_const_void_star &input_items,
			  gr_vector_void_star &output_items)
    {
        const float *in = (const float *) input_items[0];
        char *out = (char *) output_items[0];

	for (int i=0; i<noutput_items; i++) {
	  *out++=(*(in++)>=0.0?1:0);
	}
        // Do <+signal processing+>

        // Tell runtime system how many output items we produced.
        return noutput_items;
    }

  } /* namespace ccsds */
} /* namespace gr */

