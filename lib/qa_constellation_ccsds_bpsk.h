/* -*- c++ -*- */
/*
 * Copyright 2008 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#ifndef _QA_CCSDS_CONSTELLATION_CCSDS_BPSK_H_
#define _QA_CCSDS_CONSTELLATION_CCSDS_BPSK_H_

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>


namespace gr {
  namespace ccsds {
    namespace qa {
      namespace constellation {

    class bpsk : public CppUnit::TestCase {
      CPPUNIT_TEST_SUITE(bpsk);
      CPPUNIT_TEST(test_dimensions);
      CPPUNIT_TEST(test_decisions);
      CPPUNIT_TEST(test_chain);
      CPPUNIT_TEST_SUITE_END();

    private:
      void test_dimensions();
      void test_decisions();
      void test_chain();
    };

      }  // namespace constellation
    }  // namespace qa
  }  // namespace ccsds
}  // namespace gr
#endif  // _QA_CCSDS_CONSTELLATION_CCSDS_BPSK_H_
