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
/* BINDTOOL_HEADER_FILE(randomiser_softbits.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(5699b1be65394fc3a7baf134291fdaba)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/randomiser_softbits.h>
// pydoc.h is automatically generated in the build directory
#include <randomiser_softbits_pydoc.h>

void bind_randomiser_softbits(py::module& m)
{

    using randomiser_softbits = ::gr::ccsds::randomiser_softbits;


    py::class_<randomiser_softbits,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<randomiser_softbits>>(
        m, "randomiser_softbits", D(randomiser_softbits))

        .def(py::init(&randomiser_softbits::make),
             py::arg("polynomial"),
             py::arg("seed"),
             D(randomiser_softbits, make))


        ;
}
