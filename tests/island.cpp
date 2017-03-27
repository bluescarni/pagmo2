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

#define BOOST_TEST_MODULE island_test
#include <boost/test/included/unit_test.hpp>

#include <atomic>
#include <boost/lexical_cast.hpp>
#include <initializer_list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <pagmo/algorithms/de.hpp>
#include <pagmo/island.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problems/rosenbrock.hpp>
#include <pagmo/serialization.hpp>
#include <pagmo/threading.hpp>
#include <pagmo/types.hpp>

using namespace pagmo;

struct udi_01 {
    void run_evolve(island &) const
    {
    }
    std::string get_name() const
    {
        return "udi_01";
    }
    std::string get_extra_info() const
    {
        return "extra bits";
    }
};

struct udi_02 {
    void run_evolve(island &);
};

struct udi_03 {
    void run_evolve(const island &) const;
};

BOOST_AUTO_TEST_CASE(island_type_traits)
{
    BOOST_CHECK(is_udi<thread_island>::value);
    BOOST_CHECK(!is_udi<int>::value);
    BOOST_CHECK(!is_udi<const thread_island>::value);
    BOOST_CHECK(!is_udi<const thread_island &>::value);
    BOOST_CHECK(!is_udi<thread_island &>::value);
    BOOST_CHECK(!is_udi<void>::value);
    BOOST_CHECK(is_udi<udi_01>::value);
    BOOST_CHECK(!is_udi<udi_02>::value);
    BOOST_CHECK(is_udi<udi_03>::value);
}

BOOST_AUTO_TEST_CASE(island_constructors)
{
    // Various constructors.
    island isl;
    BOOST_CHECK(isl.get_algorithm().is<null_algorithm>());
    BOOST_CHECK(isl.get_population().get_problem().is<null_problem>());
    BOOST_CHECK(isl.get_population().size() == 0u);
    auto isl2 = isl;
    BOOST_CHECK(isl2.get_algorithm().is<null_algorithm>());
    BOOST_CHECK(isl2.get_population().get_problem().is<null_problem>());
    BOOST_CHECK(isl2.get_population().size() == 0u);
    island isl3{de{}, population{rosenbrock{}, 25}};
    BOOST_CHECK(isl3.get_algorithm().is<de>());
    BOOST_CHECK(isl3.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl3.get_population().size() == 25u);
    auto isl4 = isl3;
    BOOST_CHECK(isl4.get_algorithm().is<de>());
    BOOST_CHECK(isl4.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl4.get_population().size() == 25u);
    island isl5{thread_island{}, de{}, population{rosenbrock{}, 26}};
    BOOST_CHECK(isl5.get_algorithm().is<de>());
    BOOST_CHECK(isl5.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl5.get_population().size() == 26u);
    island isl6{de{}, rosenbrock{}, 27};
    BOOST_CHECK(isl6.get_algorithm().is<de>());
    BOOST_CHECK(isl6.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl6.get_population().size() == 27u);
    island isl7{de{}, rosenbrock{}, 27, 123};
    BOOST_CHECK(isl7.get_algorithm().is<de>());
    BOOST_CHECK(isl7.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl7.get_population().size() == 27u);
    BOOST_CHECK(isl7.get_population().get_seed() == 123u);
    island isl8{thread_island{}, de{}, rosenbrock{}, 28};
    BOOST_CHECK(isl8.get_algorithm().is<de>());
    BOOST_CHECK(isl8.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl8.get_population().size() == 28u);
    island isl9{thread_island{}, de{}, rosenbrock{}, 29, 124};
    BOOST_CHECK(isl9.get_algorithm().is<de>());
    BOOST_CHECK(isl9.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl9.get_population().size() == 29u);
    BOOST_CHECK(isl9.get_population().get_seed() == 124u);
    island isl10{std::move(isl9)};
    BOOST_CHECK(isl10.get_algorithm().is<de>());
    BOOST_CHECK(isl10.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl10.get_population().size() == 29u);
    BOOST_CHECK(isl10.get_population().get_seed() == 124u);
    // Revive isl9;
    isl9 = island{thread_island{}, de{}, rosenbrock{}, 29, 124};
    BOOST_CHECK(isl9.get_algorithm().is<de>());
    BOOST_CHECK(isl9.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl9.get_population().size() == 29u);
    BOOST_CHECK(isl9.get_population().get_seed() == 124u);
    // Copy assignment.
    isl9 = isl8;
    BOOST_CHECK(isl9.get_algorithm().is<de>());
    BOOST_CHECK(isl9.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl9.get_population().size() == 28u);
    // Self assignment.
    BOOST_CHECK((std::is_same<island &, decltype(isl9 = isl9)>::value));
    isl9 = isl9;
    BOOST_CHECK(isl9.get_algorithm().is<de>());
    BOOST_CHECK(isl9.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl9.get_population().size() == 28u);
#if !defined(__clang__)
    BOOST_CHECK((std::is_same<island &, decltype(isl9 = std::move(isl9))>::value));
    isl9 = std::move(isl9);
    BOOST_CHECK(isl9.get_algorithm().is<de>());
    BOOST_CHECK(isl9.get_population().get_problem().is<rosenbrock>());
    BOOST_CHECK(isl9.get_population().size() == 28u);
#endif
    // Some type-traits.
    BOOST_CHECK((std::is_constructible<island, de, population>::value));
    BOOST_CHECK((std::is_constructible<island, de, problem &&, unsigned>::value));
    BOOST_CHECK((std::is_constructible<island, de, problem &&, unsigned>::value));
    BOOST_CHECK((std::is_constructible<island, const thread_island &, de, problem &&, unsigned>::value));
    BOOST_CHECK((!std::is_constructible<island, double, std::string &&>::value));
}

