#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "pointcloud.h"

void test_small_grid() {
    printf("\n=== Testing Small Grid ===\n");
    // Create a small test file
    FILE *f = fopen("test_grid.xyz", "w");
    if (!f) {
        printf("Failed to create test file\n");
        return;
    }
    
    // Write a 3x3 grid with varying heights
    fprintf(f, "3\n");  // 3 columns
    fprintf(f, "0.0 0.0 1.0\n");
    fprintf(f, "1.0 0.0 2.0\n");
    fprintf(f, "2.0 0.0 3.0\n");
    fprintf(f, "0.0 1.0 4.0\n");
    fprintf(f, "1.0 1.0 5.0\n");
    fprintf(f, "2.0 1.0 6.0\n");
    fprintf(f, "0.0 2.0 7.0\n");
    fprintf(f, "1.0 2.0 8.0\n");
    fprintf(f, "2.0 2.0 9.0\n");
    fclose(f);
    
    f = fopen("test_grid.xyz", "r");
    if (!f) {
        printf("Failed to open test file\n");
        return;
    }
    
    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);
    
    if (!pc) {
        printf("Failed to read pointcloud\n");
        return;
    }
    
    // Verify structure
    assert(pc->rows == 3 && pc->cols == 3 && "Grid dimensions incorrect");
    assert(pc->points.size == 9 && "Wrong number of points");
    
    // Test statistics
    assert(pc->stats.min_height == 1.0 && "Min height incorrect");
    assert(pc->stats.max_height == 9.0 && "Max height incorrect");
    
    // Generate visualization
    imagePointCloud(pc, "test_grid_output.gif");
    printf("Generated test_grid_output.gif\n");
    
    pointcloud_free(pc);
}

void test_ames_data() {
    printf("\n=== Testing Ames State Data ===\n");
    
    FILE *f = fopen("cleaned_AmesState.xyz", "r");
    if (!f) {
        printf("Failed to open cleaned_AmesState.xyz\n");
        printf("Make sure the file exists in the current directory\n");
        return;
    }
    
    // Read and verify data
    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);
    
    if (!pc) {
        printf("Failed to read Ames State pointcloud\n");
        return;
    }
    
    // Print statistics
    pointcloud_print_stats(pc);
    
    // Generate visualization
    printf("\nGenerating visualization of Ames State data...\n");
    imagePointCloud(pc, "ames_state_output.gif");
    printf("Generated ames_state_output.gif\n");
    
    // Verify the image was created
    FILE *check = fopen("ames_state_output.gif", "r");
    if (check) {
        printf("Successfully created output image\n");
        fclose(check);
    } else {
        printf("Failed to create output image\n");
    }
    
    pointcloud_free(pc);
}

void test_error_cases() {
    printf("\n=== Testing Error Cases ===\n");
    
    // Test NULL file
    pointcloud_t *pc = readPointCloudData(NULL);
    assert(pc == NULL && "Should return NULL for NULL input");
    
    // Test empty file
    FILE *f = fopen("empty.xyz", "w");
    fclose(f);
    f = fopen("empty.xyz", "r");
    pc = readPointCloudData(f);
    fclose(f);
    assert(pc == NULL && "Should return NULL for empty file");
    
    // Test invalid image generation
    imagePointCloud(NULL, "should_not_create.gif");
    FILE *check = fopen("should_not_create.gif", "r");
    assert(check == NULL && "Should not create image with NULL input");
    if (check) fclose(check);
    
    printf("Error case tests passed\n");
}

