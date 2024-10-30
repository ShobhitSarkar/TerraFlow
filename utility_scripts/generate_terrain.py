import math

def generate_terrain_data():
    # Set dimensions
    width = 1000
    height = 1000
    
    base_x = 445000.5
    base_y = 4650999.5
    
    with open('terrain.xyz', 'w') as f:
        # Write the number of columns first
        f.write(f"{width}\n")
        
        # Generate terrain data with more pronounced features
        for y in range(height):
            for x in range(width):
                curr_x = base_x + x
                curr_y = base_y - y
                
                # Increase amplitude and adjust frequencies for more distinct features
                height_base = 304.0
                height_variation = (
                    30.0 * math.sin(x/100.0) * math.cos(y/100.0) +  # Large features
                    15.0 * math.sin(x/20.0) * math.cos(y/20.0) +    # Medium features
                    5.0 * math.sin(x/5.0) * math.cos(y/5.0)         # Small features
                )
                
                height = height_base + height_variation
                
                f.write(f"{curr_x:.1f} {curr_y:.1f} {height:.15f}\n")

if __name__ == "__main__":
    generate_terrain_data()