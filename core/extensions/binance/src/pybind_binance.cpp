/**
 * This is source code under the Apache License 2.0.
 * Original Author: kx@godzilla.dev
 * Original date: March 3, 2025
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <kungfu/yijinjing/io.h>

#include "marketdata_binance.h"
#include "trader_binance.h"

namespace py = pybind11;

using namespace kungfu::yijinjing::data;
using namespace kungfu::wingchun::binance;

PYBIND11_MODULE(kfext_binance, m)
{
    py::class_<MarketDataBinance, kungfu::practice::apprentice, std::shared_ptr<MarketDataBinance>>(m, "MD")
            .def(py::init<bool, locator_ptr, const std::string &>())
            .def("run", &MarketDataBinance::run, py::call_guard<py::gil_scoped_release>());

    py::class_<TraderBinance, kungfu::practice::apprentice, std::shared_ptr<TraderBinance>>(m, "TD")
            .def(py::init<bool, locator_ptr, const std::string &, const std::string &>())
            .def("run", &TraderBinance::run, py::call_guard<py::gil_scoped_release>());
}
