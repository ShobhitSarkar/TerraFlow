#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "pointcloud.h"

/**
 * Analyzes point cloud data from standard input
 * Input: Standard input 
 * Output: Prints min/max heights and their locations, average height 
 */
void stat1() {
    double x, y, height; // 
    double minHeight = DBL_MAX;
    double maxHeight = -DBL_MAX;
    double sumHeight = 0;
    int count = 0;
    double minX = 0, minY = 0, maxX = 0, maxY = 0;

    while (scanf("%lf %lf %lf", &x, &y, &height) == 3) {
        if (height < minHeight) {
            minHeight = height;
            minX = x;
            minY = y;
        }
        if (height > maxHeight) {
            maxHeight = height;
            maxX = x;
            maxY = y;
        }
        sumHeight += height;
        count++;
    }

    if (count > 0) {
        double avgHeight = sumHeight / count;
        printf("Minimum height: %.2f at grid location (%.1f, %.1f)\n", minHeight, minX, minY);
        printf("Maximum height: %.2f at grid location (%.1f, %.1f)\n", maxHeight, maxX, maxY);
        printf("Average height of all data points: %.2f\n", avgHeight);
    } else {
        printf("No data points found in the input.\n");
    }
}

/**
 * Reads point cloud data from a file stream and handles memory allocation and statistics 
 * Input: FILE* stream (an input file strem)
 * Output: Populated pointcloud_t struct or NULL if there's an error 
 */
pointcloud_t* readPointCloudData(FILE *stream) {
    if (stream == NULL) {
        fprintf(stderr, "Error: NULL stream provided\n");
        return NULL;
    }

    pointcloud_t *pc = malloc(sizeof(pointcloud_t));
    if (!pc) {
        fprintf(stderr, "Error: Failed to allocate pointcloud structure\n");
        return NULL;
    }

    pc->water_coef = 0.1; 
    pc->evap_coef = 0.95; 

    // Read number of columns
    int total_points;
    if (fscanf(stream, "%d", &total_points) != 1) {
        fprintf(stderr, "Error: Could not read number of points\n");
        free(pc);
        return NULL;
    }

    // Initialize points list
    if (!listInit(&pc->points, sizeof(pcd_t))) {
        fprintf(stderr, "Error: Failed to initialize points list\n");
        free(pc);
        return NULL;
    }

    // Initialize statistics
    pc->stats.min_height = DBL_MAX;
    pc->stats.max_height = -DBL_MAX;
    pc->stats.min_x = DBL_MAX;
    pc->stats.max_x = -DBL_MAX;
    pc->stats.min_y = DBL_MAX;
    pc->stats.max_y = -DBL_MAX;
    double height_sum = 0;
    int point_count = 0;

    // First pass: Read all points and find min/max x,y coordinates
    double x, y, z;
    while (fscanf(stream, "%lf %lf %lf", &x, &y, &z) == 3) {
        pcd_t point = {
            .x = x, .y = y, .z = z,
            .wd = 0.0,
            .north = NULL, .south = NULL,
            .east = NULL, .west = NULL
        };

        // Update statistics
        if (z < pc->stats.min_height) pc->stats.min_height = z;
        if (z > pc->stats.max_height) pc->stats.max_height = z;
        if (x < pc->stats.min_x) pc->stats.min_x = x;
        if (x > pc->stats.max_x) pc->stats.max_x = x;
        if (y < pc->stats.min_y) pc->stats.min_y = y;
        if (y > pc->stats.max_y) pc->stats.max_y = y;
        
        height_sum += z;
        point_count++;

        listAddEnd(&pc->points, &point);
    }

    // Calculate grid dimensions
    double x_step = (pc->stats.max_x - pc->stats.min_x) / (sqrt(point_count) - 1);
    double y_step = (pc->stats.max_y - pc->stats.min_y) / (sqrt(point_count) - 1);
    
    pc->cols = (int)((pc->stats.max_x - pc->stats.min_x) / x_step + 0.5) + 1;
    pc->rows = (int)((pc->stats.max_y - pc->stats.min_y) / y_step + 0.5) + 1;
    
    pc->stats.avg_height = height_sum / point_count;

    printf("Grid Analysis:\n");
    printf("Total points: %d\n", point_count);
    printf("Calculated dimensions: %d rows x %d columns\n", pc->rows, pc->cols);
    printf("X step size: %.2f\n", x_step);
    printf("Y step size: %.2f\n", y_step);

    return pc;
}


