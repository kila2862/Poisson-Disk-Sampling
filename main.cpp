#include <iostream>
#include <glut.h>
#include <math.h>
#include <random>
#include <ctime>

#define R 10 //The R value mentioned in paper
#define K 30 //The K value mentioned in paper
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#ifndef M_PI
#define M_PI 3.14159265358979323846 //Need PI to generate the sample points of different angles 
#endif

const float GRID_WIDTH = floor(R / sqrt(2)); //The width value for the table's grid
const int rows = floor(WINDOW_WIDTH / GRID_WIDTH); //Total rows numbers in the table
const int columns = floor(WINDOW_WIDTH / GRID_WIDTH); //Total columns numbers in the table

using namespace std;

struct PointData {
	float x;
	float y;
	PointData() {
		x = 0.0;
		y = 0.0;
	}
	PointData(float pos_x, float pos_y) {
		x = pos_x;
		y = pos_y;
	}
};

std::vector<int> table; //Public data used to record if the grid in table is visited 
std::vector<PointData> point_positions; //Public data which is used to record the real position of the index in table
std::vector<PointData> active_list; //Public data which is used to record the active point

/////////////////////////////////////////////////////////////////////////////////////
// Function Name: get_transform 
// Purpose: To transform the position in window form to openGL coordinate system
// Return: The transformed position that we need to draw for openGL 
////////////////////////////////////////////////////////////////////////////////////
PointData get_transform(int x, int y) {
	PointData pos;
	pos.x = -1 + (static_cast<float>(x) / WINDOW_WIDTH * 2);
	pos.y = -1 + (static_cast<float>(y) / WINDOW_HEIGHT * 2);
	return pos;
}

///////////////////////////////////////////////////////////////////////////////////
// Functoin Name: display
// Purpose: Used to render the sample points 
// Return: No return value
//////////////////////////////////////////////////////////////////////////////////
void display(void) {
	glPushMatrix();
	glPointSize(5.0);
	glColor3f(1.0f, 0.0f, 0.0f);
	glEnable(GL_POINT_SMOOTH);
	glColor3f(0.0f, 1.0f, 0.0f);
	for (unsigned i = 0; i < table.size(); i++) {
		if (-1 != table[i]) {
			float x = point_positions[i].x;
			float y = point_positions[i].y;
			PointData point = get_transform(x, y);
			glBegin(GL_POINTS);
			glVertex2f(point.x, point.y);
			glEnd();
		}
	}
	glEnd();
	glDisable(GL_POINT_SMOOTH);
	glPopMatrix();
	glutSwapBuffers();
}

