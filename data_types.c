#ifndef DATA_TYPES
#define DATA_TYPES
#define MAX_NAME_LENGTH 100

typedef struct {
	char name[MAX_NAME_LENGTH];
	double x, y;
} point;

typedef struct point_list {
	point p; 
	struct point_list* next;
} point_list;


typedef struct {
	int indices[3];
} triangle;

typedef struct angle_list {
	int point;
	double angle;
	struct angle_list* next;
} angle_list;

typedef struct link_list {
	int ends[2];
	struct link_list *next;
} link_list;

typedef struct {
	angle_list *list;
	link_list *links;
	int anchor;
} list_controller;



#endif
