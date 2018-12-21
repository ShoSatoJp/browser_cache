#include "stdafx.h"
#include "firefox_cache.hpp"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(firefox_cache, m) {
	py::class_<FirefoxCache>(m, "FirefoxCache")
		.def(py::init<string,  bool>(), py::arg("cache2_dir"), py::arg("use_index") = false)
		.def("keys", &FirefoxCache::keys)
		.def("find_save", &FirefoxCache::find_save, py::arg("key"), py::arg("path"))
		.def("find", &FirefoxCache::find, py::arg("key"));

	py::class_<FirefoxCacheEntry>(m, "FirefoxCacheEntry")
		.def_readonly("key", &FirefoxCacheEntry::key)
		.def("get_header", &FirefoxCacheEntry::get_header)
		.def("save", &FirefoxCacheEntry::save);

	py::class_<HttpHeader>(m, "HttpHeader")
		.def_readonly("status_code", &HttpHeader::status_code)
		.def_readonly("status_source", &HttpHeader::status_source)
		.def_readonly("protocol", &HttpHeader::protocol)
		.def_readonly("headers", &HttpHeader::headers);

}