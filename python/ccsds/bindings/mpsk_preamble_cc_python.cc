/*
 * Copyright 2023 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(mpsk_preamble_cc.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(4b844478e55f94aa65dfd7f23eb1a912)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/mpsk_preamble_cc.h>
// pydoc.h is automatically generated in the build directory
#include <mpsk_preamble_cc_pydoc.h>

void bind_mpsk_preamble_cc(py::module& m)
{

    using mpsk_preamble_cc = ::gr::ccsds::mpsk_preamble_cc;


    py::class_<mpsk_preamble_cc,
               gr::block,
               gr::basic_block,
               std::shared_ptr<mpsk_preamble_cc>>(
        m, "mpsk_preamble_cc", D(mpsk_preamble_cc))

        .def(py::init(&mpsk_preamble_cc::make),
             py::arg("num_symbols"),
             D(mpsk_preamble_cc, make))


        ;
}
