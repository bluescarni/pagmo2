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

#ifndef PAGMO_PROBLEM_TRANSLATE_HPP
#define PAGMO_PROBLEM_TRANSLATE_HPP

#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <type_traits>

#include "../exceptions.hpp"
#include "../io.hpp"
#include "../problem.hpp"
#include "../serialization.hpp"
#include "../type_traits.hpp"
#include "../types.hpp"

namespace pagmo
{

/// The translate meta-problem
/**
 * This meta-problem translates the whole search space of an input problem
 * by a fixed translation vector. pagmo::translate objects are user-defined problems that can be used in
 * the definition of a pagmo::problem.
 */
class translate : public problem
{
    // Enabler for the UDP ctor.
    template <typename T>
    using ctor_enabler
        = enable_if_t<std::is_constructible<problem, T &&>::value && !std::is_same<uncvref_t<T>, problem>::value, int>;

public:
    /// Default constructor
    /**
     * The default constructor will initialize a non-translated pagmo::null_problem.
     */
    translate() : problem(null_problem{}), m_translation({0.})
    {
    }

    /// Constructor from UDP and translation vector
    /**
     * **NOTE** This constructor is enabled only if \p T can be used to construct a pagmo::problem,
     * and \p T is not pagmo::problem.
     *
     * Wraps a user-defined problem so that its fitness , bounds, etc. will be shifted by a
     * translation vector.
     *
     * @param p a user-defined problem.
     * @param translation an <tt>std::vector</tt> containing the translation to apply.
     *
     * @throws std::invalid_argument if the length of \p translation is
     * not equal to the problem dimension \f$ n_x\f$.
     * @throws unspecified any exception thrown by the pagmo::problem constructor.
     */
    template <typename T, ctor_enabler<T> = 0>
    explicit translate(T &&p, const vector_double &translation)
        : problem(std::forward<T>(p)), m_translation(translation)
    {
        if (translation.size() != static_cast<const problem *>(this)->get_nx()) {
            pagmo_throw(std::invalid_argument, "Length of shift vector is: " + std::to_string(translation.size())
                                                   + " while the problem dimension is: "
                                                   + std::to_string(static_cast<const problem *>(this)->get_nx()));
        }
    }

    /// Fitness
    /**
     * The fitness computation is forwarded to the inner UDP, after the translation of \p x.
     *
     * @param x the decision vector.
     *
     * @return the fitness of \p x.
     *
     * @throws unspecified any exception thrown by memory errors in standard containers,
     * or by problem::fitness().
     */
    vector_double fitness(const vector_double &x) const
    {
        vector_double x_deshifted = translate_back(x);
        return static_cast<const problem *>(this)->fitness(x_deshifted);
    }

    /// Box-bounds
    /**
     * The box-bounds returned by this method are the translated box-bounds of the inner UDP.
     *
     * @return the lower and upper bounds for each of the decision vector components.
     *
     * @throws unspecified any exception thrown by memory errors in standard containers,
     * or by problem::get_bounds().
     */
    std::pair<vector_double, vector_double> get_bounds() const
    {
        auto b_sh = static_cast<const problem *>(this)->get_bounds();
        // NOTE: this should be safe as the translation vector has been checked against the
        // bounds size upon construction (via get_nx()).
        return {apply_translation(b_sh.first), apply_translation(b_sh.second)};
    }

    /// Gradients
    /**
     * The gradients computation is forwarded to the inner UDP, after the translation of \p x.
     *
     * @param x the decision vector.
     *
     * @return the gradient of the fitness function.
     *
     * @throws unspecified any exception thrown by memory errors in standard containers,
     * or by problem::gradient().
     */
    vector_double gradient(const vector_double &x) const
    {
        vector_double x_deshifted = translate_back(x);
        return static_cast<const problem *>(this)->gradient(x_deshifted);
    }

