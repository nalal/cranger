//C includes
#include <stdio.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h> 
#include <stdlib.h>
#include <pwd.h>

//C++ includes
#include <filesystem>
#include <thread> 

//ENUM defs
enum win_type {RIGHT, CENTER, LEFT};

//Struct defs
struct window_obj {
	int width;
	int height;
	int init_x;
	int init_y;
	bool borders;
	enum win_type this_win_type;
	WINDOW * win_ptr;
};