///////////////////////////////////////////////////////////////////////////////////////////
// Function Name: table_init
// Purpose: init function for out public member data and generate the first sample point
// Return: no return value
//////////////////////////////////////////////////////////////////////////////////////////
void table_init() {
	//Step 0 in paper: initialize the grid with -1;
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	for (auto i = 0; i < rows*columns; i++) {
		table.push_back(-1);
		point_positions.push_back(PointData());
	}

	//Step 1 in paper: Get the start point and push it in activelist
	std::default_random_engine gen = std::default_random_engine(time(NULL));
	std::uniform_real_distribution<float> dis_width(0, WINDOW_WIDTH);
	std::uniform_real_distribution<float> dis_height(0, WINDOW_HEIGHT);
	int x = dis_width(gen);
	int y = dis_height(gen);
	int index_x = floor(x / GRID_WIDTH);
	int index_y = floor(y / GRID_WIDTH);
	PointData point(x, y);

	table[index_x + index_y * columns] = 1;
	point_positions[index_x + index_y * columns].x = x;
	point_positions[index_x + index_y * columns].y = y;
	active_list.push_back(point);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function Name: check_distance
// Purpose: To check if the distance of the new sample point and its neiborhood is acceptable
// Return: True - if the distance of the new sample point and neiborhood is larger than grid_width
//         False - if the distance of the new sample point and neiborhood is too close
//////////////////////////////////////////////////////////////////////////////////////////////////
bool check_distance(int index, float pos_x, float pos_y) {
	bool accept = true;
	if (1 == table[index]) {
		PointData point = point_positions[index];
		float distance = sqrt(abs(pos_x - point.x)*abs(pos_x - point.x) + abs(pos_y - point.y)*abs(pos_y - point.y));
		if (distance < GRID_WIDTH) {
			accept = false;
		}
	}
	return accept;
}

////////////////////////////////////////////////////////////////////////////////////
// Function Name: check_out_of_table_boundary
// Purpose: Used to check if the given index is out of the range of the table
// Return : Ture - if the index is a valid index in table
//          False - if the index is out of the boundary of the table 
///////////////////////////////////////////////////////////////////////////////////
bool check_out_of_table_boundary(int index) {
	if (index < 0 || index >= table.size()-1) {
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////
// Function Name: update_data
// Purpose: To update the public member data table and active_list
// Return: no return value
///////////////////////////////////////////////////////////////////////////////////
void update_data(int index, float pos_x, float pos_y) {
	table[index] = 1;
	point_positions[index].x = pos_x;
	point_positions[index].y = pos_y;
	PointData new_point(pos_x, pos_y);
	active_list.push_back(new_point);
}

///////////////////////////////////////////////////////////////////////////////////
// Function Name: generate_samples
// Purpose: To generate the sample points for a given point in active_list
//          And finish when there is no points in active_list
// Return: no return value
//////////////////////////////////////////////////////////////////////////////////
void generate_samples() {
	//step 2 in paper: Generate the new sample points near the given poins in active_list
	const unsigned int seed = time(0);
	mt19937_64 rng(seed);
	std::uniform_real_distribution<float> dis_angle(0.0, 2 * M_PI);
	std::uniform_real_distribution<float> dis_distance(floor(R / sqrt(2)), 2 * floor(R / sqrt(2)));

	while (!active_list.empty()) {
		PointData point = active_list.back();
		active_list.pop_back();
		for (auto count = 0; count < K; count++) {
			float angle = dis_angle(rng);
			float distance = dis_distance(rng);
			float pos_x = point.x + distance * cos(angle);
			float pos_y = point.y + distance * sin(angle);
			if (0 > pos_x || pos_x > WINDOW_WIDTH || pos_y < 0 || pos_y > WINDOW_HEIGHT) continue; //skip the points that is out of the window form

			int index_x = floor(pos_x / GRID_WIDTH);
			int index_y = floor(pos_y / GRID_WIDTH);
			int index = index_x + index_y * columns;

			if (!check_out_of_table_boundary(index)) continue; //skip the invalid index of table
			if (1 == table[index]) continue; //skip for the the created grid in table
			bool all_ok = true;
			for (auto offset_x = -1; offset_x <= 1; offset_x++) {
				for (auto offset_y = -1; offset_y <= 1; offset_y++) {
					int new_index = (index_x + offset_x) + ((index_y + offset_y)*columns);
					if (!check_out_of_table_boundary(new_index)) continue; //skip the index that is out of table range
					if (false == check_distance(new_index, pos_x, pos_y)) {
						all_ok = false;
					}
				}
			}

			if (true == all_ok) {
				update_data(index, pos_x, pos_y);
			}
		}// end of the for loop for K times sample generation
	}//end of while loop 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functiona Name: main
// Purpose: The entry point of the program. Not so many thing done. mainly are 
//          (1) Set the window size and its position
//          (2) Call table_init() to initialize the public member data and get the initial sample point
//          (3) Call generate_samples() to generate the sample points from the the initialized sample point
//          (4) Render the sample points we get from generate_samples function
// Retuen: 0 after process ends
////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutInitWindowPosition(100, 100); //set the position of the window
	glutCreateWindow("Poisson Disk Sampling"); //set the title of the window
	table_init();
	generate_samples();
	glutDisplayFunc(display);
	glutMainLoop();
	return 0;
}