#ifndef POINTCLOUD_H
#define POINTCLOUD_H

#include <stdio.h>
#include "util.h"
#include "bmp.h"

// Structure to store points from the point cloud
typedef struct pcd_t {
    double x;        // x coordinate
    double y;        // y coordinate
    double z;        // z coordinate (height)
    double wd;    // amount of water at this location
    struct pcd_t *north;  // pointer to the north neighbor
    struct pcd_t *south;  // pointer to the south neighbor
    struct pcd_t *east;   // pointer to the east neighbor
    struct pcd_t *west;   // pointer to the west neighbor
} pcd_t;

// Struct to store the statistics for points 
typedef struct{
    double min_height; 
    double max_height; 
    double avg_height; 
    double min_x; 
    double max_x; 
    double min_y; 
    double max_y; 
} pointcloud_stats_t; 

typedef struct {
    List points; // List of pcd_t points
    int rows; // number of rows in the pointcloud
    int cols; // number of columns in the pointcloud
    pointcloud_stats_t stats; // statistics about the pointcloud 
    double water_coef; //water flow coefficient  
    double evap_coef; //evaporation coefficient 
} pointcloud_t; 

// essential functions according to project doc
void stat1();
pointcloud_t* readPointCloudData(FILE *stream);
void imagePointCloud(pointcloud_t *pc, char *filename);
int initializeWatershed(pointcloud_t *pc); 
void watershedAddUniformWater(pointcloud_t *pc, double amount); 
void watershedStep(pointcloud_t *pc); 
void imagePointCloudWater(pointcloud_t *pc, double maxwd, char *filename); 

// helper functions 
void pointcloud_free(pointcloud_t *pc); 
void pointcloud_print_stats(const pointcloud_t *pc); 
pcd_t* pointcloud_get_point(pointcloud_t *pc, int row, int col); 
void update_watershed_coefficients(pointcloud_t *pc, double wcoef, double ecoef);

#endif // POINTCLOUD_H