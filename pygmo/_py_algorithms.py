# -*- coding: utf-8 -*-

# Copyright 2017-2018 PaGMO development team
#
# This file is part of the PaGMO library.
#
# The PaGMO library is free software; you can redistribute it and/or modify
# it under the terms of either:
#
#   * the GNU Lesser General Public License as published by the Free
#     Software Foundation; either version 3 of the License, or (at your
#     option) any later version.
#
# or
#
#   * the GNU General Public License as published by the Free Software
#     Foundation; either version 3 of the License, or (at your option) any
#     later version.
#
# or both in parallel, as here.
#
# The PaGMO library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received copies of the GNU General Public License and the
# GNU Lesser General Public License along with the PaGMO library.  If not,
# see https://www.gnu.org/licenses/.

# for python 2.0 compatibility
from __future__ import absolute_import as _ai


class scipy_de(object):
    def __init__(self, **kwargs):
        # Check the keyword arguments.
        if 'func' in kwargs:
            raise ValueError(
                'The "func" parameter must not be passed to the constructor of scipy_de, as the objective function will be provided by the pygmo problem instance.')
        if 'bounds' in kwargs:
            raise ValueError(
                'The "bounds" parameter must not be passed to the constructor of scipy_de, as the problem bounds will be provided by the pygmo problem instance.')
        if 'args' in kwargs:
            raise ValueError(
                'The "args" parameter must not be passed to the constructor of scipy_de, as the objective function will be provided by the pygmo problem instance.')
        if 'popsize' in kwargs:
            raise ValueError(
                'The "popsize" parameter must not be passed to the constructor of scipy_de, as the population will be provided by pygmo.')
        if 'init' in kwargs:
            raise ValueError(
                'The "init" parameter must not be passed to the constructor of scipy_de, as the population will be provided by pygmo.')

        # Store the keyword arguments.
        self._kwargs = kwargs

        # Init the result attribute to None.
        self._result = None

    def evolve(self, pop):
        from scipy.optimize import differential_evolution as de
        from numpy import array

        prob = pop.problem
        prob_name = prob.get_name()

        # Check the properties of the problem.
        if prob.get_nc() != 0:
            raise ValueError(
                'scipy_de supports only unconstrained optimisation, thus it cannot solve a constrained optimisation problem of type "{}"'.format(prob_name))

        if prob.get_nf() > 1:
            raise ValueError(
                'scipy_de supports only single-objective optimisation, thus it cannot solve a multi-objective optimisation problem of type "{}"'.format(prob_name))

        if prob.is_stochastic():
            raise ValueError(
                'scipy_de does not support stochastic problems, thus it cannot solve a stochastic optimisation problem of type "{}"'.format(prob_name))

        # Just return an empty input population.
        if len(pop) == 0:
            return pop

        # Define the objfun.
        def objfun(x):
            return prob.fitness(x)[0]

        # Get the bounds and transform them
        # in the format required by scipy.
        lb, ub = prob.get_bounds()
        bounds = [(_[0], _[1]) for _ in zip(lb, ub)]

        # Get the initial population. This is already
        # in the format required by scipy.
        init = pop.get_x()

        # Run the optimisation, store the result
        self._result = de(objfun, bounds, init=init, **self._kwargs)

        # Fetch the result of the optimisation,
        # replace the worst individual in the population.
        worst_idx = pop.worst_idx()
        pop.set_xf(worst_idx, self._result.x, [self._result.fun])

        # Return the modified population.
        return pop

    @property
    def result(self):
        return self._result
