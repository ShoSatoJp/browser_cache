#include "stdafx.hpp"
#include "chrome_cache.hpp"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(chrome_cache, m) {
	py::class_<ChromeCache>(m, "ChromeCache")
		.def(py::init<string, string, bool>(), py::arg("cache_dir"), py::arg("temp_dir"), py::arg("update_index") = true)
		.def("keys", &ChromeCache::keys)
		.def("find_save", &ChromeCache::find_save, py::arg("key"), py::arg("path"))
		.def("find", &ChromeCache::find, py::arg("key"));

	py::class_<ChromeCacheEntry>(m, "ChromeCacheEntry")
		.def_readonly("key", &ChromeCacheEntry::key)
		.def("get_header", &ChromeCacheEntry::get_header)
		.def("save", &ChromeCacheEntry::save);

	py::class_<HttpHeader>(m, "HttpHeader")
		.def_readonly("status_code", &HttpHeader::status_code)
		.def_readonly("status_source", &HttpHeader::status_source)
		.def_readonly("protocol", &HttpHeader::protocol)
		.def_readonly("headers", &HttpHeader::headers);
}