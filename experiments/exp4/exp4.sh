#!/bin/bash

# exp4: Evaluation of parameter ALPHA for the semigreedy pricing

declare -a ALPHA_PRI=("0.0" "0.1" "0.2" "0.3")
declare VARIANT="4"
declare ALPHA_HEUR="0.1"
declare REPETITIONS_HEUR="500"

declare INPUT="../../instances/dpcp/random"
declare INSTANCES="$INPUT/instances.txt"
declare BIN="../../dpcp"
declare OUT="out/"

echo "Running experiment #4"

# First, create output directories
for a in "${ALPHA_PRI[@]}"
do
    mkdir -p "$OUT/a$a"
done

# Second, run experiments
while IFS= read -r LINE
do
    echo "Processing instance: $LINE"
    for a in "${ALPHA_PRI[@]}"
    do
        # Pricing #1: heur + exact
        time $BIN -s byp -f "$INPUT/$LINE" -o "$OUT/a$a/" --relax --heur-root 3 --heur-2step-variant $VARIANT --heur-semigreedy-iter $REPETITIONS_HEUR --heur-semigreedy-alpha $ALPHA_HEUR --feas-root 0 --feas-nodes 0 --inherit-cols 0 --pricing-pq-mwsp-off --pricing-p-mwsp-off --pricing-greedy-alpha $a --pricing-greedy-max-cols 10 --pricing-exact-time 900
    done
done < "$INSTANCES"
