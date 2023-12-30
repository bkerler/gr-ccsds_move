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
/* BINDTOOL_HEADER_FILE(softbit_msg_source_f.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(259069adfa464e2e01fba4d91d7af1c2)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/softbit_msg_source_f.h>
// pydoc.h is automatically generated in the build directory
#include <softbit_msg_source_f_pydoc.h>

void bind_softbit_msg_source_f(py::module& m)
{

    using softbit_msg_source_f = ::gr::ccsds::softbit_msg_source_f;


    py::class_<softbit_msg_source_f,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<softbit_msg_source_f>>(
        m, "softbit_msg_source_f", D(softbit_msg_source_f))

        .def(py::init(&softbit_msg_source_f::make),
             py::arg("frame_len"),
             D(softbit_msg_source_f, make))


        ;
}
