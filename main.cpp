#include "main.h"

/*
NCURSES FUNCTIONS TO REMEMBER BECAUSE I'M SLOW:
	attron(ATTRIBUTE_ENUM); 	//set attributes
	printw("string");			//print to screen
	attroff(ATTRIBUTE_ENUM);	//disables attributes
	mvprintw(Y,X, "string");	//move to and print
	initscr();					//start ncurses
	cbreak();					//catch terminate input
	endwin();					//clear window
*/

namespace fs = std::filesystem;
using std::thread;

//Total windows to be loaded, (this shouldn't be changed)
const int total_windows = 3;
window_obj windows[total_windows];

bool running = true;


//Get terminal size
struct winsize get_win()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w;
}

//Print string to window
//[win] = window pointer to print to
//[input] = string to print
//[start_y] = y coord in window to start at
//[start_x] = x coord in window to start at
void print_to_window(
	WINDOW * win, 
	char * input, 
	int start_y = 1, 
	int start_z = 1
)
{
	mvwprintw(win, 1, 1, "%s", input);
	wrefresh(win);
}

//Create window with given info (can only create up to 3)
//[win_num] = the number of the window to be created
//[win_h] = window height 
//[win_w] = window width
void mk_window(int win_num, int win_h, int win_w)
{	
	WINDOW * win = newwin(win_h, win_w, 1, win_w * (win_num - 1));
	box(win, 0, 0);
	window_obj win_obj;
	win_obj.win_ptr = win;
	windows[win_num - 1] = win_obj;
}

//Create the windows as needed
void create_windows()
{
	struct winsize w = get_win();
	double width = double(w.ws_col) / 3;
	int w_int = int(floor(width));
	for(int i = 1; i <= total_windows; i++)
	{
		mk_window(i, w.ws_row - 2, w_int);
	}
}


//Get name of current user
char * get_username()
{
	struct passwd *pw = getpwuid(geteuid());
	if(pw)
	{
		return pw->pw_name;
	}
	return "";
}

void window_update()
{
	struct winsize w = get_win();
	fs::path path_data = fs::current_path();
	printw("ACTIVE DIR: %s", path_data.c_str());
	mvprintw(w.ws_row - 1, 0, "USER: %s", get_username());
	refresh();
	print_to_window(windows[0].win_ptr, "test");
	print_to_window(windows[1].win_ptr, "test");
	print_to_window(windows[2].win_ptr, "test");
}

//Refreshing function loaded to new thread
void refresher()
{
	while(running)
	{
		window_update();
	}
}

int main()
{
	initscr();
	cbreak();
	create_windows();
	//Launch refresh func on new thread
	thread(refresher).detach();
	getch();
	running = false;
	endwin();
	return 0;
}
