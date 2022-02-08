#pragma once
#include <iostream>
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

class cell
{
private:
	int celltype;
	int direction;
	GLfloat r, g, b;
public:
	cell(int);
	cell();
	void setcelltype(int,int);
	int getcelltype();
	void swap(cell celllist_input[][768], cell celllist_output[][768], int i, int j, int i2, int j2);
	void move(cell celllist_input[][768], cell celllist_output[][768], int,int,int,int);
	void setdirection(float injectx, float injecty, int x, int y);
	int getdirection();
	GLfloat getr() { return this->r; }
	GLfloat getg() { return this->g; }
	GLfloat getb() { return this->b; }
};

