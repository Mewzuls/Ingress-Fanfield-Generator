#include <stdlib.h>
#include <math.h>
#include "data_types.c"

#define FP_ERROR .000000001

angle_list* add_list(angle_list *head, angle_list *new, point* points) {
/*

	function to add a point to the list of points (list is ordered by angle)

*/
	angle_list *curr = head, *prev = head;
	
	//advance forward until the angle in the list is greater than the angle of the new point to be added
	while((curr->angle > new->angle) && (curr->next != NULL)) {
		prev = curr;
		curr = curr->next;
	}

	//curr == prev only if the body of the above loop was never run
	 if(curr == prev) {
		//put the new element at the head of the list
		if(curr->angle <= new->angle) {
			new->next = curr;
			head = new;
		}
		else {
			//put the new element at the end of the list
			if(curr->next == NULL) {
				curr->next = new;
			}
			else {
				//code here should never run
				fprintf(stderr, "ERROR IN add_list: IN BAD SECTOR OF CODE #1");
				return NULL;
			}
		}
	}
	else {
		//if the new element should go between prev and curr
		if(curr->angle < new->angle) {
			new->next=prev->next;
			prev->next = new;
		}
		else {
			//if the new element should go at the end of the list
			if(curr->next == NULL) {
				curr->next = new;
			}
			else {
				//code here should never run
				fprintf(stderr, "ERROR IN add_list: IN BAD SECTOR OF CODE #2");
				fprintf(stderr, "\n%lf %d\t%lf %d\n", curr->angle, curr->point, new->angle, new->point);
				return NULL;
			}
		}
	}

	//in case new was placed at the beginning of the list, the calling environment need the new head
	return head;
}



void print_angle_list(angle_list *head, point *points) {
/*

	function to go through a list of points and print out the readable names of the points; not fully documented as it is unused

*/
	angle_list *temp = head;
	while(temp->next != NULL) {
		printf("%s\n", points[temp->point].name);
		temp = temp->next;
	}

	printf("%s\n", points[temp->point].name);
}


void print_list(list_controller *list, point *points) {
/*

	function to go through a list of points and print out the readable names of the points; not fully documented as it is unused

*/
	angle_list *temp = list->list;
	printf("anchor is %s\n", points[list->anchor].name);
		printf("Link from %s to %s\n", points[list->anchor].name, points[temp->point].name);
	while(temp->next != NULL) {
		printf("Link from %s to %s\n", points[list->anchor].name, points[temp->next->point].name);
		temp = temp->next;
	}
}


angle_list* is_anchor_point(point* points, int portal_count, int test_portal) {
/*

	function which finds an anchor point (defined below) for a given set of points; also assmbles a list of all the other points in counter-clockwise order

	for the purposes here, an anchorpoint is one for which all points exist on one side
	alternative definition: a point is an anchor point if two rays with an angle less than pi (180 degrees) between them can be made such that all other points in the data set are in between the two rays

	assuming non colinearity (all points in one line), there should be a minimum of three possible anchor points (colinearity would allow down to two)

*/
	double angle, initial_angle, min_angle, max_angle;
	int initial_point;
	angle_list *head, *tail;

	//allocate space for list and initialize pointers
	head = calloc(sizeof(angle_list), 1);
	tail = head;

	//compare the test point to an initial point != the test point
	if(test_portal != 0) {
		angle = atan2(points[0].y-points[test_portal].y, points[0].x-points[test_portal].x);
		head->point = 0;
	}
	else {
		angle = atan2(points[1].y-points[test_portal].y, points[1].x-points[test_portal].x);
		head->point = 1;
	}

	//whether point 1 or 0 was used, store the angle as the current largest and smallest, as well as in the head node list
	min_angle = angle;
	max_angle = angle;
	head->angle = angle;


	for(int i = 1; i < portal_count; i++) {
		//don't compare test_point to itself, and if i=1 and test_portal is 0, don't do it again
		if((i == test_portal) || (i==1 && test_portal == 0)) {
			continue;
		}

		//find angle between point i and the point that's being tested
		angle = atan2(points[i].y-points[test_portal].y, points[i].x-points[test_portal].x);

		//allocate and set up the data in a new node for the list
		angle_list *temp = calloc(sizeof(angle_list), 1);
		temp->point = i;
		temp->angle = angle;

		//add the new node to the list
		head = add_list(head, temp, points);

		//check if there's a new min or max angle
		if(angle < min_angle) {
			min_angle = angle;
		}
		if(angle > max_angle) {
			max_angle = angle;
		}
	}

 
	double arc = max_angle - min_angle;

	//may not be necessary, but left in to ensure working order until testing can be completed; in case highly negative min_angle and highly positive max_angle go over a full circle through rounding errors
	if(arc > M_PI*2) {
		arc -= M_PI*2;
	}

	//check to see if arc is less than pi
	if(arc > M_PI - FP_ERROR) {
		return NULL;
	}

	return head;
}