    /// Hessians
    /**
     * The hessians computation is forwarded to the inner UDP, after the translation of \p x.
     *
     * @param x the decision vector.
     *
     * @return the hessians of the fitness function.
     *
     * @throws unspecified any exception thrown by memory errors in standard containers,
     * or by problem::hessians().
     */
    std::vector<vector_double> hessians(const vector_double &x) const
    {
        vector_double x_deshifted = translate_back(x);
        return static_cast<const problem *>(this)->hessians(x_deshifted);
    }

    /// Problem name
    /**
     * This method will add <tt>[translated]</tt> to the name provided by the UDP.
     *
     * @return a string containing the problem name.
     *
     * @throws unspecified any exception thrown by problem::get_name() or memory errors in standard classes.
     */
    std::string get_name() const
    {
        return static_cast<const problem *>(this)->get_name() + " [translated]";
    }

    /// Extra info
    /**
     * This method will append a description of the translation vector to the extra info provided
     * by the UDP.
     *
     * @return a string containing extra info on the problem.
     *
     * @throws unspecified any exception thrown by problem::get_extra_info(), the public interface of
     * \p std::ostringstream or memory errors in standard classes.
     */
    std::string get_extra_info() const
    {
        std::ostringstream oss;
        stream(oss, m_translation);
        return static_cast<const problem *>(this)->get_extra_info() + "\n\tTranslation Vector: " + oss.str();
    }

    /// Get the translation vector
    /**
     * @return a reference to the translation vector.
     */
    const vector_double &get_translation() const
    {
        return m_translation;
    }

    /// Object serialization
    /**
     * This method will save/load \p this into/from the archive \p ar.
     *
     * @param ar target archive.
     *
     * @throws unspecified any exception thrown by the serialization of the UDP and of primitive types.
     */
    template <typename Archive>
    void serialize(Archive &ar)
    {
        ar(cereal::base_class<problem>(this), m_translation);
    }

private:
    // Delete all that we do not want to inherit from problem
    // A - Common to all meta
    vector_double::size_type get_nx() const = delete;
    vector_double::size_type get_nf() const = delete;
    vector_double::size_type get_nc() const = delete;
    unsigned long long get_fevals() const = delete;
    unsigned long long get_gevals() const = delete;
    unsigned long long get_hevals() const = delete;
    vector_double::size_type get_gs_dim() const = delete;
    std::vector<vector_double::size_type> get_hs_dim() const = delete;
    bool is_stochastic() const = delete;
    bool feasibility_f(const vector_double &) const = delete;
    bool feasibility_x(const vector_double &) const = delete;
    vector_double get_c_tol() const = delete;
    void set_c_tol(const vector_double &) = delete;

// The CI using gcc 4.8 fails to compile this delete, excluding it in that case does not harm
// it would just result in a "weird" behaviour in case the user would try to stream this object
#if __GNUC__ > 4
    // NOTE: We delete the streaming operator overload called with translate, otherwise the inner prob would stream
    // NOTE: If a streaming operator is wanted for this class remove the line below and implement it
    friend std::ostream &operator<<(std::ostream &, const translate &) = delete;
#endif
    template <typename Archive>
    void save(Archive &) const = delete;
    template <typename Archive>
    void load(Archive &) = delete;

    vector_double translate_back(const vector_double &x) const
    {
        // NOTE: here we use assert instead of throwing because the general idea is that we don't
        // protect UDPs from misusesm, and we have checks in problem. In Python, UDP methods that could cause
        // troubles are not exposed.
        assert(x.size() == m_translation.size());
        vector_double x_sh(x.size());
        std::transform(x.begin(), x.end(), m_translation.begin(), x_sh.begin(), std::minus<double>());
        return x_sh;
    }

    vector_double apply_translation(const vector_double &x) const
    {
        assert(x.size() == m_translation.size());
        vector_double x_sh(x.size());
        std::transform(x.begin(), x.end(), m_translation.begin(), x_sh.begin(), std::plus<double>());
        return x_sh;
    }
    /// translation vector
    vector_double m_translation;
};
}

PAGMO_REGISTER_PROBLEM(pagmo::translate)

#endif
