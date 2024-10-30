#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include "pointcloud.h"

/**
 * Method to print out the height, grid location of the lowest and highest point in the automata.
 * Now reads from standard input instead of a file.
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

List* readPointCloudData(FILE *stream) {
    if (stream == NULL) {
        fprintf(stderr, "Error: NULL stream provided\n");
        return NULL; 
    }

    int columns; 
    if (fscanf(stream, "%d", &columns) != 1) {
        fprintf(stderr, "Error: Could not read number of columns\n");
        return NULL; 
    }
    printf("Number of columns read: %d\n", columns);

    List* pointList = (List*)malloc(sizeof(List)); 
    if (pointList == NULL) {
        fprintf(stderr, "Error: Failed to allocate pointList\n");
        return NULL;
    }

    if (!listInit(pointList, sizeof(pcd_t))) {
        fprintf(stderr, "Error: Failed to initialize list\n");
        free(pointList); 
        return NULL; 
    }

    double x, y, z; 
    int pointCount = 0;
    double minZ = DBL_MAX, maxZ = -DBL_MAX;  // Track height range for verification
    
    // Print first few points for verification
    while (fscanf(stream, "%lf %lf %lf", &x, &y, &z) == 3) {
        // Print first 5 points for debugging
        if (pointCount < 5) {
            printf("Point %d: (%.2f, %.2f, %.2f)\n", pointCount, x, y, z);
        }
        
        // Track height range
        if (z < minZ) minZ = z;
        if (z > maxZ) maxZ = z;

        pcd_t point = {
            .x = x, 
            .y = y, 
            .z = z,
            .water = 0.0, 
            .north = NULL, 
            .south = NULL, 
            .east = NULL, 
            .west = NULL
        }; 

        listAddEnd(pointList, &point);
        pointCount++;
    }

    if (pointList->size == 0) {
        fprintf(stderr, "Error: No points were read\n");
        free(pointList->data); 
        free(pointList); 
        return NULL; 
    }

    printf("Successfully read %d points\n", pointCount);
    printf("Height range: %.2f to %.2f\n", minZ, maxZ);
    printf("First coordinate: (%.2f, %.2f)\n", 
           ((pcd_t*)listGet(pointList, 0))->x,
           ((pcd_t*)listGet(pointList, 0))->y);
    printf("Last coordinate: (%.2f, %.2f)\n", 
           ((pcd_t*)listGet(pointList, pointCount-1))->x,
           ((pcd_t*)listGet(pointList, pointCount-1))->y);

    return pointList;
}

void imagePointCloud(List *l, char *filename) {
    if (l == NULL || l->size == 0 || filename == NULL) {
        fprintf(stderr, "Invalid parameters passed to imagePointCloud\n");
        return;
    }

    // 1. Find min and max height values
    double minHeight = DBL_MAX;
    double maxHeight = -DBL_MAX;
    double minX = DBL_MAX, maxX = -DBL_MAX;
    double minY = DBL_MAX, maxY = -DBL_MAX;

    for (int i = 0; i < l->size; i++) {
        pcd_t *point = (pcd_t *)listGet(l, i);
        if (!point) continue;
        
        minHeight = fmin(minHeight, point->z);
        maxHeight = fmax(maxHeight, point->z);
        minX = fmin(minX, point->x);
        maxX = fmax(maxX, point->x);
        minY = fmin(minY, point->y);
        maxY = fmax(maxY, point->y);
    }

    printf("Height range: [%.2f, %.2f]\n", minHeight, maxHeight);

    // Create bitmap
    int size = 800;
    Bitmap *bmp = bm_create(size, size);
    if (!bmp) {
        fprintf(stderr, "Failed to create bitmap\n");
        return;
    }

    // Fill with white background
    bm_set_color(bmp, bm_rgb(255, 255, 255));
    bm_clear(bmp);

    // Calculate scaling factors for x,y coordinates
    double scaleX = (double)(size - 1) / (maxX - minX);
    double scaleY = (double)(size - 1) / (maxY - minY);
    double scale = fmin(scaleX, scaleY);

    // Arrays for accumulating heights and counts
    double *heightSum = calloc(size * size, sizeof(double));
    int *heightCount = calloc(size * size, sizeof(int));
    
    if (!heightSum || !heightCount) {
        free(heightSum);
        free(heightCount);
        bm_free(bmp);
        return;
    }

    // Accumulate heights for each pixel
    for (int i = 0; i < l->size; i++) {
        pcd_t *point = (pcd_t *)listGet(l, i);
        if (!point) continue;

        int x = (int)((point->x - minX) * scale);
        int y = size - 1 - (int)((point->y - minY) * scale);

        if (x >= 0 && x < size && y >= 0 && y < size) {
            int idx = y * size + x;
            heightSum[idx] += point->z;
            heightCount[idx]++;
        }
    }

    // Render pixels with simple linear mapping to grayscale
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int idx = y * size + x;
            if (heightCount[idx] > 0) {
                // Calculate average height for this pixel
                double avgHeight = heightSum[idx] / heightCount[idx];
                
                // Simple linear mapping to 0-255 range
                double heightRange = maxHeight - minHeight;
                int intensity = (int)(((avgHeight - minHeight) / heightRange) * 255);
                
                // Set the pixel color (grayscale)
                bm_set_color(bmp, bm_rgb(intensity, intensity, intensity));
                bm_putpixel(bmp, x, y);
            }
        }
    }

    free(heightSum);
    free(heightCount);

    // Save the image
    printf("Saving image to %s...\n", filename);
    if (!bm_save(bmp, filename)) {
        fprintf(stderr, "Failed to save bitmap\n");
    }

    bm_free(bmp);
}