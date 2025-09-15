#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define MAX_STRING_LENGTH 34
#define MAX_AIR_ROUTES 5
#define CACHE_SIZE 10000

#define TOP_LEFT 0
#define LEFT 1
#define BOTTOM_LEFT 2
#define BOTTOM_RIGHT 3
#define RIGHT 4
#define TOP_RIGHT 5


//----------------------------------------------------------------------------------------------------------------------
//STRUCTS

typedef struct {
    int xd, yd;
    int air_cost;
} air_route_t;


typedef struct {
    int cost;
    air_route_t* air;
} hexagon_t;


typedef struct {
    int x, y;
    int distance;
} queue_t;


typedef struct {
    int x, y;
    int distance;
} heap_node_t;


typedef struct {
    int col, row;
} axial_coords_t;


typedef struct {
    int xs, ys, xd, yd;
    int distance;
} cache_t;


//----------------------------------------------------------------------------------------------------------------------
//GLOBAL VARIABLES

hexagon_t** map = NULL;
int cols = 0;
int rows = 0;

int** dijkstra_distance_glob = NULL;
bool** visited_glob = NULL;
heap_node_t* heap_glob = NULL;

const axial_coords_t axial_directions[6] = {
    {0, -1 },     //TOP-LEFT
    {-1, 0 },     //LEFT
    {-1, 1 },    //BOTTOM-LEFT
    {0, 1 },     //BOTTOM-RIGHT
    {1, 0 },    //RIGHT
    {1, -1 }     //TOP-RIGHT
};


cache_t cache[CACHE_SIZE];
int cache_occupied_pos = 0;
int cache_index = 0;


//----------------------------------------------------------------------------------------------------------------------
//CORE FUNCTIONS
void init();
void change_cost(int x, int y, int v, int radius);
void toggle_air_route(int xs, int ys, int xd, int yd);
void travel_cost(int xs, int ys, int xd, int yd);

//----------------------------------------------------------------------------------------------------------------------
//MIN-HEAP FUNCTIONS
void insert_node(heap_node_t* heap, int* heap_size, heap_node_t node);
int father();
int left_son();
int right_son();
void swap_node(heap_node_t* node_a, heap_node_t* node_b);
heap_node_t extract_min(heap_node_t* heap, int* heap_size);
void min_heapify(heap_node_t* heap, int heap_size, int index);

//----------------------------------------------------------------------------------------------------------------------
//QUEUE FUNCTIONS
void enqueue(queue_t* queue, int* queue_size, queue_t new_element, int* last, int max_queue_size);
queue_t dequeue(const queue_t* queue, int* queueSize, int* first);

//----------------------------------------------------------------------------------------------------------------------
//CACHE FUNCTIONS
void invalidate_cache();
void add_to_cache(cache_t element);
cache_t find_in_cache(cache_t element);

//----------------------------------------------------------------------------------------------------------------------
//AUXILIARY FUNCTIONS
void free_map();
inline bool are_coordinates_valid(int x, int y);
int air_route_size(const air_route_t* array);
void cost_limit(int* cost);
inline void navigate_map(int move, int* x, int* y);

//----------------------------------------------------------------------------------------------------------------------


int main() {


    //READ OF THE INPUT AND CALL OF THE FUNCTION
    char input[MAX_STRING_LENGTH];
    char command[MAX_STRING_LENGTH];
    int params[4] = { 0 };


    while ( fgets(input, MAX_STRING_LENGTH, stdin) ) {


        sscanf(input, "%s %d %d %d %d", command, &params[0], &params[1], &params[2], &params[3]);


        //THE FUNCTIONS ARE CALLED
        if ( strcmp(command, "init") == 0 ) {
            init(params[0], params[1]);
        }

        else if ( strcmp(command, "change_cost") == 0 ) {
            change_cost(params[0], params[1], params[2], params[3]);
        }

        else if ( strcmp(command, "toggle_air_route") == 0 ) {
            toggle_air_route(params[0], params[1], params[2], params[3]);
        }

        else if ( strcmp(command, "travel_cost") == 0 ) {
            travel_cost(params[0], params[1], params[2], params[3]);
        }
    }

    free_map();

    return 0;
}



