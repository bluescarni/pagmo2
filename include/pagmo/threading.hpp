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

#ifndef PAGMO_THREADING_HPP
#define PAGMO_THREADING_HPP

namespace pagmo
{

/// Thread safety levels.
/**
 * This enum defines a set of values that can be used to specify
 * the thread safety of problems, algorithms, etc.
 */
enum class thread_safety {
    none, ///< No thread safety: concurrent operations on distinct instances are unsafe
    basic ///< Basic thread safety: concurrent operations on distinct instances are safe
};
}

#endif
