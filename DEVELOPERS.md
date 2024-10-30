# Developer Documentation 

## Project Structure: 

### Core Components
1. Point Cloud Processing(`pointcloud.c`, `pointcloud.h`): 
    - Core functionality for processing the point cloud representation 
    - Functions for reading data and data manipulation
    - Functions for visualizing the data

2. Utility Functions (`util.c`, `util.h`): 
    - Implementation of the dynamic array 
    - Memory management functions 
    - Helper macros 

3. Watershed Simulation(`watershed.c`): 
    - Main simulation logic 
    - Parameter processing 
    - Output generation functions

### Data Structures

### PointCloud(`pointcloud_t`): 

```c
typedef struct {
    List points; // array of points
    int rows;  // number of rows 
    int cols;  // number of cols
    pointcloud_stats_t stats; // stats related to the point
} pointcloud_t;
```

### Point Cloud Data (pcd_t)
```c
typedef struct pcd_t {
    double x, y, z;          // Coordinates
    double wd;               // Water depth
    struct pcd_t *north;     // Neighbor pointers
    struct pcd_t *south;
    struct pcd_t *east;
    struct pcd_t *west;
} pcd_t;
```

### Dynamic List Structure : 

```c
typedef struct {
    int max_size; // Maximum size of the list
    int max_element_size; // Size of each element
    void* data; // Pointer to data array
    int size; // Current number of elements
} List;
```

## Buuilding and Testing: 

### Build Process: 

The project has a makefile with the following target: 
- `make all`: makes all the executables 
- `make clean`: removes all the build executables 
- `make test`: builds and run the tests 

## Function Documentation: 

### Array Management: 

```c
void *allocateArray(int rows, int columns)
```
**Purpose:** Allocates a 2D array with the specified dimensions and initializes all elements to 0

**Parameters:**
- rows: Number of rows in the array
- columns: Number of columns in the array

**Returns:** Pointer to allocated array, or NULL if allocation fails

### List Management: 
```c
int listInit(List* l, int max_elmt_size)
```
**Purpose:** Initializes a dynamic list struct

**Parameters:**
- l: Pointer to List structure
- max_elmt_size: Size of each element in bytes


**Returns:** 1 on success, 0 on failure


**Note:** Initial capacity is 10 elements
___ 

```c
void listAddEnd(List* l, void* elmt)
```

**Purpose:** Adds an element to the end of the list

**Parameters:**
- l: Pointer to List structure
- elmt: Pointer to element to add

**Notes:** Doubles capacity when full
___ 
```c
void *listGet(List* l, int index)
```

**Purpose:** Retrieves element at specified index

**Parameters:**
- l: Pointer to List structure
- index: Index of desired element

**Returns:** Pointer to element, or NULL if index invalid
___ 
### Point Cloud Processing: 
```c
List* readPointCloudData(FILE *stream)
```
**Purpose:** Reads point cloud data from input stream

**Parameters:**
- stream: Input file stream

**Returns:** List containing point cloud data

**Note:** First line must contain number of columns
___


```c
void imagePointCloud(List *l, char *filename)
```
**Purpose:** Creates grayscale visualization of terrain

**Parameters:**
- l: List containing point cloud data
- filename: Output filename for GIF

**Note:** Maps height values to grayscale intensities

## Build System:
```
CFLAGS = -Wall -Werror
```
- Enables all warnings 
- Treats warnings as errors 



