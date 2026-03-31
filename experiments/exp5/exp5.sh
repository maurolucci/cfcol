#!/bin/bash

# exp5: Evaluation of number of repetitions for the semigreedy pricing

declare ALPHA_PRI="0.2"
declare -a GREEDY_MAX_COLS=("1" "10" "100" "1000" "10000")
declare VARIANT="3"
declare ALPHA_HEUR="0.1"
declare REPETITIONS_HEUR="500"

declare INPUT="../../instances/dpcp/random"
declare INSTANCES="$INPUT/instances.txt"
declare BIN="../../dpcp"
declare OUT="out/"

echo "Running experiment #4"

# First, create output directories
for r in "${GREEDY_MAX_COLS[@]}"
do
    mkdir -p "$OUT/r$r"
done

# Second, run experiments
while IFS= read -r LINE
do
    echo "Processing instance: $LINE"
    for r in "${GREEDY_MAX_COLS[@]}"
    do
        # Pricing #1: heur + exact
        time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/r$r/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-pq-mwsp-off --pricing-p-mwsp-off --pricing-greedy-alpha $ALPHA_PRI --pricing-greedy-max-cols $r --pricing-exact-time 900
    done
done < "$INSTANCES"
