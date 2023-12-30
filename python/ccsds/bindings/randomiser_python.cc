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
/* BINDTOOL_HEADER_FILE(randomiser.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(f0f7ad4ea42c5e812963bc1672ee0318)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/randomiser.h>
// pydoc.h is automatically generated in the build directory
#include <randomiser_pydoc.h>

void bind_randomiser(py::module& m)
{

    using randomiser = ::gr::ccsds::randomiser;


    py::class_<randomiser,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<randomiser>>(m, "randomiser", D(randomiser))

        .def(py::init(&randomiser::make),
             py::arg("polynomial"),
             py::arg("seed"),
             D(randomiser, make))


        ;
}
