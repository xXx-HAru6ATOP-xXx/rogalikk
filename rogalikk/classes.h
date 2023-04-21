#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <conio.h>
#include "curses.h"
#include <fstream>
#include <windows.h>

using namespace std;

#define MAP_HEIGHT 20
#define MAP_WIDTH 40
#define ENEMY_COUNT 1

char map[MAP_HEIGHT][MAP_WIDTH];

struct spawn {
	int x, y;
};

int countlog = 0;

void init_ncurses() {
	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_BLACK, COLOR_BLACK);
}

class gui {
public:

};

class character {
public:
	int hp, dmg, _x = -1, _y = -1;
	character(int _hp, int _dmg) { hp = _hp; dmg = _dmg; }
	int gethp() { return hp; }
	int getdmg() { return dmg; }
};

class player : public character {
private:
	int lvl = 1, hammeruses = 3, potionuses = 1;
public:
	int exp = 0;
	player(int x, int y) : character(30, 100) { _x = x; _y = y; }
	void spawn(int x, int y) { _x = x; _y = y; }
	int getx() { return _x; }
	int gety() { return _y; }
	void move(int dx, int dy) {
		int x = _x + dx;
		int y = _y + dy;
		if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && map[y][x] == '.') {
			_x = x;
			_y = y;
		}
	}
	void again() {
		hammeruses = 3; potionuses = 1;
	}
	void potion() {
		if (potionuses == 1) {
			potionuses--;
			cout << "You've used healing potion! Healed 10 healthpoints!" << endl;
			hp += 10;
			if (hp > 20) hp = 20;
			countlog++;
			if (countlog == 6) {
				clear();
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				COORD pos = { 0, 0 };
				SetConsoleCursorPosition(hConsole, pos);
				for (int i = 0; i < 23; i++)
					cout << endl;
				countlog = 0;
			}
		}
	}
	void hammer(int dx, int dy) {
		int x = _x + dx;
		int y = _y + dy;
		if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && map[y][x] == '#' && hammeruses != 0) {
			map[y][x] = '.';
			hammeruses--;
			cout << "You've used the hammer! Now you have " << hammeruses << " charges!" << endl;
			countlog++;
			if (countlog == 6) {
				clear();
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				COORD pos = { 0, 0 };
				SetConsoleCursorPosition(hConsole, pos);
				for (int i = 0; i < 23; i++)
					cout << endl;
				countlog = 0;
			}
		}
	}
};

class enemy : public character {
public:
	enemy() : character(10, 3) {}
	void spawnenemy() {
		int randx, randy;
		bool spawned = false;
		while (!spawned) {
			randx = rand() % MAP_WIDTH;
			randy = rand() % MAP_HEIGHT;
			if (map[randy][randx] != '#' && map[randy][randx] != '@' && map[randy][randx] != 'E') {
				_x = randx; _y = randy;
				map[randy][randx] = 'E';
				spawned = true;
			}
		}
	}
	void maketurn(player& hero) {
		int dx, dy;
			// Calculate distance between enemy and player
			int dist_x = hero.getx() - _x;
			int dist_y = hero.gety() - _y;
			int dist = sqrt(dist_x * dist_x + dist_y * dist_y);
			int dmgdeal = dmg - dmg/2 + rand() % (dmg - dmg/2);

			// If player is within range, pursue them
			if (dist <= 5) {
				if (dist_x > 0) {
					dx = 1;
				}
				else if (dist_x < 0) {
					dx = -1;
				}
				else {
					dx = 0;
				}
				if (dist_y > 0) {
					dy = 1;
				}
				else if (dist_y < 0) {
					dy = -1;
				}
				else {
					dy = 0;
				}
				if (dist == 1) { 
					hero.hp -= dmgdeal;
					cout << "Ouch! You've got " << dmgdeal << " damage!" << endl;
					countlog++;
					if (countlog == 6) {
						clear();
						HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
						COORD pos = { 0, 0 };
						SetConsoleCursorPosition(hConsole, pos);
						for (int i = 0; i < 23; i++)
							cout << endl;
						countlog = 0;
					}
				}
			}
			else {
				// If player is out of range, move randomly
				dx = rand() % 3 - 1;
				dy = rand() % 3 - 1;
			}

			// Check if new position is valid
			if (map[_y + dy][_x + dx] == '.') {
				map[_y][_x] = '.';
				_x += dx;
				_y += dy;
				map[_y][_x] = 'E';
			}
	}
	void attack(int dx, int dy, player& hero) {
		int x = _x + dx;
		int y = _y + dy;
		int dmgdeal = hero.dmg - hero.dmg/2 + rand() % (hero.dmg - hero.dmg/2);
		if (x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && y == hero._y && x == hero._x) {
			hp -= dmgdeal;
			cout << "You've damaged a monster with " << dmgdeal << " ATTACK! ";
			countlog++;
			if (hp < 1) {
				cout << "He's dead!" << endl;
				map[_y][_x] = '.';
				_x = -1;
				_y = -1;
				hero.exp++;
			}
			else
				cout << "Now he's health is " << hp << "!" << endl;
			if (countlog == 6) {
				clear();
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				COORD pos = { 0, 0 };
				SetConsoleCursorPosition(hConsole, pos);
				for (int i = 0; i < 23; i++)
					cout << endl;
				countlog = 0;
			}
		}
	}
};

