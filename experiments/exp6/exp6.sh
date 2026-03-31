#!/bin/bash

# exp5: Evaluation of pricing

declare ALPHA_PRI="0.2"
declare GREEDY_MAX_COLS="1000"
declare VARIANT="3"
declare ALPHA_HEUR="0.1"
declare REPETITIONS_HEUR="500"

declare INPUT="../../instances/dpcp/random"
declare INSTANCES="$INPUT/instances.txt"
declare BIN="../../dpcp"
declare OUT="out/"

echo "Running experiment #6"

# First, create output directories
for p in "0" "1" "2" "3" "4" "5"
do
    mkdir -p "$OUT/p$p"
done

# Second, run experiments
while IFS= read -r LINE
do
    echo "Processing instance: $LINE"
    # Pricing #0: exact
    time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/p0/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-greedy-off --pricing-pq-mwsp-off --pricing-p-mwsp-off --pricing-exact-time 900
    # Pricing #1: greedy + exact
    time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/p1/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-pq-mwsp-off --pricing-p-mwsp-off --pricing-greedy-alpha $ALPHA_PRI --pricing-greedy-max-cols $GREEDY_MAX_COLS --pricing-exact-time 900
    # Pricing #2: greedy + MWSSP I + exact
    time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/p2/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-p-mwsp-off --pricing-greedy-alpha $ALPHA_PRI --pricing-greedy-max-cols $GREEDY_MAX_COLS --pricing-exact-time 900
    # Pricing #3: greedy + MWSSP II + exact
    time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/p3/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-pq-mwsp-off --pricing-greedy-alpha $ALPHA_PRI --pricing-greedy-max-cols $GREEDY_MAX_COLS --pricing-exact-time 900
    # Pricing #4: greedy + MWSSP I + MWSSP II + exact
    time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/p4/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-order 1 --pricing-greedy-alpha $ALPHA_PRI --pricing-greedy-max-cols $GREEDY_MAX_COLS --pricing-exact-time 900
    # Pricing #5: greedy + MWSSP II + MWSSP I + exact
    time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/p5/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-order 2 --pricing-greedy-alpha $ALPHA_PRI --pricing-greedy-max-cols $GREEDY_MAX_COLS --pricing-exact-time 900
done < "$INSTANCES"
