#!/bin/bash

# Check if 3 arguments are provided
if [ "$#" -ne 3 ]; then
    echo "Usage: ./run_tests.sh <input_directory> <output_directory> <cycle_count>"
    exit 1
fi

# Get input arguments
input_dir="$1"
output_dir="$2"
cycle_count="$3"

# Create output directory if it doesn't exist
mkdir -p "$output_dir"

# Loop through all files in the input directory
for file in "$input_dir"/*.txt; do
    # Extract filename without path
    filename=$(basename "$file" .txt)
    
    # Run forward and noforward executables with the file and save outputs
    ./forward "$file" "$cycle_count" > "$output_dir/${filename}_forward_out.txt"
    ./noforward "$file" "$cycle_count" > "$output_dir/${filename}_noforward_out.txt"
    
    echo "Processed: $filename"
done

echo "✅ All tests executed. Outputs saved in $output_dir"
