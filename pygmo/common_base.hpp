/* Copyright 2017 PaGMO development team

This file is part of the PaGMO library.

The PaGMO library is free software; you can redistribute it and/or modify
it under the terms of either:

  * the GNU Lesser General Public License as published by the Free
    Software Foundation; either version 3 of the License, or (at your
    option) any later version.

or

  * the GNU General Public License as published by the Free Software
    Foundation; either version 3 of the License, or (at your option) any
    later version.

or both in parallel, as here.

The PaGMO library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received copies of the GNU General Public License and the
GNU Lesser General Public License along with the PaGMO library.  If not,
see https://www.gnu.org/licenses/. */

#ifndef PYGMO_COMMON_BASE_HPP
#define PYGMO_COMMON_BASE_HPP

#include "python_includes.hpp"

#include <boost/python/extract.hpp>
#include <boost/python/object.hpp>
#include <string>

#include "common_utils.hpp"

namespace pygmo
{

namespace bp = boost::python;

// A common base class with methods useful in the implementation of
// the pythonic problem and algorithm.
struct common_base {
    static void check_mandatory_method(const bp::object &o, const char *s, const char *target)
    {
        if (callable_attribute(o, s).is_none()) {
            pygmo_throw(PyExc_NotImplementedError, ("the mandatory '" + std::string(s)
                                                    + "()' method has not been detected in the user-defined Python "
                                                    + std::string(target) + " '" + str(o) + "' of type '" + str(type(o))
                                                    + "': the method is either not present or not callable")
                                                       .c_str());
        }
    }
    // A simple wrapper for getters. It will try to:
    // - get the attribute "name" from the object o,
    // - call it without arguments,
    // - extract an instance from the ret value and return it.
    // If the attribute is not there or it is not callable, the value "def_value" will be returned.
    template <typename RetType>
    static RetType getter_wrapper(const bp::object &o, const char *name, const RetType &def_value)
    {
        auto a = callable_attribute(o, name);
        if (a.is_none()) {
            return def_value;
        }
        return bp::extract<RetType>(a());
    }
    // Check if the user is trying to construct a pagmo object from a type, rather than from an object.
    // This is an easy error to commit, and it is sneaky because the callable_attribute() machinery will detect
    // the methods of the *class* (rather than instance methods), and it will thus not error out.
    static void check_not_type(const bp::object &o, const char *target)
    {
        if (isinstance(o, builtin().attr("type"))) {
            pygmo_throw(PyExc_TypeError, ("it seems like you are trying to instantiate a pygmo " + std::string(target)
                                          + " using a type rather than an object instance: please construct an object "
                                            "and use that instead of the type in the "
                                          + std::string(target) + " constructor")
                                             .c_str());
        }
    }
};
}

#endif
