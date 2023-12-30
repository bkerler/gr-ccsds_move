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
/* BINDTOOL_HEADER_FILE(ldpc.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(b24b38edc62812ee4982f26e3af38ea1)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/ldpc.h>
// pydoc.h is automatically generated in the build directory
#include <ldpc_pydoc.h>

void bind_ldpc(py::module& m)
{


    py::enum_<::gr::ccsds::punct_t>(m, "punct_t")
        .value("LDPC_PUNCT_NONE", ::gr::ccsds::punct_t::LDPC_PUNCT_NONE)     // 0
        .value("LDPC_PUNCT_FRONT", ::gr::ccsds::punct_t::LDPC_PUNCT_FRONT)   // 1
        .value("LDPC_PUNCT_BACK", ::gr::ccsds::punct_t::LDPC_PUNCT_BACK)     // 2
        .value("LDPC_PUNCT_CUSTOM", ::gr::ccsds::punct_t::LDPC_PUNCT_CUSTOM) // 3
        .export_values();

    py::implicitly_convertible<int, ::gr::ccsds::punct_t>();
    py::enum_<::gr::ccsds::sys_t>(m, "sys_t")
        .value("LDPC_SYS_NONE", ::gr::ccsds::sys_t::LDPC_SYS_NONE)   // 0
        .value("LDPC_SYS_FRONT", ::gr::ccsds::sys_t::LDPC_SYS_FRONT) // 1
        .value("LDPC_SYS_BACK", ::gr::ccsds::sys_t::LDPC_SYS_BACK)   // 2
        .export_values();

    py::implicitly_convertible<int, ::gr::ccsds::sys_t>();
}
