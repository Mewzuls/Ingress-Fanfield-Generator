#include <stdio.h>


int count_lines(FILE* f) {
	int line_count = 0;
	char s1[100];
	double s2, s3;

	while(fscanf(f, "%s %lf %lf", s1, &s2, &s3) != EOF) {
		line_count++;
	}

	fseek(f, 0L, SEEK_SET);

	return line_count;
}



void read_file(FILE* f, point* points, int line_count) {
	int i = 0;

	while((fscanf(f, "%s %lf %lf", points[i].name, &(points[i].x), &(points[i].y)) != EOF) && i < line_count) {
		i++;
	}
}



