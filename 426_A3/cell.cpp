#include "cell.h"
#include <iostream>
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include <random>
#include <math.h>

constexpr auto PI = 3.1415926;;

cell::cell() {
	this->celltype = 0;
	this->direction = -1;
	this->r = 0.0f;
	this->g = 0.0f;
	this->b = 0.0f;
}

cell::cell(int celltype){
	this->celltype = celltype;
	this->direction = -1;
	this->r = 0.0f;
	this->g = 0.0f;
	this->b = 0.0f;
	
	if (this->celltype == 2) {
		this->r = 0.0f;
		this->g = 1.0f;
		this->b = 0.0f;
	}
	else if (this->celltype == 3){
		this->r = 1.0f;
		this->g = 0.0f;
		this->b = 0.0f;
	}
	
}

/*
* cell type is defined by the color. 
* 0->healthy -> green
* 1->cancer -> red
* 2->medecine -> yellow
* else -> null -> black
*/
void cell::setcelltype(int celltype,int direction){
	this->direction = direction;
	std::random_device rd;
	std::uniform_int_distribution<int> dist(1, 8);
	this->celltype = celltype;
	if (this->celltype == 2) {
		this->r = 0.0f;
		this->g = 1.0f;
		this->b = 0.0f;
	}
	else if (this->celltype == 3) {
		this->r = 1.0f;
		this->g = 0.0f;
		this->b = 0.0f;
	}
	else if (this->celltype == 4) {
		this->r = 1.0f;
		this->g = 1.0f;
		this->b = 0.0f;
	}
	else {
		this->r = 0.0f;
		this->g = 0.0f;
		this->b = 0.0f;
	}
}

int cell::getcelltype()
{
	return this->celltype;
}

/*
* Swap position between 2 cells
*/
void cell :: swap(cell celllist_input[][768], cell celllist_output[][768],int i, int j, int i2, int j2) {
	celllist_output[i][j].setcelltype(celllist_input[i2][j2].getcelltype(), celllist_input[i2][j2].getdirection());
	celllist_output[i2][j2].setcelltype(celllist_input[i][j].getcelltype(), celllist_input[i][j].getdirection());
}

int cell::getdirection() {
	return this->direction;
}

/*
* Move function for med cell only. Based on the direction, the cell moves 1 cell each time.
*/
void cell::move(cell celllist_input[][768], cell celllist_output[][768], int i, int j, int width, int height){
	int tmp;
	std::random_device rd;
	std::uniform_int_distribution<int> dist(1, 8);
	if (this->direction == 1 && (i - 1 >= 0 && j + 1 < height)) {
		swap(celllist_input,celllist_output, i, j, i - 1, j + 1);
	}
	else if (this->direction == 2 && (j + 1 < height)) {
		swap(celllist_input, celllist_output, i, j, i, j + 1);
	}
	else if (this->direction == 3 && (i+1< width && j + 1 < height)) {
		swap(celllist_input, celllist_output, i, j, i+1, j+1);
	}
	else if (this->direction == 4 && (i - 1 >= 0)) {
		swap(celllist_input, celllist_output, i, j, i-1, j);
	}
	else if (this->direction == 5 && (i+1 < width)) {
		swap(celllist_input, celllist_output, i, j, i+1, j);
	}
	else if (this->direction == 6 && (i-1>0 && j - 1 >= 0)) {
		swap(celllist_input, celllist_output, i, j, i-1, j - 1);
	}
	else if (this->direction == 7 && (j - 1 >= 0)) {
		swap(celllist_input, celllist_output, i, j, i, j - 1);
	}
	else if (this->direction == 8 && (i+1< width && j - 1 >= 0)) {
		swap(celllist_input, celllist_output, i, j, i+1, j - 1);
	}
	else
		celllist_output[i][j].direction = dist(rd);
}

/*
* The direction is defined by the angle of med cell - injection point vector.
* There are 8 direction going outwards and depends on the angle we chose the direction.
*/
void cell::setdirection(float injectx, float injecty,int x,int y){
	int tmparray[3];
	double angle = tan((y-injecty)/(x-injectx));
	double pi_direction[] = { 3 * PI / 4,PI / 2,PI / 4,PI,0,5 * PI / 4,3 * PI / 2,7 * PI / 4 };
	double smallest = 0,tmp;
	int index=0;
	for (int i = 0; i < 8; i++) {
		 tmp=abs(angle - pi_direction[i]);
		 if (tmp < smallest) {
			 smallest = tmp;
			 index = i;
		 }
	}
	this->direction = index + 1;

}
