#define MAX_POINTS 20
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "data_types.c"
#include "file_io.c"
#include "lib.c"

//#include "tri_area.c"

int main(int argc, char* argv[]) {

	//check to see if the right number of arguements were passed in
	if(argc != 2) {
		fprintf(stderr, "Not enough args... now exiting\n");
		return 1;
	}

	//open file and ensure it opened properly
	FILE *f = fopen(argv[1], "r");
	if(f == NULL) {
		fprintf(stderr, "File failed to open... now exiting\n");
		return 1;
	}

	//determine how much space is needed and allocate it
	int line_count = count_lines(f);
	point *points = calloc(sizeof(point), line_count);

	//error checking
	if(line_count < 3) {
		fprintf(stderr, "Improperly formatted input file or not enough portals (need at least three)... now exiting\n");
		return 1;
	}

	if(points == NULL) {
		fprintf(stderr, "Failed to allocate... now exiting\n");
		return 1;
	}

	//read the data from the file into points
	read_file(f, points, line_count);

	//find a anchor point (the purpose of which is explained in find_anchor_point and documentation)
	list_controller *list = find_anchor_point(points, line_count);

	//make and print out the links needed to create the most possible fields using the given set of points
	make_links(list, points, line_count);	


	return 0;
}
