#include "stdafx.h"
#include <string>
#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace std;

string add(int a, string b) {
	return to_string(a) + b;
}

PYBIND11_MODULE(test, m) {
	m.doc() = "hogehoge";
	m.def("add", &add);
}