void test_initialize_watershed() {
    printf("\n=== Testing Initialize Watershed ===\n");

    // Create a small 3x3 test grid (same as before)
    FILE *f = fopen("test_watershed.xyz", "w");
    if (!f) {
        printf("Failed to create test file\n");
        return;
    }

    // Write a 3x3 grid
    fprintf(f, "9\n");
    fprintf(f, "0.0 0.0 1.0\n");  // (0,0) - Top left
    fprintf(f, "1.0 0.0 2.0\n");  // (0,1) - Top middle
    fprintf(f, "2.0 0.0 3.0\n");  // (0,2) - Top right
    fprintf(f, "0.0 1.0 4.0\n");  // (1,0) - Middle left
    fprintf(f, "1.0 1.0 5.0\n");  // (1,1) - Center
    fprintf(f, "2.0 1.0 6.0\n");  // (1,2) - Middle right
    fprintf(f, "0.0 2.0 7.0\n");  // (2,0) - Bottom left
    fprintf(f, "1.0 2.0 8.0\n");  // (2,1) - Bottom middle
    fprintf(f, "2.0 2.0 9.0\n");  // (2,2) - Bottom right
    fclose(f);

    // Rest of setup remains the same...
    f = fopen("test_watershed.xyz", "r");
    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);

    if (!pc) {
        printf("Failed to read test pointcloud\n");
        return;
    }

    printf("Initializing watershed...\n");
    int result = initializeWatershed(pc);
    if (result != 0) {
        printf("Failed to initialize watershed\n");
        pointcloud_free(pc);
        return;
    }

    // Test 1: Water initialization (remains the same)
    printf("\nChecking water initialization...\n");
    int water_init_passed = 1;
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (point->wd != 0.0) {
            printf("ERROR: Point at index %d has non-zero water: %f\n", i, point->wd);
            water_init_passed = 0;
        }
    }
    printf("Water initialization test: %s\n", water_init_passed ? "PASSED" : "FAILED");

    // Test 2: Check neighbor connections
    printf("\nChecking neighbor connections...\n");
    int connections_passed = 1;
    
    // Get center point (1,1)
    pcd_t *center = (pcd_t*)listGet(&pc->points, 4);
    if (!center) {
        printf("ERROR: Couldn't get center point\n");
        return;
    }

    printf("Center point (1,1) connections:\n");
    if (!center->north || !center->south || !center->east || !center->west) {
        printf("ERROR: Center point missing some connections\n");
        connections_passed = 0;
    }

    // Test 3: Check boundary conditions
    printf("\nChecking boundary conditions...\n");
    int boundary_passed = 1;

    // Test top-left corner (0,0)
    pcd_t *top_left = (pcd_t*)listGet(&pc->points, 0);
    if (top_left->north != NULL || !top_left->south || !top_left->east || top_left->west != NULL) {
        printf("ERROR: Top-left corner has incorrect connections\n");
        boundary_passed = 0;
    }

    // Test bottom-right corner (2,2)
    pcd_t *bottom_right = (pcd_t*)listGet(&pc->points, 8);
    if (!bottom_right->north || bottom_right->south != NULL || 
        bottom_right->east != NULL || !bottom_right->west) {
        printf("ERROR: Bottom-right corner has incorrect connections\n");
        boundary_passed = 0;
    }

    printf("\nBoundary conditions test: %s\n", boundary_passed ? "PASSED" : "FAILED");

    // Overall test results
    printf("\nOverall test results:\n");
    printf("Water initialization: %s\n", water_init_passed ? "PASSED" : "FAILED");
    printf("Neighbor connections: %s\n", connections_passed ? "PASSED" : "FAILED");
    printf("Boundary conditions: %s\n", boundary_passed ? "PASSED" : "FAILED");

    pointcloud_free(pc);
}