//----------------------------------------------------------------------------------------------------------------------
//CORE FUNCTIONS

void init(const int column, const int row) {


    //check if the parameters are valid
    if ( column <= 0 || row <= 0 ) {
        printf("KO\n");
        return;
    }


    //the hash_table is no longer valid
    invalidate_cache();


    //free the map from any use made before
    free_map();


    //initialize the matrix and update the global variables
    map = malloc( row * sizeof(hexagon_t*));
    dijkstra_distance_glob = malloc( row * sizeof(int*));
    visited_glob = malloc( row * sizeof(bool*));
    heap_glob = malloc( row * column * sizeof(heap_node_t));

    for ( int i = 0; i < row; i++) {
        map[i] = malloc( column * sizeof(hexagon_t));
        dijkstra_distance_glob[i] = malloc( column * sizeof(int));
        visited_glob[i] = malloc( column * sizeof(bool));
    }
    rows = row;
    cols = column;


    //initialize the cost of each hexagon of the map matrix
    for ( int r = 0; r < row; r++) {
        for ( int c = 0; c < column; c++) {
            map[r][c].cost = 1;
            map[r][c].air = NULL;
        }
    }


    printf("OK\n");
}



void change_cost(const int x, const int y, const int v, const int radius) {


    //check if conditions are valid
    if ( !are_coordinates_valid(x, y) || ( v < -10 || v > 10 ) || radius <= 0 || map == NULL || visited_glob == NULL ) {
        printf("KO\n");
        return;
    }


    //the hash table is no longer valid
    invalidate_cache();


    //initialize the queue and the visited matrix
    int max_queue_size = 1;
    for ( int i = 1; i < radius; i++) {
        max_queue_size += 6*i;
    }

    queue_t queue[max_queue_size];
    int first = 0;
    int last = 0;
    int queue_size = 0;


    for (int r = 0; r < rows; r++) {
        memset(visited_glob[r], 0, cols * sizeof(bool));
    }


    //insert the first element in the queue;
    queue_t elem = (queue_t) { x, y, 0 };
    enqueue(queue, &queue_size, elem, &last, max_queue_size);
    visited_glob[y][x] = true;


    //visit all the nodes in the queue until there aren't anymore to visit
    while ( queue_size > 0 ) {

        //dequeue the first element of the queue
        const queue_t curr_elem = dequeue(queue, &queue_size, &first);


        //calculate the delta cost
        const int delta = (int) floorf( (float) v * ( (float) (radius - curr_elem.distance) / (float) radius) );


        //update the exit cost and the air_costs
        map[curr_elem.y][curr_elem.x].cost += delta;
        cost_limit(&map[curr_elem.y][curr_elem.x].cost);

        //update air routes
        const int air_size = air_route_size(map[curr_elem.y][curr_elem.x].air);
        for ( int i = 0; i < air_size; i++ ) {
            map[curr_elem.y][curr_elem.x].air[i].air_cost += delta;
            cost_limit(&map[curr_elem.y][curr_elem.x].air[i].air_cost);
        }


        //add the neighbors to the queue
        for (int direction = TOP_LEFT; direction <= TOP_RIGHT; direction++) {

            //reset the coordinates and find their neighbor in a given direction
            int neighbor_x = curr_elem.x;
            int neighbor_y = curr_elem.y;
            navigate_map(direction, &neighbor_x, &neighbor_y);

            int neighbor_dist = curr_elem.distance + 1;


            //check if the neighbor has valid attributes: (coordinates are valid, the distance is right,
            //and it wasn't visited before)
            if (are_coordinates_valid(neighbor_x, neighbor_y) && neighbor_dist < radius
                && visited_glob[neighbor_y][neighbor_x] == false ) {


                //enqueue the new element in the queue
                queue_t neighbor_elem = (queue_t) { neighbor_x, neighbor_y, neighbor_dist };
                enqueue(queue, &queue_size, neighbor_elem, &last, max_queue_size);


                //mark the hexagon as visited
                visited_glob[neighbor_y][neighbor_x] = true;
            }
        }
    }

    printf("OK\n");
}