list_controller* find_anchor_point(point* points, int portal_count) {
/*

	function which finds an anchor point (defined below) for a given set of points

	for the purposes here, an anchor point is one for which all points exist on one side
	alternative definition: a point is an anchor point if two rays with an angle less than pi (180 degrees) between them can be made such that all other points in the data set are in between the two rays

	assuming non colinearity (all points in one line), there should be a minimum of three (colinearity would allow down to two)
	
*/

	//look for anchor points until one is found
	for(int i = 0; i < portal_count; i++) {
		list_controller *list = calloc(sizeof(list_controller), 1);
		list->list = is_anchor_point(points, portal_count, i);

		//if an anchor point has been found
		if(list->list != NULL) {
			list->anchor = i;
			return list;
		}
	}


	//should never run
	return NULL;
}

int is_blocking(int link1, int link2, int source_point, int dest_point, point *points) {

/*
	function to determine if two line segments cross (block), originally for use in an Ingress fielding optimizer.  

	all this does is check to see if the solution to the crossing of the two lines is whithin the right range as follows
		y1(x) = m1*x+b1
		y2(x) = m2*x+b2

	setting these equal and solving for x yields:
		x = (b2-b1)/(m1-m2)

	if this x value is within the possible range of x values for both the line segments, then the two segments cross
*/

	double x_min_1, x_max_1, x_min_2, x_max_2;

//find the min and max x value of each of the lines

//	if(source_point == 3 && dest_point == 4) {
//		printf("\n%s  %d\t%s  %d\n\n", points[link1].name, link1, points[link2].name, link2);
//	}

	//line one (link being tested)
	if(points[source_point].x < points[dest_point].x) {
		x_min_1 = points[source_point].x;
		x_max_1 = points[dest_point].x;
	}
	else {
		x_min_1 = points[dest_point].x;
		x_max_1 = points[source_point].x;	
	}

	//line two (link already made)
	if(points[link1].x < points[link2].x) {
		x_min_2 = points[link1].x;
		x_max_2 = points[link2].x;
	}
	else {
		x_min_2 = points[link2].x;
		x_max_2 = points[link1].x;	
	}

	//if the two don't even have the same rage of x values, then they don't cross
	if(x_max_1 < x_min_2 || x_max_1 < x_min_2) {
		return 0;
	}

	//two links sharing a common anchor can't block unless both points are the same, in which case the link is redundant
	if(source_point == link1 || source_point == link2 || dest_point == link1 || dest_point == link2) {
		return 0;
	}

	//Find the slopes of the lines
	double m1 = (points[source_point].y-points[dest_point].y)/(points[source_point].x-points[dest_point].x);
	double m2 = (points[link1].y-points[link2].y)/(points[link1].x-points[link2].x);

	//parallel links can't block each other unless they're the same, which is assumed not to be the case
	if((m1-m2) == 0) {
		return 0;
	}

	//find the y-intercepts
	double b1 = points[source_point].y-points[source_point].x*m1;
	double b2 = points[link1].y-points[link1].x*m2;

	//find the x-coordinate of the point where the two lines cross
	double x = (b2-b1)/(m1-m2);

	//if the found x-coordinate is on the two line segments, return indicating that it blocks
//	if((x < x_max_1-0.0000001) && (x < x_max_2-0.0000001) && (x > x_min_1+0.0000001) && (x > x_min_2+0.0000001)) {
	if(x < x_max_1 && x < x_max_2 && x > x_min_1 && x > x_min_2) {
		return 1;
	}

	//getting this far means the two segments don't block each other
	return 0;
}

void add_link(link_list *links, link_list *new_link) {
	while(links->next != NULL) {
		links = links->next;
	}

	links->next = new_link;
}