void test_add_uniform_water() {
    printf("\n=== Testing Add Uniform Water ===\n");

    // Create same 3x3 test grid
    FILE *f = fopen("test_watershed.xyz", "w");
    if (!f) {
        printf("Failed to create test file\n");
        return;
    }

    // Write test data
    fprintf(f, "9\n");
    fprintf(f, "0.0 0.0 1.0\n");
    fprintf(f, "1.0 0.0 2.0\n");
    fprintf(f, "2.0 0.0 3.0\n");
    fprintf(f, "0.0 1.0 4.0\n");
    fprintf(f, "1.0 1.0 5.0\n");
    fprintf(f, "2.0 1.0 6.0\n");
    fprintf(f, "0.0 2.0 7.0\n");
    fprintf(f, "1.0 2.0 8.0\n");
    fprintf(f, "2.0 2.0 9.0\n");
    fclose(f);

    f = fopen("test_watershed.xyz", "r");
    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);

    if (!pc) {
        printf("Failed to read test pointcloud\n");
        return;
    }

    // Initialize watershed
    initializeWatershed(pc);

    // Test 1: Add water and verify
    double water_amount = 2.5;
    printf("Adding %.1f units of water to each point...\n", water_amount);
    watershedAddUniformWater(pc, water_amount);

    // Verify water was added correctly
    int water_test_passed = 1;
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (point->wd != water_amount) {
            printf("ERROR: Point %d has incorrect water amount: %.1f (expected %.1f)\n",
                   i, point->wd, water_amount);
            water_test_passed = 0;
        }
    }
    
    // Test 2: Add more water
    printf("\nAdding another %.1f units of water...\n", water_amount);
    watershedAddUniformWater(pc, water_amount);

    // Verify cumulative water amount
    double expected_total = water_amount * 2;
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (point->wd != expected_total) {
            printf("ERROR: Point %d has incorrect water amount: %.1f (expected %.1f)\n",
                   i, point->wd, expected_total);
            water_test_passed = 0;
        }
    }

    // Test 3: Try adding negative water (should be ignored)
    printf("\nTrying to add negative water (should be ignored)...\n");
    watershedAddUniformWater(pc, -1.0);
    
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (point->wd != expected_total) {
            printf("ERROR: Water amount changed after negative water addition\n");
            water_test_passed = 0;
        }
    }

    printf("\nWater addition test: %s\n", water_test_passed ? "PASSED" : "FAILED");

    pointcloud_free(pc);
}

void test_watershed_step() {
    printf("\n=== Testing Watershed Step ===\n");

    // Create a test grid where center is lower than surroundings to test water flow
    FILE *f = fopen("test_watershed_step.xyz", "w");
    if (!f) {
        printf("Failed to create test file\n");
        return;
    }

    // Write a 3x3 grid with a depression in the center
    fprintf(f, "9\n");  // 9 points total
    fprintf(f, "0.0 0.0 10.0\n");  // Higher elevation corners
    fprintf(f, "1.0 0.0 10.0\n");
    fprintf(f, "2.0 0.0 10.0\n");
    fprintf(f, "0.0 1.0 10.0\n");
    fprintf(f, "1.0 1.0 5.0\n");   // Center is lower
    fprintf(f, "2.0 1.0 10.0\n");
    fprintf(f, "0.0 2.0 10.0\n");
    fprintf(f, "1.0 2.0 10.0\n");
    fprintf(f, "2.0 2.0 10.0\n");
    fclose(f);

    f = fopen("test_watershed_step.xyz", "r");
    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);

    if (!pc) {
        printf("Failed to read test pointcloud\n");
        return;
    }

    // Initialize watershed
    printf("Initializing watershed...\n");
    initializeWatershed(pc);

    // Add uniform water
    printf("Adding initial water...\n");
    watershedAddUniformWater(pc, 2.0);  // Add 2 units of water to each cell

    // Print initial state
    printf("\nInitial state:\n");
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        printf("Point %d: elevation=%.1f, water=%.1f\n", 
               i, point->z, point->wd);
    }

    // Perform one step
    printf("\nPerforming watershed step...\n");
    watershedStep(pc);

    // Print state after one step
    printf("\nState after one step:\n");
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        printf("Point %d: elevation=%.1f, water=%.1f\n", 
               i, point->z, point->wd);
    }

    // Verify conditions
    int test_passed = 1;
    
    // Get center point
    pcd_t *center = (pcd_t*)listGet(&pc->points, 4);
    if (!center) {
        printf("ERROR: Couldn't get center point\n");
        test_passed = 0;
    } else {
        // Center should have more water than starting amount due to flow
        if (center->wd <= 2.0) {
            printf("ERROR: Center point didn't accumulate water as expected\n");
            test_passed = 0;
        }
        
        // Check surrounding points have less water due to flow and evaporation
        for (int i = 0; i < pc->points.size; i++) {
            if (i == 4) continue; // Skip center
            pcd_t *point = (pcd_t*)listGet(&pc->points, i);
            if (point->wd >= 2.0) {
                printf("ERROR: Point %d didn't lose water as expected\n", i);
                test_passed = 0;
            }
        }
    }

    // Perform multiple steps to see if water continues to move
    printf("\nPerforming multiple steps...\n");
    for (int step = 0; step < 5; step++) {
        watershedStep(pc);
        
        // Print center point water level after each step
        pcd_t *center = (pcd_t*)listGet(&pc->points, 4);
        printf("Step %d: Center water level = %.2f\n", step + 2, center->wd);
    }

    printf("\nWatershed step test: %s\n", test_passed ? "PASSED" : "FAILED");

    pointcloud_free(pc);
}

