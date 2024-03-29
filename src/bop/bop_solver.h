// Copyright 2010-2014 Google
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OR_TOOLS_BOP_BOP_SOLVER_H_
#define OR_TOOLS_BOP_BOP_SOLVER_H_

// Solver for Boolean Optimization Problems built on top of the SAT solver.
// To optimize a problem the solver uses several optimization strategies like
// Local Search (LS), Large Neighborhood Search (LNS), and Linear
// Programming (LP). See bop_parameters.proto to tune the strategies.
//
// Note that the BopSolver usage is limited to:
//   - Boolean variables,
//   - Linear constraints and linear optimization objective,
//   - Integral weights for both constraints and objective,
//   - Minimization.
// To deal with maximization, integral variables and floating weights, one can
// use the bop::IntegralSolver.
//
// Usage example:
//   const LinearBooleanProblem problem = BuildProblem();
//   BopSolver bop_solver(problem);
//   BopParameters bop_parameters;
//   bop_parameters.set_max_deterministic_time(10);
//   bop_solver.SetParameters(bop_parameters);
//   const BopSolveStatus solve_status = bop_solver.Solve();
//   if (solve_status == BopSolveStatus::OPTIMAL_SOLUTION_FOUND) { ... }

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/integral_types.h"
#include "base/logging.h"
#include "base/macros.h"
#include "base/int_type_indexed_vector.h"
#include "base/int_type.h"
#include "bop/bop_base.h"
#include "bop/bop_parameters.pb.h"
#include "bop/bop_solution.h"
#include "bop/bop_types.h"
#include "glop/lp_solver.h"
#include "sat/boolean_problem.pb.h"
#include "sat/sat_solver.h"
#include "util/stats.h"
#include "util/time_limit.h"

namespace operations_research {
namespace bop {
// Solver of Boolean Optimization Problems based on Local Search.
class BopSolver {
 public:
  explicit BopSolver(const LinearBooleanProblem& problem);
  virtual ~BopSolver();

  // Parameters management.
  void SetParameters(const BopParameters& parameters) {
    parameters_ = parameters;
  }

  // Returns the status of the optimization.
  BopSolveStatus Solve();
  BopSolveStatus Solve(const BopSolution& first_solution);

  // Runs the solver with an external time limit.
  // NOTE(user): These methods do not use the external boolean registered as
  // a limit within this class, but the caller can register the same boolean
  // directly into the time limit object to achieve the same effect. Eventually,
  // we should remove the explicit 'external_boolean_as_limit_' from this class,
  // and use only the boolean registered in the time limit object.
  BopSolveStatus SolveWithTimeLimit(TimeLimit* time_limit);
  BopSolveStatus SolveWithTimeLimit(const BopSolution& first_solution,
                                    TimeLimit* time_limit);

  const BopSolution& best_solution() const { return problem_state_.solution(); }
  bool GetSolutionValue(VariableIndex var_id) const {
    return problem_state_.solution().Value(var_id);
  }

  // Returns the scaled best bound.
  // In case of minimization (resp. maximization), the best bound is defined as
  // the lower bound (resp. upper bound).
  double GetScaledBestBound() const;
  double GetScaledGap() const;

  // Sets an external limit to stop the search when the Boolean value becomes
  // true. Note that the Solve() call may still linger for a while depending on
  // the conditions.
  void RegisterExternalBooleanAsLimit(const bool* external_boolean_as_limit);

 private:
  void UpdateParameters();
  BopSolveStatus InternalMonothreadSolver(TimeLimit* time_limit);
  BopSolveStatus InternalMultithreadSolver(TimeLimit* time_limit);

  const LinearBooleanProblem& problem_;
  ProblemState problem_state_;
  BopParameters parameters_;
  const bool* external_boolean_as_limit_;

  mutable StatsGroup stats_;
};
}  // namespace bop
}  // namespace operations_research
#endif  // OR_TOOLS_BOP_BOP_SOLVER_H_
