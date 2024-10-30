#include <stdio.h>
#include <stdlib.h>
#include "pointcloud.h"

int main() {
    // Read point cloud data from stdin
    List* pointList = readPointCloudData(stdin);
    if (pointList == NULL) {
        printf("Error: Failed to read point cloud data\n");
        return 1;
    }

    // Create the image
    imagePointCloud(pointList, "out.gif");

    // Clean up
    free(pointList->data);
    free(pointList);

    return 0;
}