void test_image_point_cloud_water() {
    printf("\n=== Testing Image Point Cloud Water ===\n");

    // Open the Ames State data
    FILE *f = fopen("cleaned_AmesState.xyz", "r");
    if (!f) {
        printf("Failed to open cleaned_AmesState.xyz\n");
        return;
    }

    printf("Reading Ames State data...\n");
    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);

    if (!pc) {
        printf("Failed to read Ames State pointcloud\n");
        return;
    }

    // Print initial statistics
    printf("\nInitial state:\n");
    printf("Dimensions: %d rows x %d columns\n", pc->rows, pc->cols);
    printf("Height range: %.2f to %.2f\n", pc->stats.min_height, pc->stats.max_height);

    // Initialize watershed
    printf("\nInitializing watershed...\n");
    initializeWatershed(pc);

    // Add initial water (small amount relative to height differences)
    double initial_water = 2.0;
    printf("Adding %.1f units of water...\n", initial_water);
    watershedAddUniformWater(pc, initial_water);

    // Run several watershed steps
    printf("\nRunning watershed simulation...\n");
    int steps = 10;
    for (int i = 0; i < steps; i++) {
        watershedStep(pc);
        
        // Print periodic status
        if (i % 2 == 0) {
            // Sample a few points to show water movement
            pcd_t *sample1 = (pcd_t*)listGet(&pc->points, 0);                    // Corner
            pcd_t *sample2 = (pcd_t*)listGet(&pc->points, pc->points.size / 2);  // Middle
            printf("Step %d: Corner water=%.2f, Middle water=%.2f\n", 
                   i + 1, sample1->wd, sample2->wd);
        }
    }

    // Generate two visualizations
    printf("\nGenerating visualizations...\n");
    
    // First, regular terrain visualization
    printf("Saving terrain visualization to terrain.gif\n");
    imagePointCloud(pc, "terrain.gif");
    
    // Then, water visualization
    // Use maxwd slightly higher than initial water to show variation
    printf("Saving water visualization to water_ames.gif\n");
    imagePointCloudWater(pc, initial_water * 2, "water_ames.gif");

    printf("\nVisualization files created:\n");
    printf("1. terrain.gif - Regular terrain visualization\n");
    printf("2. water_ames.gif - Water accumulation visualization\n");
    printf("\nPlease compare these files to verify water accumulation in low areas\n");

    pointcloud_free(pc);
}

int main() {
    printf("Starting pointcloud tests...\n");
    
    test_error_cases();
    test_small_grid();
    test_initialize_watershed();
    test_add_uniform_water();
    test_watershed_step();
    test_image_point_cloud_water();  // Add this line
    test_ames_data();
    
    printf("\nAll tests completed!\n");
    return 0;
}