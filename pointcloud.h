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
    double water;    // amount of water at this location
    struct pcd_t *north;  // pointer to the north neighbor
    struct pcd_t *south;  // pointer to the south neighbor
    struct pcd_t *east;   // pointer to the east neighbor
    struct pcd_t *west;   // pointer to the west neighbor
} pcd_t;

void stat1();
List* readPointCloudData(FILE *stream);
void imagePointCloud(List *l, char *filename); 

#endif // POINTCLOUD_H