class world {
public:
	void clearlvl() {
		for (int y = 0; y < MAP_HEIGHT; y++)
			for (int x = 0; x < MAP_WIDTH; x++)
				map[y][x] = '.';
	}
	void drawlevel() {
		erase();
		for (int y = 0; y < MAP_HEIGHT; y++) {
			for (int x = 0; x < MAP_WIDTH; x++) {
				if (map[y][x] == '@') {
					init_pair(1, COLOR_YELLOW, COLOR_BLACK);
					attron(COLOR_PAIR(1));
					printw("@");
					attroff(COLOR_PAIR(1));

				}
				else if (map[y][x] == 'E') {
					attron(COLOR_PAIR(2));
					printw("E");
					attroff(COLOR_PAIR(2));
				}
				else if (map[y][x] == '#') printw("#");
				else if (map[y][x] == '.') {
					attron(COLOR_PAIR(3));
					printw(".");
					attroff(COLOR_PAIR(3));
				}
			}
			printw("\n");
		}
		refresh();
	}
	void generatelvl() {
		clearlvl();
		for (int x = 0; x < MAP_WIDTH; x++) {
			map[0][x] = '#';
			map[MAP_HEIGHT - 1][x] = '#';
		}
		for (int y = 0; y < MAP_HEIGHT; y++) {
			map[y][0] = '#';
			map[y][MAP_WIDTH - 1] = '#';
		}
		srand(time(NULL));
		for (int i = 0; i < (MAP_WIDTH * MAP_HEIGHT) / 3; i++) {
			int x = rand() % (MAP_WIDTH - 2) + 1;
			int y = rand() % (MAP_HEIGHT - 2) + 1;
			map[y][x] = '#';
		}
	}
	bool is_clear_area(int x, int y) {
		int clear_tiles = 0;
		for (int i = 0; i < 9; i++) {
			int x1 = x + (i % 3);
			int y1 = y + (i / 3);
			if (map[y1][x1] == '.') {
				clear_tiles++;
			}
		}
		return (clear_tiles == 9);
	}
	spawn find_clear_areas() {
		spawn answer{};
		for (int y = 1; y < MAP_HEIGHT - 3; y++)
			for (int x = 1; x < MAP_WIDTH - 3; x++)
				if (is_clear_area(x, y)) {
					answer.x = x; answer.y = y;
				}
		return answer;
	}
};

class game {
public:
	void initgame() {
		init_ncurses();
		int levels = 0;
		world level;
		player hero(-1, -1);
		spawn temp;
		while (levels != 2) {
			int countenemy = 0;
			bool enemyturn = false;
			bool winrule = false;
			level.generatelvl();
			levels++;
			temp = level.find_clear_areas();
			map[temp.y][temp.x] = '@';
			hero.spawn(temp.x, temp.y);
			hero.again();
			vector <enemy> monsters;
			for (int i = 0; i < ENEMY_COUNT; i++) {
				enemy temp;
				temp.spawnenemy();
				monsters.push_back(temp);
			}
			level.drawlevel();
			for (int i = 0; i < 23; i++)
				cout << endl;
			while (hero.gethp() > 0) {
				int ch = getch();
				switch (ch) {
				case 't':
					hero.hammer(0, -1);
					break;
				case 'f':
					hero.hammer(-1, 0);
					break;
				case 'g':
					hero.hammer(0, 1);
					break;
				case 'h':
					hero.hammer(1, 0);
					break;
				case '1':
					hero.potion();
					break;
				case KEY_UP:
					map[hero.gety()][hero.getx()] = '.';
					hero.move(0, -1);
					map[hero.gety()][hero.getx()] = '@';
					break;
				case KEY_DOWN:
					map[hero.gety()][hero.getx()] = '.';
					hero.move(0, 1);
					map[hero.gety()][hero.getx()] = '@';
					break;
				case KEY_LEFT:
					map[hero.gety()][hero.getx()] = '.';
					hero.move(-1, 0);
					map[hero.gety()][hero.getx()] = '@';
					break;
				case KEY_RIGHT:
					map[hero.gety()][hero.getx()] = '.';
					hero.move(1, 0);
					map[hero.gety()][hero.getx()] = '@';
					break;
				case 'd':
					for (int i = 0; i < ENEMY_COUNT; i++)
						monsters.at(i).attack(-1, 0, hero);
					break;
				case 'w':
					for (int i = 0; i < ENEMY_COUNT; i++)
						monsters.at(i).attack(0, 1, hero);
					break;
				case 'a':
					for (int i = 0; i < ENEMY_COUNT; i++)
						monsters.at(i).attack(1, 0, hero);
					break;
				case 's':
					for (int i = 0; i < ENEMY_COUNT; i++)
						monsters.at(i).attack(0, -1, hero);
					break;
				}
				if (enemyturn) {
					for (int i = 0; i < ENEMY_COUNT; i++)
						monsters.at(i).maketurn(hero);
					enemyturn = false;
				}
				else enemyturn = true;
				level.drawlevel();
				printw("HP: %d\nEXP: %d\n", hero.gethp(), hero.exp);
				printw("LEVEL: %d\n", levels);
				for (int i = 0; i < ENEMY_COUNT; i++)
					if (monsters.at(i)._x == -1)
						countenemy++;
				if (countenemy == ENEMY_COUNT) {
					for (int i = 0; i < ENEMY_COUNT; i++)
						monsters.pop_back();
					winrule = true;
					break;
				}
				countenemy = 0;
			}
			if (winrule) {
				clear();
				printw("You won!");
				getch();
				refresh();
				clear();
			}
			else {
				clear();
				printw("You lost!");
				getch();
				break;
			}
		}
		endwin();
	}
};