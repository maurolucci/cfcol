#ifndef _CPLEX_ENV_HPP_
#define _CPLEX_ENV_HPP_

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>

struct CplexEnv {
  IloEnv Xenv;          // CPLEX environment structure
  IloModel Xmodel;      // CPLEX model
  IloObjective Xobj;    // CPLEX objective function
  IloNumVarArray Xvars; // CPLEX variables
  IloRangeArray Xrestr; // CPLEX constraints
  CplexEnv()
      : Xenv(IloEnv()), Xmodel(Xenv), Xobj(Xenv), Xvars(Xenv), Xrestr(Xenv) {}
  ~CplexEnv() {
    Xrestr.end();
    Xvars.end();
    Xobj.end();
    Xmodel.end();
    Xenv.end();
  }
};

#endif // _CPLEX_ENV_HPP_