import math
import random

def generate_test_data():
    # Smaller dimensions for test data
    width = 4  # 4x4 grid = 16 points
    height = 4
    
    points = []
    total_height = 0
    
    with open('test_partb.xyz', 'w') as f:
        # Write number of points
        f.write(f"{width * height}\n")
        
        # Generate corner points to ensure min=0 and max=10
        points.append((0.0, 0.0, 0.0))  # min height
        points.append((3.0, 3.0, 10.0)) # max height
        
        # Generate remaining points with heights between 0-2
        # to achieve average height of ~1
        remaining_points = (width * height) - 2
        remaining_height = (width * height - 10) # Target total for avg=1
        
        for y in range(height):
            for x in range(width):
                if len(points) < width * height:
                    if (x,y) not in [(0,0), (3,3)]:
                        height_val = remaining_height / (remaining_points - len(points) + 2)
                        points.append((float(x), float(y), height_val))
        
        # Write all points
        for x, y, h in points:
            f.write(f"{x:.1f} {y:.1f} {h:.15f}\n")

if __name__ == "__main__":
    generate_test_data()