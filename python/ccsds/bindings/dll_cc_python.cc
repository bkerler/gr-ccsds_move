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
/* BINDTOOL_HEADER_FILE(dll_cc.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(8a255a96d43306bcde7b70c13d8eeba5)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/dll_cc.h>
// pydoc.h is automatically generated in the build directory
#include <dll_cc_pydoc.h>

void bind_dll_cc(py::module& m)
{

    using dll_cc = ::gr::ccsds::dll_cc;


    py::class_<dll_cc, gr::block, gr::basic_block, std::shared_ptr<dll_cc>>(
        m, "dll_cc", D(dll_cc))

        .def(py::init(&dll_cc::make), py::arg("osf"), py::arg("gamma"), D(dll_cc, make))


        ;
}