void pointcloud_free(pointcloud_t *pc){
    if (!pc){
        return; 
    }

    if (pc-> points.data){
        free(pc->points.data); 
    }

    free(pc); 
}

/**
 * Writing a new function to print out the pointcloud data inorder to work with the 
 * new input format. 
 */
void pointcloud_print_stats(const pointcloud_t *pc){
    if (!pc){
        printf("Null pointcloud\n"); 
        return; 
    }

    printf("\nPointcloud Statistics:\n");
    printf("=====================\n");
    printf("Dimensions: %d rows × %d columns (%d total points)\n", 
           pc->rows, pc->cols, pc->points.size);
    printf("Height range: %.2f to %.2f (avg: %.2f)\n", 
           pc->stats.min_height, pc->stats.max_height, pc->stats.avg_height);
    printf("X range: %.2f to %.2f\n", pc->stats.min_x, pc->stats.max_x);
    printf("Y range: %.2f to %.2f\n", pc->stats.min_y, pc->stats.max_y);
    printf("Memory usage: %zu bytes\n", 
           sizeof(pointcloud_t) + pc->points.size * sizeof(pcd_t));

}

pcd_t* pointcloud_get_point(pointcloud_t *pc, int row, int col) {
    if (!pc || row < 0 || row >= pc->rows || col < 0 || col >= pc->cols) {
        return NULL;
    }
    
    int index = row * pc->cols + col;
    return (pcd_t*)listGet(&pc->points, index);
}

/**
 * Generates a visualization of terrain according to given input, maps height values to grayscale
 * Inputs: 
 *  - pc: point cloud data 
 *  - filename: output file name 
 */
void imagePointCloud(pointcloud_t *pc, char *filename) {
    if (!pc || !pc->points.data || !filename) {
        fprintf(stderr, "Invalid parameters passed to imagePointCloud\n");
        return;
    }

    // Creating the bitmap image 
    int size = 800;
    Bitmap *bmp = bm_create(size, size);
    if (!bmp) {
        fprintf(stderr, "Failed to create bitmap\n");
        return;
    }

    //Fill the entire background with black color
    bm_set_color(bmp, bm_rgb(0, 0, 0));
    bm_clear(bmp);

    // Scale colors according to heights 
    // I noticed that doing this significantly improved the contrast and clarity of the image 
    double scaleX = (double)(size - 1) / (pc->stats.max_x - pc->stats.min_x);
    double scaleY = (double)(size - 1) / (pc->stats.max_y - pc->stats.min_y);
    double scale = fmin(scaleX, scaleY);

    // Use a 2D array to work with points
    double **heightSum = malloc(size * sizeof(double*));
    int **heightCount = malloc(size * sizeof(int*));
    for(int i = 0; i < size; i++) {
        heightSum[i] = calloc(size, sizeof(double));
        heightCount[i] = calloc(size, sizeof(int));
    }
    
    if (!heightSum || !heightCount) {
        printf("It seems like something is wrong with your dataset. Please check.\n");
        return;
    }

    //Iterate through the points and find the min and max heights of the points 
    double mappedMin = DBL_MAX;
    double mappedMax = -DBL_MAX;
    
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t *)listGet(&pc->points, i);
        if (!point) continue;

        int x = (int)((point->x - pc->stats.min_x) * scale);
        int y = size - 1 - (int)((point->y - pc->stats.min_y) * scale);

        if (x >= 0 && x < size && y >= 0 && y < size) {
            heightSum[y][x] += point->z;
            heightCount[y][x]++;
            
            double avgHeight = heightSum[y][x] / heightCount[y][x];
            if (avgHeight < mappedMin) mappedMin = avgHeight;
            if (avgHeight > mappedMax) mappedMax = avgHeight;
        }
    }

    // Render the image 
    double heightRange = mappedMax - mappedMin;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (heightCount[y][x] > 0) {
                double avgHeight = heightSum[y][x] / heightCount[y][x];
                
                // Improve contrast with better scaling
                int intensity = (int)(((avgHeight - mappedMin) / heightRange) * 255);
                intensity = intensity < 0 ? 0 : (intensity > 255 ? 255 : intensity);
                
                bm_set_color(bmp, bm_rgb(intensity, intensity, intensity));
                bm_putpixel(bmp, x, y);
            }
        }
    }

    // Dealloc memory 
    for(int i = 0; i < size; i++) {
        free(heightSum[i]);
        free(heightCount[i]);
    }
    free(heightSum);
    free(heightCount);

    printf("Saving image to %s...\n", filename);
    if (!bm_save(bmp, filename)) {
        fprintf(stderr, "Failed to save bitmap\n");
    }

    bm_free(bmp);
}

