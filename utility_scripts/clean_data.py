def clean_point_cloud_data(input_file, output_file):
    # Read all lines from input file
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    # Skip the first line (number of columns) and process points
    valid_points = []
    for line in lines[1:]:
        try:
            # Parse the line into x, y, z coordinates
            x, y, z = map(float, line.strip().split())
            
            # Only keep points with non-negative heights
            if z >= 0:
                valid_points.append((x, y, z))
        except ValueError:
            # Skip lines that can't be parsed
            continue
    
    # Write cleaned data to output file
    with open(output_file, 'w') as f:
        # First write the number of columns (which is also number of points in this case)
        f.write(f"{len(valid_points)}\n")
        
        # Write each valid point
        for x, y, z in valid_points:
            f.write(f"{x:.1f} {y:.1f} {z:.15f}\n")
    
    print(f"Original number of points: {len(lines) - 1}")
    print(f"Valid points after cleaning: {len(valid_points)}")
    print(f"Removed {len(lines) - 1 - len(valid_points)} points with negative heights")

if __name__ == "__main__":
    # Use your input and output file names here
    input_file = "AmesState.xyz"
    output_file = "cleaned_AmesState.xyz"
    
    clean_point_cloud_data(input_file, output_file)