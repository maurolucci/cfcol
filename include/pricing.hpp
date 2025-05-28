#ifndef __PRICING_HPP__
#define __PRICING_HPP__

#include "graph.hpp"
#include "stats.hpp"

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>
#include <random>

#define THRESHOLD 1.1           // Threshold for early stop
#define PRICING_EPSILON 0.00001 // 10e-5

// This is the class implementing the generic callback interface.
class ThresholdCallback : public IloCplex::Callback::Function {

private:
  GraphEnv &in;
  StableEnv &stab;

  // Variables
  IloNumVarArray &y;
  IloNumVarArray &w;

public:
  // Constructor with data.
  ThresholdCallback(GraphEnv &in, StableEnv &stab, IloNumVarArray &y,
                    IloNumVarArray &w)
      : in(in), stab(stab), y(y), w(w){};

  inline void check_thresolhd(const IloCplex::Callback::Context &context);

  // This is the function that we have to implement and that CPLEX will call
  // during the solution process at the places that we asked for.
  virtual void invoke(const IloCplex::Callback::Context &context) ILO_OVERRIDE;

  /// Destructor
  ~ThresholdCallback(){};
};

class PricingEnv {

private:
  GraphEnv &in;
  StableEnv stab;

  // CPLEX variables
  IloEnv cxenv;
  IloModel cxmodel;
  IloObjective cxobj;
  IloNumVarArray y;
  IloNumVarArray w;
  IloConstraintArray cxcons;
  IloCplex cplex;

  // Callback variables
  ThresholdCallback cb;
  CPXLONG contextMask;

  std::list<std::tuple<double, size_t, TypeA, TypeB>> heurCandidates;

  void exact_init();

public:
  PricingEnv(GraphEnv &in);
  ~PricingEnv();

  std::pair<StableEnv, PRICING_STATE>
  heur_solve(IloNumArray &dualsA, IloNumArray &dualsB, Vertex v_first);

  std::pair<StableEnv, PRICING_STATE>
  exact_solve(IloNumArray &dualsA, IloNumArray &dualsB, double timelimit);
};

#endif // __PRICING_HPP__