void update_watershed_coefficients(pointcloud_t *pc, double wcoef, double ecoef){
    if (!pc) return;
    
    if (wcoef < 0.0 || wcoef > 0.2) {
        fprintf(stderr, "Invalid water coefficient (must be 0.0-0.2)\n");
        return;
    }
    if (ecoef < 0.9 || ecoef > 1.0) {
        fprintf(stderr, "Invalid evaporation coefficient (must be 0.9-1.0)\n");
        return;
    }
    
    pc -> water_coef = wcoef;
    pc -> evap_coef = ecoef;
}

/**
 * Prepares the pointcloud for water simulation: 
 *  - sets initial water to 0 
 *  - establishes pointers to the neighbors 
 *  - validates the grid 
 * Input: 
 *  - pc: point cloud to initialize 
 * Returns: 0 on sucess, -1 on failure 
 * 
 */
int initializeWatershed(pointcloud_t *pc) {
    if (!pc || !pc->points.data) {
        return -1;  // Invalid input
    }

    // For each point in the grid
    for (int row = 0; row < pc->rows; row++) {
        for (int col = 0; col < pc->cols; col++) {
            // Get current point
            int index = row * pc->cols + col;
            pcd_t *current = (pcd_t*)listGet(&pc->points, index);
            if (!current) continue;

            // Initialize water to 0
            current->wd = 0.0;

            // Set north neighbor (row - 1)
            if (row > 0) {
                current->north = (pcd_t*)listGet(&pc->points, (row-1) * pc->cols + col);
            } else {
                current->north = NULL;
            }

            // Set south neighbor (row + 1)
            if (row < pc->rows - 1) {
                current->south = (pcd_t*)listGet(&pc->points, (row+1) * pc->cols + col);
            } else {
                current->south = NULL;
            }

            // Set east neighbor (col + 1)
            if (col < pc->cols - 1) {
                current->east = (pcd_t*)listGet(&pc->points, row * pc->cols + (col+1));
            } else {
                current->east = NULL;
            }

            // Set west neighbor (col - 1)
            if (col > 0) {
                current->west = (pcd_t*)listGet(&pc->points, row * pc->cols + (col-1));
            } else {
                current->west = NULL;
            }

            // Debug output for corner points
            if ((row == 0 && col == 0) || (row == 0 && col == pc->cols-1) ||
                (row == pc->rows-1 && col == 0) || (row == pc->rows-1 && col == pc->cols-1)) {
                printf("Corner point at (%d,%d):\n", row, col);
                printf("  North: %s\n", current->north ? "Present" : "NULL");
                printf("  South: %s\n", current->south ? "Present" : "NULL");
                printf("  East: %s\n", current->east ? "Present" : "NULL");
                printf("  West: %s\n", current->west ? "Present" : "NULL");
            }
        }
    }

    return 0;
}

/**
 * Function to add water to all points equally 
 * Inputs: 
 *  - pc: point cloud data 
 *  - amoung: amount of water to add 
 * Note: ignores negative input 
 */
void watershedAddUniformWater(pointcloud_t *pc, double amount) {
    if (!pc || !pc->points.data || amount < 0) {
        return;
    }

    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (point) {
            point->wd += amount;
        }
    }
}

/**
 * Simulates a single step of water movement 
 * Input: 
 *  - pc: point cloud to process 
 */