BOOST_AUTO_TEST_CASE(island_concurrent_access)
{
    island isl{de{}, rosenbrock{}, 27, 123};
    auto thread_func = [&isl]() {
        for (auto i = 0; i < 100; ++i) {
            auto pop = isl.get_population();
            isl.set_population(pop);
            auto algo = isl.get_algorithm();
            isl.set_algorithm(algo);
        }
    };
    std::thread t1(thread_func), t2(thread_func), t3(thread_func), t4(thread_func);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
}

BOOST_AUTO_TEST_CASE(island_evolve)
{
    island isl{de{}, population{rosenbrock{}, 25}};
    isl.evolve(0);
    isl.get();
    isl.evolve();
    isl.get();
    isl.evolve(20);
    isl.get();
    // Copy/move operations with a few tasks queued.
    auto enqueue_n = [](island &is, int n) {
        for (auto i = 0; i < n; ++i) {
            is.evolve(20);
        }
    };
    enqueue_n(isl, 10);
    auto isl2(isl);
    auto isl3(std::move(isl));
    isl = island{de{}, population{rosenbrock{}, 25}};
    enqueue_n(isl, 10);
    isl2 = isl;
    isl3 = std::move(isl);
    isl2.wait();
    isl3.get();
}

static std::atomic_bool flag = ATOMIC_VAR_INIT(false);

struct prob_01 {
    vector_double fitness(const vector_double &) const
    {
        while (!flag.load()) {
        }
        return {.5};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0.}, {1.}};
    }
};

BOOST_AUTO_TEST_CASE(island_get_wait_busy)
{
    flag.store(true);
    island isl{de{}, population{prob_01{}, 25}};
    BOOST_CHECK(!isl.busy());
    flag.store(false);
    isl.evolve();
    BOOST_CHECK(isl.busy());
    flag.store(true);
    isl.wait();
    flag.store(false);
    isl = island{de{}, population{rosenbrock{}, 3}};
    isl.evolve(10);
    isl.evolve(10);
    isl.evolve(10);
    isl.evolve(10);
    BOOST_CHECK_THROW(isl.get(), std::invalid_argument);
    isl.get();
    isl.wait();
}

struct prob_02 {
    vector_double fitness(const vector_double &) const
    {
        return {.5};
    }
    std::pair<vector_double, vector_double> get_bounds() const
    {
        return {{0.}, {1.}};
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
};

struct algo_01 {
    population evolve(const population &pop) const
    {
        return pop;
    }
    thread_safety get_thread_safety() const
    {
        return thread_safety::none;
    }
};

BOOST_AUTO_TEST_CASE(island_tread_safety)
{
    island isl{de{}, population{rosenbrock{}, 25}};
    auto ts = isl.get_thread_safety();
    BOOST_CHECK(ts[0] == thread_safety::basic);
    BOOST_CHECK(ts[1] == thread_safety::basic);
    isl.evolve();
    isl.get();
    isl = island{de{}, population{prob_02{}, 25}};
    ts = isl.get_thread_safety();
    BOOST_CHECK(ts[0] == thread_safety::basic);
    BOOST_CHECK(ts[1] == thread_safety::none);
    isl.evolve();
    BOOST_CHECK_THROW(isl.get(), std::invalid_argument);
    isl = island{algo_01{}, population{rosenbrock{}, 25}};
    ts = isl.get_thread_safety();
    BOOST_CHECK(ts[0] == thread_safety::none);
    BOOST_CHECK(ts[1] == thread_safety::basic);
    isl.evolve();
    BOOST_CHECK_THROW(isl.get(), std::invalid_argument);
    isl = island{algo_01{}, population{prob_02{}, 25}};
    ts = isl.get_thread_safety();
    BOOST_CHECK(ts[0] == thread_safety::none);
    BOOST_CHECK(ts[1] == thread_safety::none);
    isl.evolve();
    BOOST_CHECK_THROW(isl.get(), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(island_name_info_stream)
{
    std::ostringstream oss;
    island isl{de{}, population{rosenbrock{}, 25}};
    oss << isl;
    BOOST_CHECK(!oss.str().empty());
    BOOST_CHECK(isl.get_name() == "Thread island");
    BOOST_CHECK(isl.get_extra_info() == "");
    oss.str("");
    isl = island{udi_01{}, de{}, population{rosenbrock{}, 25}};
    oss << isl;
    BOOST_CHECK(!oss.str().empty());
    BOOST_CHECK(isl.get_name() == "udi_01");
    BOOST_CHECK(isl.get_extra_info() == "extra bits");
}

BOOST_AUTO_TEST_CASE(island_serialization)
{
    island isl{de{}, population{rosenbrock{}, 25}};
    isl.evolve();
    isl.get();
    std::stringstream ss;
    auto before = boost::lexical_cast<std::string>(isl);
    // Now serialize, deserialize and compare the result.
    {
        cereal::JSONOutputArchive oarchive(ss);
        oarchive(isl);
    }
    isl = island{de{}, population{rosenbrock{}, 25}};
    {
        cereal::JSONInputArchive iarchive(ss);
        iarchive(isl);
    }
    auto after = boost::lexical_cast<std::string>(isl);
    BOOST_CHECK_EQUAL(before, after);
}
