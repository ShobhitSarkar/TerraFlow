#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pointcloud.h"

void print_usage() {
    printf("Usage: ./watershed <ifile> <iter> <iwater> <wcoef> <ecoef> <ofilebase> [seq]\n");
    printf("  ifile     - Input pointcloud file name\n");
    printf("  iter      - Number of computation steps\n");
    printf("  iwater    - Initial water amount\n");
    printf("  wcoef     - Water flow coefficient (0.0-0.2)\n");
    printf("  ecoef     - Evaporation coefficient (0.9-1.0)\n");
    printf("  ofilebase - Output file base name\n");
    printf("  seq       - Optional: Output interval for intermediate steps\n");
}

int main(int argc, char *argv[]) {
    // Check arguments
    if (argc != 7 && argc != 8) {
        print_usage();
        return 1;
    }

    // Parse arguments
    char *ifile = argv[1];
    int iter = atoi(argv[2]);
    double iwater = atof(argv[3]);
    double wcoef = atof(argv[4]);
    double ecoef = atof(argv[5]);
    char *ofilebase = argv[6];
    int seq = (argc == 8) ? atoi(argv[7]) : 0;

    // Validate parameters
    if (iter <= 0 || iwater < 0 || 
        wcoef < 0.0 || wcoef > 0.2 || 
        ecoef < 0.9 || ecoef > 1.0) {
        printf("Error: Invalid parameters\n");
        print_usage();
        return 1;
    }

    // Read input file
    FILE *f = fopen(ifile, "r");
    if (!f) {
        printf("Error: Cannot open input file %s\n", ifile);
        return 1;
    }

    pointcloud_t *pc = readPointCloudData(f);
    fclose(f);

    if (!pc) {
        printf("Error: Failed to read pointcloud data\n");
        return 1;
    }

    // Initialize watershed
    if (initializeWatershed(pc) != 0) {
        printf("Error: Failed to initialize watershed\n");
        pointcloud_free(pc);
        return 1;
    }

    //update the coefficients
    update_watershed_coefficients(pc, wcoef, ecoef);

    // Add initial water
    watershedAddUniformWater(pc, iwater);

    // Prepare output filename buffer
    char outfile[256];

    // Run simulation steps
    for (int i = 0; i < iter; i++) {
        // Perform watershed step
        watershedStep(pc);

        // Generate output if needed
        if (seq > 0 && (i % seq == 0 || i == iter - 1)) {
            // Create filename with step number
            snprintf(outfile, sizeof(outfile), "%s%d.gif", ofilebase, i);
            imagePointCloudWater(pc, iwater * 2, outfile);
            printf("Generated: %s\n", outfile);
        }
    }

    // Generate final output if seq was not specified
    if (seq == 0) {
        snprintf(outfile, sizeof(outfile), "%s.gif", ofilebase);
        imagePointCloudWater(pc, iwater * 2, outfile);
        printf("Generated final output: %s\n", outfile);
    }

    pointcloud_free(pc);
    return 0;
}