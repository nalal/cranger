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

fs::path path_data = fs::current_path();

//Total windows to be loaded, (this shouldn't be changed)
const int total_windows = 4;
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
	mvwprintw(win, start_y, start_z, "%s", input);
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
    //nodelay(windows[win_num - 1].win_ptr, true);
    keypad(windows[win_num - 1].win_ptr, true);
}

int win_wi;

//Create the windows as needed
void create_windows()
{
	struct winsize w = get_win();
	double width = double(w.ws_col) / 3;
	int w_int = int(floor(width));
	win_wi = w_int;
	for(int i = 1; i <= total_windows; i++)
	{
		if (i != 4)
			mk_window(i, w.ws_row - 2, w_int);
		else
			mk_window(i, w.ws_row, w.ws_col);
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

int files_in_dir = 0;

//Update window
void window_update()
{
	files_in_dir = 0;
	struct winsize w = get_win();
    move(0,0);
    clrtoeol();
	//print_to_window(windows[0].win_ptr, "test");
    int counter_0 = 1;
    for(auto& p: fs::directory_iterator(path_data.parent_path().c_str()))
    {
        char file[NAME_MAX];
        sprintf(file, "%s", p.path().filename().c_str());
        if(counter_0 < w.ws_row - 3)
            print_to_window(windows[0].win_ptr, file, counter_0);
        counter_0++;
    }
	//print_to_window(windows[1].win_ptr, "test");
    int counter_1 = 1;
    for(auto& p: fs::directory_iterator(path_data.c_str()))
    {
        char file[NAME_MAX];
        sprintf(file, "%s", p.path().filename().c_str());
        if(counter_1 < w.ws_row - 3)
   		{
			files_in_dir++;
   		    print_to_window(windows[1].win_ptr, file, counter_1);
			wclrtoeol(windows[1].win_ptr);
		}
        counter_1++;
    }
	
    print_to_window(windows[2].win_ptr, "test");
	mvprintw(0,0, "ACTIVE DIR: %s", path_data.c_str());
	mvprintw(w.ws_row - 1, 0, "USER: %s", get_username());
	box(windows[1].win_ptr, 0, 0);
	box(windows[2].win_ptr, 0, 0);
	box(windows[0].win_ptr, 0, 0);
	refresh();
	
//    int counter_2 = 1;
//    for(auto& p: fs::directory_iterator(path_data.c_str()))
//    {
//        char file[NAME_MAX];
//        sprintf(file, "%s", p.path().filename().c_str());
//        if(counter_2 < w.ws_row - 3)
//            print_to_window(windows[2].win_ptr, file, counter_2);
//        counter_2++;
//    }
}

int keypress;

int cur_x = 1;
int cur_y = 1;

void up_dir()
{
	path_data = path_data.parent_path();
	werase(windows[0].win_ptr);
	werase(windows[1].win_ptr);
	werase(windows[2].win_ptr);
	cur_y = 1;
}

void open_selected(char * selected_file)
{
	char path[2550];
	sprintf(path, "%s/%s", path_data.c_str(), selected_file);
	char * last_space = strrchr(path, 32);
	if(path[last_space - path + 1] == '\0')
	{
		for(int i = last_space - path + 1; i > 0; i--)
		{
			if(path[i] == ' ' || path[i] == '\0')
				path[i] = NULL;
			else
			{
				path[i+1] = '\0';
				break;
			}
		}
	}
	if(fs::exists(path) && fs::is_directory(path))
	{
		path_data = path;
		werase(windows[0].win_ptr);
		werase(windows[1].win_ptr);
		werase(windows[2].win_ptr);
		cur_y = 1;
	}
}

void scroll_down()
{
	if(cur_y != files_in_dir)
		cur_y++;
}

void scroll_up()
{
	if(cur_y != 1)
		cur_y--;
}

//Refreshing function loaded to new thread
void refresher()
{	
	while(running)
	{
		window_update();
		//wmove(windows[1].win_ptr, cur_y, cur_x);
		mvwchgat(windows[1].win_ptr, cur_y, 1, win_wi - 2, A_STANDOUT, 0, NULL);
		wrefresh(windows[1].win_ptr);
		keypress = wgetch(windows[0].win_ptr);
		switch(keypress)
		{
			//BACK FUNCTION
			case KEY_LEFT:
				up_dir();
				break;
			//OPEN FUNCTION
			case KEY_RIGHT:
				char hold[FILENAME_MAX];
				//printf("%i", win_wi);
				winnstr(windows[1].win_ptr, hold, win_wi - 2);
				open_selected(hold);
				break;
			//SCROLL DOWN
			case KEY_DOWN:
				scroll_down();
				break;
			//SCROLL UP
			case KEY_UP:
				scroll_up();
				break;
			//TERMINATE
			case 27:
				running = false;
				break;
			default:
				printf("%i\n", keypress);
				break;
		}
	}
}

int main()
{
	initscr();
	cbreak();
    //noecho();
	create_windows();
	//Launch refresh func on new thread
	refresher();
	endwin();
	return 0;
}