void toggle_air_route(const int xs, const int ys, const int xd, const int yd) {


    //check if given coordinates are valid
    if ( !are_coordinates_valid(xs, ys) || !are_coordinates_valid(xd, yd) || (xs == xd && ys == yd)
        || map == NULL ) {
        printf("KO\n");
        return;
    }


    //the hash table is no longer valid
    invalidate_cache();


    //create a pointer to the single air route array (easier to read later)
    air_route_t** routes = &map[ys][xs].air;


    //find the array size
    int size = air_route_size(*routes);


    //if there is at least one air route (!= NULL)
    if ( *routes != NULL ) {

        //if source and destination exist in air_routes delete it
        for ( int i = 0; i < size; i++) {

            if ( xd == (*routes)[i].xd && yd == (*routes)[i].yd ) {

                //delete the element in position I and shift the array to the left
                for ( int j = i; j < size - 1; j++ ) {
                    (*routes)[j] = (*routes)[j + 1];
                }

                (*routes)[size - 1].air_cost = -1;
                (*routes)[size - 1].xd = -1;
                (*routes)[size - 1].yd = -1;
                size--;


                //if the route has size 0 deallocate it
                if ( size == 0 ) {
                    free(*routes);
                    *routes = NULL;
                }

                printf("OK\n");

                return;
            }
        }
    }


    //Check if there is enough space to insert a new element
    if ( *routes != NULL && size == 5 ) {
        printf("KO\n");
        return;
    }


    //Create a new route
    //if the route never existed, allocate a new vector
    if ( *routes == NULL ) {
        *routes = malloc(5 * sizeof(air_route_t));

        //initialize the vector
        for ( int i = 0; i < 5; i++) {
            (*routes)[i].air_cost = -1;
            (*routes)[i].xd = -1;
            (*routes)[i].yd = -1;
        }
        size = 0;
    }


    //Find the new air route cost
    int sum = 0;
    for ( int i = 0; i < size; i++) {
        sum += (*routes)[i].air_cost;
    }

    int new_cost = (sum + map[ys][xs].cost) / (size + 1);

    //add the new air route
    (*routes)[size].air_cost = new_cost;
    (*routes)[size].xd = xd;
    (*routes)[size].yd = yd;

    printf("OK\n");
}



