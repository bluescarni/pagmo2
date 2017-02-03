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

#define BOOST_TEST_MODULE hypervolume_utilities_test
#include <boost/test/included/unit_test.hpp>
#include <exception>
#include <tuple>

#include <pagmo/detail/hypervolume_all.hpp>
#include <pagmo/population.hpp>
#include <pagmo/problem.hpp>
#include <pagmo/types.hpp>
#include <pagmo/io.hpp>
#include <pagmo/problems/zdt.hpp>
#include <pagmo/problems/rosenbrock.hpp>

using namespace pagmo;


/**
* Assertion method that tests correct computation of contributions for the whole contribution method
* and the single exclusive method.
*/
void assertContribs(const std::vector<vector_double> &points, std::vector<double> &ref, std::vector<double> &answers) {
	hypervolume hv = hypervolume(points, true);
	BOOST_CHECK((hv.contributions(ref) == answers));
	for (unsigned int i = 0; i < answers.size(); i++) {
		BOOST_CHECK((hv.exclusive(i, ref) == answers[i]));
	}
}


BOOST_AUTO_TEST_CASE(hypervolume_compute_test)
{
	hypervolume hv;

	// by vector
	std::vector<vector_double> x1{ { 1,2 },{ 3,4 } };
	hv = hypervolume(x1, true);
	BOOST_CHECK(hv.get_points() == x1);

	// by list constructor
	hv = hypervolume{ { 6,4 },{ 3,5 } };
	std::vector<vector_double> x2{ { 6,4 },{ 3,5 } };
	BOOST_CHECK((hv.get_points() == x2));

		// by population
	population pop1{ problem{ zdt{ 1,5 } }, 2 };
	hv = hypervolume(pop1, true);

	// errors
	population pop2{ problem{ rosenbrock(10) }, 2 };
	BOOST_CHECK_THROW(hypervolume(pop2, true), std::invalid_argument);

	// 2d computation of hypervolume indicator
	hv = hypervolume{ {1, 2},{2, 1} };
	BOOST_CHECK((hv.compute({ 3,3 }) == 3));

	// point on the border of refpoint(2D)
	BOOST_CHECK((hv.compute({ 2,2 }) == 0));

	// 3d computation of hypervolume indicator
	hv = hypervolume{ {1, 1, 1},{2, 2, 2,} };
	BOOST_CHECK((hv.compute({ 3, 3, 3 }) == 8));

	// points on the border of refpoint(3D)
	hv = hypervolume{ {1, 2, 1},{2, 1, 1} };
	BOOST_CHECK((hv.compute({ 2, 2, 2 }) == 0));

	// 4d computation of hypervolume indicator
	hv = hypervolume{ {1, 1, 1, 1},{2, 2, 2, 2} };
	BOOST_CHECK((hv.compute({ 3, 3, 3, 3 }) == 16));

	// points on the border of refpoint(4D)
	hv = hypervolume{ {1, 1, 1, 3},{2, 2, 2, 3} };
	BOOST_CHECK((hv.compute({ 3, 3, 3, 3 }) == 0));

	// 4d duplicate point
	hv = hypervolume{ { 1, 1, 1, 1 },{ 1, 1, 1, 1 } };
	BOOST_CHECK((hv.compute({ 2, 2, 2, 2 }) == 1));

	// 4d duplicate and dominated
	hv = hypervolume{ { 1.0, 1.0, 1.0, 1.0 },{ 1.0, 1.0, 1.0, 1.0 },{0.0,0.0,0.0,0.0} };
	BOOST_CHECK((hv.compute({ 2.0, 2.0, 2.0, 2.0 }) == 16.0));

	// tests for invalid reference points
	hv = hypervolume{ {1, 3},{2, 2},{3, 1} };
	// equal to some other point
	BOOST_CHECK_THROW(hv.compute({ 3,1 }), std::invalid_argument);
	// refpoint dominating some points
	BOOST_CHECK_THROW(hv.compute({ 1.5,1.5 }), std::invalid_argument);
	// refpoint dominating all points
	BOOST_CHECK_THROW(hv.compute({ 0, 0 }), std::invalid_argument);

	// invalid dimensions of points.
	BOOST_CHECK_THROW(hv = hypervolume({ {2.3, 3.4, 5.6}, {1.0, 2.0, 3.0, 4.0} }), std::invalid_argument);

	// Calling specific algorithms
	std::shared_ptr<hv_algorithm> hv_algo_2d = hv2d().clone();
	std::shared_ptr<hv_algorithm> hv_algo_3d = hv3d().clone();
	std::shared_ptr<hv_algorithm> hv_algo_nd = hvwfg().clone();

	hv = hypervolume{ {2.3, 4.5},{3.4, 3.4},{6.0, 1.2} };
	BOOST_CHECK((hv.compute({ 7.0, 7.0 }) == 17.91));
	BOOST_CHECK((hv.compute({ 7.0, 7.0 }, hv_algo_2d) == 17.91));
	BOOST_CHECK_THROW(hv.compute({ 7.0, 7.0 }, hv_algo_3d), std::invalid_argument);
	BOOST_CHECK((hv.compute({ 7.0, 7.0 }, hv_algo_nd) == 17.91));

	hv = hypervolume{ { 2.3, 4.5, 3.2 },{ 3.4, 3.4, 3.4 },{ 6.0, 1.2, 3.6 } };
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0 }) == 66.386));
	BOOST_CHECK_THROW(hv.compute({ 7.0, 7.0, 7.0 }, hv_algo_2d), std::invalid_argument);
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0 }, hv_algo_3d) == 66.386));
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0 }, hv_algo_nd) == 66.386));

	hv = hypervolume{ { 2.3, 4.5, 3.2 },{ 3.4, 3.4, 3.4 },{ 6.0, 1.2, 3.6 } };
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0 }) == 66.386));
	BOOST_CHECK_THROW(hv.compute({ 7.0, 7.0, 7.0 }, hv_algo_2d), std::invalid_argument);
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0 }, hv_algo_3d) == 66.386));
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0 }, hv_algo_nd) == 66.386));

	hv = hypervolume{ {2.3, 4.5, 3.2, 1.9, 6.0},{3.4, 3.4, 3.4, 2.1, 5.8},{6.0, 1.2, 3.6, 3.0, 6.0} };
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0, 7.0, 7.0 }) == 373.21228));
	BOOST_CHECK_THROW(hv.compute({ 7.0, 7.0, 7.0, 7.0, 7.0 }, hv_algo_2d), std::invalid_argument);
	BOOST_CHECK_THROW(hv.compute({ 7.0, 7.0, 7.0, 7.0, 7.0 }, hv_algo_3d), std::invalid_argument);
	BOOST_CHECK((hv.compute({ 7.0, 7.0, 7.0, 7.0, 7.0 }, hv_algo_nd) == 373.21228));

	BOOST_CHECK_THROW(hvwfg(0), std::invalid_argument);
	BOOST_CHECK_THROW(hvwfg(1), std::invalid_argument);

}

