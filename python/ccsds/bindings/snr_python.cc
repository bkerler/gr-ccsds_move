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
/* BINDTOOL_HEADER_FILE(snr.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(7623b1f392ce194a6066acc9225a77f8)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/ccsds/snr.h>
// pydoc.h is automatically generated in the build directory
#include <snr_pydoc.h>

void bind_snr(py::module& m)
{

    using snr = ::gr::ccsds::snr;


    py::class_<snr, std::shared_ptr<snr>>(m, "snr", D(snr))

        .def(py::init<>(), D(snr, snr, 0))
        .def(py::init<float, float>(), py::arg("Es"), py::arg("N0"), D(snr, snr, 1))
        .def(py::init<gr::ccsds::snr const&>(), py::arg("arg0"), D(snr, snr, 2))


        .def("valid", &snr::valid, D(snr, valid))


        .def_static("pick_first_if_valid",
                    &snr::pick_first_if_valid,
                    py::arg("first"),
                    py::arg("second"),
                    D(snr, pick_first_if_valid))


        .def("Es", &snr::Es, D(snr, Es))


        .def("sqrtEs", &snr::sqrtEs, D(snr, sqrtEs))


        .def("N0", &snr::N0, D(snr, N0))


        .def("SNR_dB", &snr::SNR_dB, D(snr, SNR_dB))

        ;
}
