# cfcol
B&amp;P algorithm to solve conflict-free (partial) coloring

Compile with:

g++ main.cpp src/graph.cpp src/col.cpp src/lp.cpp include/bp.hpp exactcolors/color.o exactcolors/color_version.h exactcolors/util.o exactcolors/rounding_mode.o exactcolors/cliq_enum.o exactcolors/color_parms.o exactcolors/graph.o exactcolors/lpcplex.o exactcolors/bbsafe.o exactcolors/mwis_grdy.o exactcolors/heap.o exactcolors/mwis.o exactcolors/mwis_sewell/mwss_ext.o exactcolors/mwis_sewell/wstable.o exactcolors/color_backup.o exactcolors/greedy.o -I include/ -I hglib/include/ -I hglib/lib/filtered_vector/include/ -I /opt/ibm/ILOG/CPLEX_Studio2211/cplex/include/ -I /opt/ibm/ILOG/CPLEX_Studio2211/concert/include/ -I exactcolors/ -ldl -L /opt/ibm/ILOG/CPLEX_Studio2211/cplex/lib/x86-64_linux/static_pic -L /opt/ibm/ILOG/CPLEX_Studio2211/concert/lib/x86-64_linux/static_pic -lconcert -lilocplex -lcplex -std=c++20 -g