void make_back_links(angle_list *start, angle_list *source, point *points, link_list *links) {
/*
	creates backwards links as part of the Ingress fanfield field maximisation algorithm (found at https://www.youtube.com/watch?v=C97xCtswH2E on May 24, 2018; video by Youtube user Michael Hartley)
*/

	angle_list *point_curr = start;
	link_list *link_curr = links;

	//move forward through the list of points, checking to see if they can be linked to source
	while(point_curr != source) {
		int blocked = 0;		

		//move forward through the links that have already been made, checking to see if they block the prospective link
		while(link_curr->next != NULL) {

			if(is_blocking(link_curr->ends[0], link_curr->ends[1], source->point, point_curr->point, points)) {
				blocked = 1;
				break;
			}

			//step forward to check if the next link is blocking
			link_curr = link_curr->next;
		}

		//if the prospective link was not blocked, allocated space, set end points, and add it to the list
		if(!blocked) {
			link_list *link_temp = calloc(sizeof(link_list), 1);
			link_temp->ends[0] = source->point;
			link_temp->ends[1]= point_curr->point;
			add_link(links, link_temp);
		}

		//step forward to creat links for the next point
		point_curr = point_curr->next;
		link_curr = links;
	}
}


int count_fields(link_list *head) {
/*

	funtion that counts the number of fields formed by a given list of links

*/

	int field_count = 0;
	link_list *link1, *link2, *link3;

	link1 = head;


	//just goes through each link (link1) and checks to see if each following link (link2) could form a field with it.  If so, it looks for a third link (link3) that would complete the field.
	do {
		link2 = link1->next;
		
		while(link2 != NULL) {
			link3 = link2->next;

			int points[3] = {-1, -1, -1};
			if((link1->ends[0] == link2->ends[0])) {
				points[0] = link1->ends[0];
				points[1] = link1->ends[1];
				points[2] = link2->ends[1];
			}
			else {
				if(link1->ends[0] == link2->ends[1]) {
					points[0] = link1->ends[0];
					points[1] = link1->ends[1];
					points[2] = link2->ends[0];
				}	
				else {
					if(link1->ends[1] == link2->ends[0]) {
						points[0] = link1->ends[1];
						points[1] = link1->ends[0];
						points[2] = link2->ends[1];
					}
					else {
						if(link1->ends[1] == link2->ends[1]) {
							points[0] = link1->ends[1];
							points[1] = link1->ends[0];
							points[2] = link2->ends[0];
						}
						else {
							link2 = link2->next;
							continue;
						}
					}
				}
			}

			while(link3 != NULL) {
				if((points[1] == link3->ends[0] && points[2] == link3->ends[1]) || (points[1] == link3->ends[1] && points[2] == link3->ends[0])) {
					field_count++;
				}

				link3 = link3->next;
			}

			link2 = link2->next;
	   	}

		link1 = link1->next;
	} while(link1 != NULL);

	return field_count;
}


void make_links(list_controller *list, point *points, int portal_count) {
/*

	function to make and print a list of links based on a set of points and an anchor point for use in creating the maximum number of fields using a fanfield algorithm in Ingress (an example of the algorithm followed can be found here: https://www.youtube.com/watch?v=C97xCtswH2E as of May 25, 2018; video made by Youtube user Michael Hartley)

*/

	int link_count = 0, field_count = 0;
	link_list *link_head, *link_curr, *link_temp;
	angle_list *point_curr, *point_temp;

	//initialize the values of and allocate space for pointers
	point_curr = list->list;
	list->links = calloc(sizeof(link_list), 1);
	link_head = list->links;
	link_head->ends[0] = list->anchor;
	link_head->ends[1] = point_curr->point;

	//if there are enough points to continue
	if(point_curr->next != NULL) {
		point_curr = point_curr->next;
	}
	else {
		fprintf(stderr, "NOT ENOUGH PORTALS; NOW EXITING\n");
		return;
	}

	//for every point, create a link to the anchor point, then make all possible backlinks
	while(point_curr != NULL) {

		//allocate space and set the endpoints for a link between point_curr and the anchor point, then add it to the list of links 
		link_temp = calloc(sizeof(link_list), 1);
		link_temp->ends[0] = list->anchor;
		link_temp->ends[1] = point_curr->point;
		add_link(link_head, link_temp);

		//make links back to previous points in a clockwise direction
		point_temp = list->list;
		make_back_links(point_temp, point_curr, points, link_head);

		//move on to next point
		point_curr = point_curr->next;

	}

	// print out list of links using readable portal names
	link_temp = link_head;
	while(link_temp != NULL) {
		printf("Link %s and %s\n", points[link_temp->ends[0]].name, points[link_temp->ends[1]].name);
		link_temp = link_temp->next;

		//count the number of links being made
		link_count ++;
	}

	//counts the number of fields
	field_count = count_fields(link_head);	

	//print links/field counts and AP gain to the user
	printf("Number of links: %d, AP from links: %d\n", link_count, link_count * 313);
	printf("Number of fields: %d, AP from fields: %d\n", field_count, field_count * 1250);
	printf("Total ap: %d\n\n", link_count*313 + field_count*1250);
}
