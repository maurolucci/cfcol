#ifndef __PRICING_HPP__
#define __PRICING_HPP__

#include "graph.hpp"
#include "stats.hpp"
extern "C" {
#include "color.h"
#include "color_private.h"
#include "mwis.h"
}

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>

#include <random>

#define THRESHOLD 1.1            // Threshold for early stop
#define PRICING_EPSILON 0.00001  // 10e-5

// This is the class implementing the generic callback interface.
class ThresholdCallback : public IloCplex::Callback::Function {
 private:
  DPCPInst& dpcp;
  StableEnv& stab;

  // Variables
  IloNumVarArray& y;
  IloNumVarArray& w;

 public:
  // Constructor with data.
  ThresholdCallback(DPCPInst& dpcpRef, StableEnv& stab, IloNumVarArray& y,
                    IloNumVarArray& w)
      : dpcp(dpcpRef), stab(stab), y(y), w(w) {};

  inline void check_threshold(const IloCplex::Callback::Context& context);

  // This is the function that we have to implement and that CPLEX will call
  // during the solution process at the places that we asked for.
  virtual void invoke(const IloCplex::Callback::Context& context) ILO_OVERRIDE;

  /// Destructor
  ~ThresholdCallback() {};
};

int double2COLORNWT(COLORNWT nweights[], COLORNWT* scalef,
                    const std::vector<double>& dbl_weights);

class PricingEnv {
 private:
  DPCPInst& dpcp;  // Reference to the DPCP instance at the current node
  StableEnv stab;  // StableEnv to store the solution of the pricing problem
  double exactTimeLimit;  // Time limit for the exact pricing method

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

  // MWIS variables
  MWISenv* mwis_env;
  COLORNWT* mwis_pi;
  int ecount;
  int* elist;

  void exact_init();
  void mwis_init();

  // std::pair<bool, size_t> get_weights(std::vector<double> &weights,
  //                                    IloNumArray &dualsP, IloNumArray
  //                                    &dualsQ);

 public:
  PricingEnv(DPCPInst& dpcpRef, double exactTimeLimit);
  ~PricingEnv();

  std::pair<StableEnv, PRICING_STATE> heur_solve(IloNumArray& dualsP,
                                                 IloNumArray& dualsQ,
                                                 double alpha = 0.1);

  std::list<std::pair<StableEnv, PRICING_STATE>> mwis_P_Q_solve(
      IloNumArray& dualsP, IloNumArray& dualsQ);
  std::pair<StableEnv, PRICING_STATE> mwis_P_solve(IloNumArray& dualsP,
                                                   IloNumArray& dualsQ);

  std::pair<StableEnv, PRICING_STATE> exact_solve(IloNumArray& dualsP,
                                                  IloNumArray& dualsQ);
};

#endif  // __PRICING_HPP__