void watershedStep(pointcloud_t *pc) {
    if (!pc || !pc->points.data) {
        return;
    }

    if (!pc || !pc->points.data || 
        pc->water_coef < 0.0 || pc->water_coef > 0.2 || 
        pc->evap_coef < 0.9 || pc->evap_coef > 1.0) {
        fprintf(stderr, "Invalid parameters in watershedStep\n");
        return;
    }

    // Allocate memory for new water amounts
    double *new_water = calloc(pc->points.size, sizeof(double));
    if (!new_water) return;

    // For each cell C in the pointcloud
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *C = (pcd_t*)listGet(&pc->points, i);
        if (!C) continue;

        double total_change = 0.0;

        // Process each neighbor using formula f(t1, w1, t2, w2) = (t2 + w2) - (t1 + w1)
        // West neighbor
        if (C->west) {
            double t1 = C->z;
            double w1 = C->wd;
            double t2 = C->west->z;
            double w2 = C->west->wd;
            double flow = (t2 + w2) - (t1 + w1);
            total_change += flow;
        }

        // East neighbor
        if (C->east) {
            double t1 = C->z;
            double w1 = C->wd;
            double t2 = C->east->z;
            double w2 = C->east->wd;
            double flow = (t2 + w2) - (t1 + w1);
            total_change += flow;
        }

        // North neighbor
        if (C->north) {
            double t1 = C->z;
            double w1 = C->wd;
            double t2 = C->north->z;
            double w2 = C->north->wd;
            double flow = (t2 + w2) - (t1 + w1);
            total_change += flow;
        }

        // South neighbor
        if (C->south) {
            double t1 = C->z;
            double w1 = C->wd;
            double t2 = C->south->z;
            double w2 = C->south->wd;
            double flow = (t2 + w2) - (t1 + w1);
            total_change += flow;
        }

        // Apply water flow coefficient
        total_change *= pc->water_coef;

        // Calculate new water amount
        new_water[i] = C->wd + total_change;

        // Apply evaporation coefficient
        new_water[i] *= pc->evap_coef;

        // Ensure non-negative water amount
        if (new_water[i] < 0) {
            new_water[i] = 0;
        }
    }

    // Update all points with new water amounts
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (point) {
            point->wd = new_water[i];
        }
    }

    // Free temporary array
    free(new_water);
}

/**
 * Visualizes the water accumulation, core function to visualize water flow 
 */
void imagePointCloudWater(pointcloud_t* pc, double maxwd, char* filename) {
    if (!pc || !pc->points.data || !filename) {
        fprintf(stderr, "Invalid parameters passed to imagePointCloudWater\n");
        return;
    }

    // Create bitmap
    int size = 800;
    Bitmap *bmp = bm_create(size, size);
    if (!bmp) {
        fprintf(stderr, "Failed to create bitmap\n");
        return;
    }

    // Calculate scaling factors for x,y coordinates
    double scaleX = (double)(size - 1) / (pc->stats.max_x - pc->stats.min_x);
    double scaleY = (double)(size - 1) / (pc->stats.max_y - pc->stats.min_y);
    double scale = fmin(scaleX, scaleY);

    // Arrays for accumulating heights and water
    double **heights = malloc(size * sizeof(double*));
    double **water = malloc(size * sizeof(double*));
    int **counts = malloc(size * sizeof(int*));
    
    for(int i = 0; i < size; i++) {
        heights[i] = calloc(size, sizeof(double));
        water[i] = calloc(size, sizeof(double));
        counts[i] = calloc(size, sizeof(int));
    }

    // Accumulate values
    for (int i = 0; i < pc->points.size; i++) {
        pcd_t *point = (pcd_t*)listGet(&pc->points, i);
        if (!point) continue;

        int x = (int)((point->x - pc->stats.min_x) * scale);
        int y = size - 1 - (int)((point->y - pc->stats.min_y) * scale); // Flip Y coordinate

        if (x >= 0 && x < size && y >= 0 && y < size) {
            heights[y][x] += point->z;
            water[y][x] += point->wd;
            counts[y][x]++;
        }
    }

    // Render pixels
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            if (counts[y][x] > 0) {
                double avg_height = heights[y][x] / counts[y][x];
                double avg_water = water[y][x] / counts[y][x];
                
                // Calculate terrain color (grayscale based on height)
                double height_factor = (avg_height - pc->stats.min_height) / 
                                     (pc->stats.max_height - pc->stats.min_height);
                int terrain = (int)(height_factor * 255);
                terrain = terrain < 0 ? 0 : (terrain > 255 ? 255 : terrain);

                // Calculate water color (blue based on water amount)
                double water_factor = avg_water / maxwd;
                water_factor = water_factor > 1.0 ? 1.0 : water_factor;
                int blue = (int)(water_factor * 255);

                // Blend terrain and water colors
                int r = terrain * (1 - water_factor);
                int g = terrain * (1 - water_factor);
                int b = terrain * (1 - water_factor) + blue;

                bm_set_color(bmp, bm_rgb(r, g, b));
                bm_putpixel(bmp, x, y);
            }
        }
    }

    // Cleanup
    for(int i = 0; i < size; i++) {
        free(heights[i]);
        free(water[i]);
        free(counts[i]);
    }
    free(heights);
    free(water);
    free(counts);

    // Save image
    printf("Saving water visualization to %s...\n", filename);
    if (!bm_save(bmp, filename)) {
        fprintf(stderr, "Failed to save bitmap\n");
    }

    bm_free(bmp);
}