void travel_cost(const int xs, const int ys, const int xd, const int yd) {


    //check if coordinates are correct
    if ( !are_coordinates_valid(xs, ys) || !are_coordinates_valid(xd, yd)
        || map == NULL || dijkstra_distance_glob == NULL || visited_glob == NULL) {
        printf("-1\n");
        return;
    }


    //try to find the element in the hash table, if it's in the hash table print the distance and return
    cache_t cache_elem = find_in_cache((cache_t) {xs, ys, xd, yd, -1} );
    if ( cache_elem.distance != -1 ) {
        printf("%d\n", cache_elem.distance);
        return;
    }


    //initialize the visited map matrix and the distance from source matrix
    for ( int r = 0; r < rows; r++ ) {
        memset(visited_glob[r], 0, cols * sizeof(bool));
        memset(dijkstra_distance_glob[r], 0x7F, cols * sizeof(int));
    }


    //initialize the min-heap and insert the source node
    int heap_size = 0;

    insert_node(heap_glob, &heap_size, (heap_node_t) {xs, ys, 0});
    dijkstra_distance_glob[ys][xs] = 0;


    //the loop continues to run until the min-heap runs out of nodes or finds a path
    while ( heap_size > 0 ) {


        //extract the node with the smallest distance from source
        heap_node_t curr_node = extract_min(heap_glob, &heap_size);


        //if the current node reaches the destination it returns the distance
        if ( curr_node.x == xd && curr_node.y == yd ) {


            //save the just found route in the hash table
            add_to_cache((cache_t) {xs, ys, xd, yd, curr_node.distance});


            printf("%d\n", curr_node.distance);
            return;
        }


        //check if the node was already visited, if so skip this iteration
        if ( visited_glob[curr_node.y][curr_node.x] == true ) {
            continue;
        }

        //mark the current node as visited
        visited_glob[curr_node.y][curr_node.x] = true;


        //if the exit cost of the hexagon is zero and the destination wasn't reached, the function returns -1
        if ( map[curr_node.y][curr_node.x].cost == 0 ) {
            continue;
        }


        //explore the neighbors hexagons
        for ( int move = TOP_LEFT; move <= TOP_RIGHT; move++ ) {


            //find the neighbor's coordinates
            int neighbor_x = curr_node.x;
            int neighbor_y = curr_node.y;

            navigate_map(move, &neighbor_x, &neighbor_y);


            //if the coordinates are valid, find the neighbor's distance form the source
            if ( are_coordinates_valid(neighbor_x, neighbor_y) ) {


                //find the new distance from the source
                const int new_distance = curr_node.distance + map[curr_node.y][curr_node.x].cost;


                //if the new distance is better than before save the value in the heap and update the distance matrix
                if ( new_distance < dijkstra_distance_glob[neighbor_y][neighbor_x] ) {

                    dijkstra_distance_glob[neighbor_y][neighbor_x] = new_distance;

                    const heap_node_t new_node = {neighbor_x, neighbor_y, new_distance};
                    insert_node(heap_glob, &heap_size, new_node );
                }

            }
        }


        //explore the air routes
        const int air_size = air_route_size(map[curr_node.y][curr_node.x].air);
        for ( int route = 0; route < air_size; route++ ) {


            //find the air neighbor's coordinates and cost
            const int neighbor_x = map[curr_node.y][curr_node.x].air[route].xd;
            const int neighbor_y = map[curr_node.y][curr_node.x].air[route].yd;
            const int air_cost = map[curr_node.y][curr_node.x].air[route].air_cost;


            //find the new distance from the source
            if ( air_cost > 0 ) {

                //find the new distance form the source
                const int new_distance = curr_node.distance + air_cost;


                //if the new distance is better than before save the value in the heap and update the distance matrix
                if ( new_distance < dijkstra_distance_glob[neighbor_y][neighbor_x] ) {

                    dijkstra_distance_glob[neighbor_y][neighbor_x] = new_distance;

                    const heap_node_t new_node = {neighbor_x, neighbor_y, new_distance};
                    insert_node(heap_glob, &heap_size, new_node);
                }
            }
        }


    }

    printf("-1\n");
}



//----------------------------------------------------------------------------------------------------------------------
//MIN-HEAP FUNCTIONS



void insert_node(heap_node_t* heap, int* heap_size, const heap_node_t node) {


    //insert the node at the end of the min-heap
    int current = *heap_size;
    heap[current] = node;
    *heap_size += 1;


    //this loop sorts the min-heap and places the node in the right place
    while (current > 0 && heap[current].distance < heap[father(current)].distance) {
        swap_node(&heap[current], &heap[father(current)]);
        current = father(current);
    }

}



int father(const int index) {
    //return the father of the node in the array
    return (index - 1) / 2;
}



int left_son(const int index) {
    //return the left son of the node in the array
    return (index * 2) + 1;
}



int right_son(const int index) {
    //return the right-son of the node in the array
    return (index * 2) + 2;
}



void swap_node(heap_node_t* node_a, heap_node_t* node_b) {
    //this function swaps the elements in the min-heap
    heap_node_t temp = *node_a;
    *node_a = *node_b;
    *node_b = temp;
}



heap_node_t extract_min(heap_node_t* heap, int* heap_size) {


    //if the min-heap size equal to one, delete it from the structure and return it
    if ( *heap_size == 1 ) {
        *heap_size -= 1;
        return heap[0];
    }


    //if the min-heap size is greater than one, save the smallest element, sort the min-heap, finally return the minimum
    heap_node_t min = heap[0];

    heap[0] = heap[*heap_size - 1];
    *heap_size -= 1;

    min_heapify(heap, *heap_size, 0);

    return min;
}