BOOST_AUTO_TEST_CASE(hypervolume_contributions_test) {
	// Tests for contributions and exclusive hypervolumes
	std::vector<vector_double> points;
	std::vector<double> ref;
	std::vector<double> answers;

	/*  This test contains a front with 3 non dominated points,
		and many dominated points. Most of the dominated points
		lie on edges of the front, which makes their exclusive contribution
		equal to 0.*/
	points = { { 1, 6.5 },{ 1, 6 },{ 1, 5 },{ 2, 5 },{ 3, 5 },{ 3, 3 },{ 4, 6.5 },
	{ 4.5, 4 },{ 5, 3 },{ 5, 1.5 },{ 7, 1.5 },{ 7, 3.5 }, };
	ref = { 7.0, 6.5, };
	answers = { 0.0, 0.0, 1.0, 0.0, 0.0, 3.5, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, };
	assertContribs(points, ref, answers);

	// same test with duplicates and points on the edge of the ref-point
	points = { { 1, 6.5 },{ 1, 6 },{ 1, 5 },{ 2, 5 },{ 3, 5 },{ 3, 3 },{ 4, 6.5 },
	{ 4.5, 4 },{ 5, 3 },{ 5, 1.5 },{ 7, 1.5 },{ 7, 3.5 },{ 7, 0.5 },{ 7, 1.0 },{ 7, 4.5 },
	{ 0.0, 6.5 },{ 5.5, 6.5 },{ 7, 0.5 },{ 5.5, 6.5 },{ 5, 5 },{ 5, 5 },{ 5, 5 } };
	ref = { 7.0, 6.5, };
	answers = { 0.0, 0.0, 1.0, 0.0, 0.0, 3.5, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0,
				0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
	assertContribs(points, ref, answers);

	// Gradually adding duplicate points to the set, making sure the contribution change accordingly.
	points = { {1, 1} };
	ref = { 2, 2 };
	answers = { 1.0 };
	assertContribs(points, ref, answers);

	points.push_back({ 1, 1 });
	answers = { 0.0, 0.0 };
	assertContribs(points, ref, answers);

	points.push_back({ 1, 1 });
	answers = { 0.0, 0.0, 0.0 };
	assertContribs(points, ref, answers);

	points.push_back({ 0.5, 0.5 });
	answers = { 0.0, 0.0, 0.0, 1.25 };
	assertContribs(points, ref, answers);

	points.push_back({ 0.5, 0.5 });
	answers = { 0.0, 0.0, 0.0, 0.0, 0.0 };
	assertContribs(points, ref, answers);

	// Next test contains a tricky front in 3D with some weakly dominated points on the "edges" of the bounding box.
	// Non - tricky base problem
	points = { { -6, -1, -6 },{ -1, -3, -5 },{ -3, -4, -4 },
	{ -4, -2, -3 },{ -5, -5, -2 },{ -2, -6, -1 } };
	ref = { 0, 0, 0 };
	answers = { 18, 2, 12, 1, 18, 2 };
	assertContribs(points, ref, answers);

	// Add some points that contribute nothing and do not alter other
	points = { { -6, -1, -6 },{ -1, -3, -5 },{ -3, -4, -4 },
	{ -4, -2, -3 },{ -5, -5, -2 },{ -2, -6, -1 }, {-3, -1, -3},{ -1, -1, -5 },{ -1, -2, -4 },
	{ -1, -3, -4 },{ -7, -7, 0 },{ 0, -5, -5 },{ -7, 0, -7 } };
	answers = { 18, 2, 12, 1, 18, 2, 0, 0, 0, 0, 0, 0, 0 };
	assertContribs(points, ref, answers);


	//	Gradually adding points, some of which are dominated or duplicates.
	//	Tests whether contributions and repeated exclusive method produce the same results.
	points = { {3, 3, 3} };
	ref = { 5, 5, 5 };
	answers = { 8.0 };
	assertContribs(points, ref, answers);

	// Decrease the contribution of first point.Second point is dominated.
	points.push_back({ 4, 4, 4 });
	answers = { 7, 0, };
	assertContribs(points, ref, answers);

	// Add duplicate point
	points.push_back({ 3, 3, 3 });
	answers = { 0, 0, 0 };
	assertContribs(points, ref, answers);

	points.push_back({ 3, 3, 2 });
	answers = { 0, 0, 0, 4 };
	assertContribs(points, ref, answers);

	points.push_back({ 3,3,1 });
	answers = { 0, 0, 0, 0, 4 };
	assertContribs(points, ref, answers);

	//	Combine extreme points together. Mixing small and large contributions in a single front
	points = { {-1, -1, -1}, { -1, -1, -1 }, { -1, -1, -1 } };
	ref = { 0, 0, 0 };
	answers = { 0, 0, 0 };
	assertContribs(points, ref, answers);


	// Adding a point far away
	points.push_back({-1000,-1000,-1000});
	answers = { 0, 0, 0, 999999999 };
	assertContribs(points, ref, answers);

	// Adding an even further point
	points.push_back({ -10000,-10000,-10000 });
	answers = { 0, 0, 0, 0, 999000000000 };
	assertContribs(points, ref, answers);

	//	Gradually adding points in 4d.	Tests whether contributions and repeated exclusive methods produce the same results.
	points = { {1, 1, 1, 1} };
	ref = { 5, 5, 5, 5 };
	answers = { 256 };
	assertContribs(points, ref, answers);

	points.push_back({ 4,4,4,4 });
	answers = { 255, 0 };
	assertContribs(points, ref, answers);

	points.push_back({ 3,3,3,3 });
	answers = { 240, 0, 0};
	assertContribs(points, ref, answers);

	points.push_back({ 1,1,1,1 });
	answers = { 0, 0, 0, 0 };
	assertContribs(points, ref, answers);

	//	Gradually adding points in 5d.	Tests whether contributions and repeated exclusive methods produce the same results.
	points = { { 1, 1, 1, 1, 1 } };
	ref = { 5, 5, 5, 5, 5 };
	answers = { 1024 };
	assertContribs(points, ref, answers);

	points.push_back({ 4,4,4,4,4});
	answers = { 1023, 0 };
	assertContribs(points, ref, answers);

	points.push_back({ 3,3,3,3,3 });
	answers = { 992, 0, 0 };
	assertContribs(points, ref, answers);

	points.push_back({ 1,1,1,1,1 });
	answers = { 0, 0, 0, 0 };
	assertContribs(points, ref, answers);

}

BOOST_AUTO_TEST_CASE(hypervolume_least_contribution_test) {
	hypervolume hv;
	std::vector<double> ref = { 4, 4 };

	hv = hypervolume({ { 3, 1 },{ 2, 2 },{ 1, 3 } });  // All points are least contributors
	BOOST_CHECK((hv.least_contributor(ref) >= 0 && hv.least_contributor(ref) <= 2));
	BOOST_CHECK((hv.greatest_contributor(ref) >= 0 && hv.greatest_contributor(ref) <= 2));


	hv = hypervolume({ { 2.5, 1 },{ 2, 2 },{ 1, 3 } });
	BOOST_CHECK((hv.least_contributor(ref) == 1));

	hv = hypervolume({ { 3.5, 1 },{ 2, 2 },{ 1, 3 } });
	BOOST_CHECK((hv.least_contributor(ref) == 0));

	hv = hypervolume({ { 3, 1 },{ 2.5, 2.5 },{ 1, 3 } });
	BOOST_CHECK((hv.least_contributor(ref) == 1));

	hv = hypervolume({ { 3, 1 },{ 2, 2 },{ 1, 3.5 } });
	BOOST_CHECK((hv.least_contributor(ref) == 2));

	hv = hypervolume({ { 3, 1 },{ 2, 2 },{ 1, 3.5 } });
	BOOST_CHECK_THROW(hv.least_contributor({ 4,4,4 }), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(hypervolume_exclusive_test) {
	hypervolume hv;
	std::vector<double> ref = { 4, 4 };

	// all are equal(take first->idx = 0)
	hv = hypervolume{ {3, 1},{2, 2},{1, 3} };
	BOOST_CHECK((hv.exclusive(0, ref) == 1));
	BOOST_CHECK((hv.exclusive(1, ref) == 1));
	BOOST_CHECK((hv.exclusive(2, ref) == 1));

	// index out of bounds
	BOOST_CHECK_THROW(hv.exclusive(200, ref), std::invalid_argument);

	// picking the wrong algorithm
	std::shared_ptr<hv_algorithm> hv_algo_3d = hv3d().clone();
	BOOST_CHECK_THROW(hv.exclusive(0, ref, hv_algo_3d), std::invalid_argument);

}

BOOST_AUTO_TEST_CASE(hypervolume_refpoint_test) {
	hypervolume hv = hypervolume{ {3, 1},{2, 2},{1, 3} };

	BOOST_CHECK((hv.refpoint() == vector_double{3, 3}));
	BOOST_CHECK((hv.refpoint(5) == vector_double{ 8, 8 }));
	BOOST_CHECK((hv.refpoint(0) == vector_double{ 3, 3 }));
	BOOST_CHECK((hv.refpoint(-0) == vector_double{ 3, 3 }));
	BOOST_CHECK((hv.refpoint(-1) == vector_double{ 2, 2 }));

}
