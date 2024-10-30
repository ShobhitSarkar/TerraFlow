#include <stdio.h>
#include <stdlib.h>
#include "util.h"
#include "pointcloud.h"

#define ROWS 3
#define COLS 4

void test_allocateArray() {
    double **array;
    int i, j;

    // Allocate the array
    array = (double **)allocateArray(ROWS, COLS);
    
    if (array == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    // Check if all elements are initialized to 0
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            if (array[i][j] != 0.0) {
                printf("Test failed: Element at [%d][%d] is not 0\n", i, j);
                return;
            }
        }
    }

    // If we've made it here, all elements are 0
    printf("Test passed: All elements are initialized to 0\n");

    // Test the ACCESS_2D macro
    int index = ACCESS_2D(1, 2, COLS);
    if (index == 6) {
        printf("Test passed: ACCESS_2D macro calculates correct index\n");
    } else {
        printf("Test failed: ACCESS_2D macro calculated incorrect index. Expected 6, got %d\n", index);
    }

    // Free the allocated memory
    for (i = 0; i < ROWS; i++) {
        free(array[i]);
    }
    free(array);
}

void test_pcd_structure() {
    pcd_t point;
    
    // Initialize the point
    point.x = 1.0;
    point.y = 2.0;
    point.z = 3.0;
    point.water = 0.5;
    point.north = NULL;
    point.south = NULL;
    point.east = NULL;
    point.west = NULL;

    // Test if the values are correctly assigned
    if (point.x == 1.0 && point.y == 2.0 && point.z == 3.0 && point.water == 0.5 &&
        point.north == NULL && point.south == NULL && point.east == NULL && point.west == NULL) {
        printf("Test passed: pcd_t structure works correctly\n");
    } else {
        printf("Test failed: pcd_t structure does not work as expected\n");
    }
}

void test_list() {
    List list;
    int result = listInit(&list, sizeof(int));
    if (!result) {
        printf("Test failed: List initialization failed\n");
        return;
    }

    // Test adding elements
    for (int i = 0; i < 15; i++) {
        listAddEnd(&list, &i);
    }

    // Test getting elements
    for (int i = 0; i < 15; i++) {
        int *value = (int*)listGet(&list, i);
        if (value == NULL || *value != i) {
            printf("Test failed: Incorrect value at index %d\n", i);
            return;
        }
    }

    printf("Test passed: List operations work correctly\n");

    // Clean up
    free(list.data);
}

void test_readPointCloudData() {
    // Create a temporary file with test data
    FILE *tmp = tmpfile();
    if (tmp == NULL) {
        printf("Test failed: Could not create temporary file\n");
        return;
    }

    // Write test data to temporary file
    fprintf(tmp, "4\n");  // number of columns
    fprintf(tmp, "445000.5 4650999.5 304.95\n");
    fprintf(tmp, "445001.5 4650999.5 304.95\n");
    fprintf(tmp, "445002.5 4650999.5 304.96\n");
    fprintf(tmp, "445003.5 4650999.5 304.94\n");
    
    // Rewind file to beginning
    rewind(tmp);

    // Read the data
    List* pointList = readPointCloudData(tmp);
    
    if (pointList == NULL) {
        printf("Test failed: readPointCloudData returned NULL\n");
        fclose(tmp);
        return;
    }

    // Verify number of points read
    if (pointList->size != 4) {
        printf("Test failed: Expected 4 points, got %d\n", pointList->size);
        free(pointList->data);
        free(pointList);
        fclose(tmp);
        return;
    }

    // Check first point's values
    pcd_t* first_point = (pcd_t*)listGet(pointList, 0);
    if (first_point->x != 445000.5 || first_point->y != 4650999.5 || first_point->z != 304.95) {
        printf("Test failed: First point has incorrect values\n");
        free(pointList->data);
        free(pointList);
        fclose(tmp);
        return;
    }

    printf("Test passed: readPointCloudData works correctly\n");

    // Clean up
    free(pointList->data);
    free(pointList);
    fclose(tmp);
}

void test_imagePointCloud() {
    // Create a test List with some point cloud data
    List list;
    if (!listInit(&list, sizeof(pcd_t))) {
        printf("Test failed: Could not initialize list\n");
        return;
    }

    // Add some test points
    pcd_t points[] = {
        {0.5, 0.5, 0.0, 0.0, NULL, NULL, NULL, NULL},  // Minimum height (black)
        {1.5, 0.5, 5.0, 0.0, NULL, NULL, NULL, NULL},  // Middle height (gray)
        {0.5, 1.5, 10.0, 0.0, NULL, NULL, NULL, NULL}, // Maximum height (white)
        {1.5, 1.5, 7.5, 0.0, NULL, NULL, NULL, NULL}   // 75% height
    };

    for (int i = 0; i < 4; i++) {
        listAddEnd(&list, &points[i]);
    }

    // Create the image
    imagePointCloud(&list, "test_output.gif");

    // Check if file was created
    FILE *f = fopen("test_output.gif", "r");
    if (f) {
        printf("Test passed: Image file was created successfully\n");
        fclose(f);
    } else {
        printf("Test failed: Could not create image file\n");
    }

    // Clean up
    free(list.data);
}

int main() {
    test_allocateArray();
    test_pcd_structure();
    test_list(); 
    test_readPointCloudData();
    test_imagePointCloud();
    return 0;
}