void min_heapify(heap_node_t* heap, int heap_size, int index) {
    //This function brings down the element in position 'index' until it's in the right position

    int left, right, smallest_index;

    while ( true ) {
        left = left_son(index);
        right = right_son(index);
        smallest_index = index;


        if ( left < heap_size && heap[left].distance < heap[smallest_index].distance ) {
            smallest_index = left;
        }


        if ( right < heap_size && heap[right].distance < heap[smallest_index].distance ) {
            smallest_index = right;
        }


        if ( smallest_index == index ) {
            break;
        }

        swap_node(&heap[index], &heap[smallest_index]);
        index = smallest_index;

    }

}



//----------------------------------------------------------------------------------------------------------------------
//QUEUE FUNCTIONS

void enqueue(queue_t* queue, int* queue_size, const queue_t new_element, int* last, const int max_queue_size) {
    queue[*last] = new_element;

    (*last)++;
    if ( *last == max_queue_size ) {
        *last = 0;
    }

    (*queue_size)++;
}



queue_t dequeue(const queue_t* queue, int* queueSize, int* first) {
    //check if there's at least one element in the queue; otherwise, return an error
    if ( *queueSize == 0 ) {
        printf("Empty queue\n");
        printf("KO\n");
        exit(-1);
    }

    //save the first element of the queue and remove it from the queue
    queue_t result = queue[*first];
    (*first)++;
    (*queueSize)--;

    return result;
}



//----------------------------------------------------------------------------------------------------------------------
//CACHE FUNCTIONS
void invalidate_cache() {
    cache_occupied_pos = 0;
}



void add_to_cache(const cache_t element) {

    int index = cache_occupied_pos % CACHE_SIZE;

    cache[index] = element;
    cache_occupied_pos++;
}



cache_t find_in_cache(const cache_t element) {
    for ( int i = 0; i < cache_occupied_pos; i++ ) {
        if ( element.xs == cache[i].xs && element.ys == cache[i].ys
            && element.xd == cache[i].xd && element.yd == cache[i].yd ) {
            return cache[i];
        }
    }

    return element;
}



//----------------------------------------------------------------------------------------------------------------------
//AUXILIARY FUNCTIONS

void free_map() {


    //if the map isn't initialised, do nothing and return
    if ( map == NULL || dijkstra_distance_glob == NULL || visited_glob == NULL || heap_glob == NULL ) {
        return;
    }


    //deallocate the air routes, then every row and finally the whole map, then also deallocate the global variables
    for ( int r = 0; r < rows; r++) {


        for ( int c = 0; c < cols; c++) {


            const int air_size = air_route_size(map[r][c].air);

            if ( air_size != 0 ) {

                free(map[r][c].air);
                map[r][c].air = NULL;

            }
        }

        free(map[r]);
        free(dijkstra_distance_glob[r]);
        free(visited_glob[r]);
    }
    free(map);
    free(dijkstra_distance_glob);
    free(visited_glob);
    free(heap_glob);


    //reset the proprieties of the map
    map = NULL;
    dijkstra_distance_glob = NULL;
    visited_glob = NULL;
    rows = 0;
    cols = 0;
}



inline bool are_coordinates_valid(const int x, const int y) {
    //checks if coordinates are valid
    if ( ( x < 0 || x >= cols ) || ( y < 0 || y >= rows ) ) {
        return false;
    }

    return true;
};



int air_route_size(const air_route_t* array) {

    //this function counts how many open air routes there are
    int size = 0;

    if ( array == NULL ) {
        return size;
    }

    for ( int i = 0; i < MAX_AIR_ROUTES; i++) {
        if ( array[i].air_cost == -1 ) {
            return size;
        }
        size++;
    }

    return size;
}



void cost_limit(int* cost) {
    //this function applies the cost limits 0 <= cost <= 100
    if ( *cost < 0 ) {
        *cost = 0;
    }

    else if ( *cost > 100) {
        *cost = 100;
    }

}



inline void navigate_map(const int move, int* x, int* y) {


    //convert from odd-r coordinates to axial
    axial_coords_t coords;
    coords.col = *x - (*y >> 1);
    coords.row = *y;


    //calculate the neighbor axial coords
    coords.col += axial_directions[move].col;
    coords.row += axial_directions[move].row;


    //convert from axial to odd-r
    *y = coords.row;
    *x = coords.col + (*y >> 1);

}