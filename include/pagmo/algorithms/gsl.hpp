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

#ifndef PAGMO_ALGORITHMS_GSL_HPP
#define PAGMO_ALGORITHMS_GSL_HPP

#include <cstddef>
#include <new>
#include <stdexcept>

#if defined(NDEBUG)
#define GSL_RANGE_CHECK_OFF
#endif

#define GSL_C99_INLINE
#define HAVE_INLINE
#include <gsl/gsl_errno.h>
#include <gsl/gsl_vector.h>
#undef HAVE_INLINE
#undef GSL_C99_INLINE

#if defined(NDEBUG)
#undef GSL_RANGE_CHECK_OFF
#endif

#include <pagmo/exceptions.hpp>

namespace pagmo
{

namespace detail
{

struct gsl_vec {
    gsl_vec(std::size_t n)
    {
        if (!n) {
            pagmo_throw(std::invalid_argument, "cannot initialise a GSL vector with zero size");
        }
        m_vec = ::gsl_vector_alloc(n);
        if (!m_vec) {
            pagmo_throw(std::bad_alloc, );
        }
    }
    gsl_vec(const gsl_vec &other)
    {
        m_vec = ::gsl_vector_alloc(other.size());
        if (!m_vec) {
            pagmo_throw(std::bad_alloc, );
        }
        auto retval = ::gsl_vector_memcpy(m_vec, other.m_vec);
    }
    ~gsl_vec()
    {
        ::gsl_vector_free(m_vec);
    }
    double &operator[](std::size_t i)
    {
        return *::gsl_vector_ptr(m_vec, i);
    }
    const double &operator[](std::size_t i) const
    {
        return *::gsl_vector_const_ptr(m_vec, i);
    }
    std::size_t size() const
    {
        return m_vec->size;
    }
    ::gsl_vector *m_vec;
};
}
}

#endif
