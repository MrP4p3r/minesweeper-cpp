/* coding: utf-8 */

#include <stdio.h>
#include <stdlib.h>

#include <ctime>
#include <clocale>

#include <string.h>
#include <math.h>

#include <GL/glut.h>
#include <GL/glu.h>

// #pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#define FRAMERATE 60
#define TIMER_FUNCTION_DELAY 1000/FRAMERATE

using namespace std;

class minesweeper
{
	enum celltype {
		TYPE_MINE,
		TYPE_EMPTY
	};
	enum conttype {
		CONT_EMPTY,
		CONT_QM,
		CONT_FLAG,
		CONT_WRONGFLAG,
		CONT_TRUEFLAG,
		CONT_COUNT,
		CONT_MINE,
		CONT_EXPLOSION,
		CONT_CLICKED
	};
	struct kletka
	{
		celltype type;  //тип клетки
		unsigned char count;  //количество мин вокруг клетки
		conttype cont;  //то, что выводится на экран
	};
public:
	int res_x, res_y; //разрешение окна
	int fld_top, fld_bot, fld_lef, fld_rig; //координаты сторон поля
	int cell_size;
	//ФУНКЦИИ
	void initGame();
	void restartGame();
	void drawfield();
	void processTimeAndScore();
	void leftclick(int x, int y);
	void rightclick(int x, int y);
private:
	kletka **mas; //массив клеток
	kletka **masm; //массив указателей на клетки с минами
	kletka **masf; //массив флагов
	time_t start_time;
	char *cur_time_string = new char[7];
	char *mines_left = new char[3];
	int x, y, m; //количество столбцов, строк, мин
	int newX, newY, newM; //новое количество столбцов, строк, мин
	int flagcount; //количество проставленных флагов
	int cellsrevealed; //количество раскрытых клеток
	bool lossflag;  //флаг поражения
	bool winflag;   //флаг победы
	//ФУНКЦИИ
	void initFieldArrays();
	void deleteFieldArrays();
	void initField();
	void drawcells();
	void revealCell(int X, int Y);
	void changeCellState(int X, int Y);
	void revealMines();
	void checkMines();
	void setValues(int nx, int ny, int nm);
};

minesweeper game;

void display();
void timer(int value);
void reshape(int width, int height);
void kbdownfunc(unsigned char KEY, int X, int Y);
void kbupfunc(unsigned char KEY, int X, int Y);
void mousefunc(int button, int state, int x, int y);
void kbactions();
void printString(const char *text, int x, int y);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "Russian");
    srand(unsigned(time(NULL)));

    game.initGame();

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowPosition(400, 250);
    glutInitWindowSize(game.res_x, game.res_y);

    glutCreateWindow("Minesweeper");
    glClearColor(0.6, 0.6, 0.6, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, game.res_x, 0.0, game.res_y, -1.0, 1.0);

    glutDisplayFunc(&display);
    glutKeyboardFunc(&kbdownfunc);
    glutKeyboardUpFunc(&kbupfunc);
    glutMouseFunc(&mousefunc);
    glutReshapeFunc(&reshape);
    glutTimerFunc(TIMER_FUNCTION_DELAY, &timer, 0);

    glutMainLoop();
    return 0;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	game.drawfield();
	glutSwapBuffers();
}

void timer(int value)
{
	game.processTimeAndScore();
	if (game.res_x != glutGet(GLUT_WINDOW_WIDTH) || game.res_y != glutGet(GLUT_WINDOW_HEIGHT))
		glutReshapeWindow(game.res_x, game.res_y);
	glutPostRedisplay();
	glutTimerFunc(TIMER_FUNCTION_DELAY, &timer, 0);
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, width, 0, height);
}

void kbdownfunc(unsigned char KEY, int X, int Y)
{
	if (KEY == 27) exit(0); //выход по Esc
}

void kbupfunc(unsigned char KEY, int X, int Y)
{

}

void mousefunc(int button, int state, int X, int Y)
{
	if (state == GLUT_DOWN)
	{
		if (button == GLUT_LEFT_BUTTON)
		{
			game.leftclick(X, game.res_y - Y);
		}
		if (button == GLUT_RIGHT_BUTTON)
		{
			game.rightclick(X, game.res_y - Y);
		}
	}
}

void minesweeper::initGame()
{
	cell_size = 29;

	x = newX = 9; //клеток по x (столбцы)
	y = newY = 9; //клеток по y (строки)
	res_x = x*(cell_size + 1) - 1 + 10;
	res_y = y*(cell_size + 1) - 1 + 10 + 80;
	fld_lef = 5;
	fld_rig = res_x - 5;
	fld_bot = 5;
	fld_top = res_y - 85;
	start_time = time(NULL);

	m = newM = 10;
	flagcount = 0;
	cellsrevealed = 0;
	lossflag = 0;
	winflag = 0;

	initFieldArrays();
	initField();
}

void minesweeper::restartGame()
{
	deleteFieldArrays();
	x = newX; //клеток по x (столбцы)
	y = newY; //клеток по y (строки)
	res_x = x*(cell_size + 1) - 1 + 10;
	res_y = y*(cell_size + 1) - 1 + 10 + 80;
	fld_lef = 5;
	fld_rig = res_x - 5;
	fld_bot = 5;
	fld_top = res_y - 85;
	start_time = time(NULL);

	m = newM;
	flagcount = 0;
	cellsrevealed = 0;
	lossflag = 0;
	winflag = 0;

	glutReshapeWindow(res_x, res_y);
	initFieldArrays();
	initField();
}

void minesweeper::initFieldArrays()
{
	mas = new kletka*[x];
	for (int i = 0; i < x; i++)
	{
		mas[i] = new kletka[y];
	}
	masm = new kletka*[m];
	masf = new kletka*[m];
}

void minesweeper::deleteFieldArrays()
{
	if (!mas) return;
	for (int i = 0; i < x; i++)
	{
		delete(mas[i]);
	}
	delete(mas);
	delete(masm);
}

void minesweeper::initField()
{
	for (int j = 0; j < y; j++)
		//заполнение всех клеток как пустых
	{
		for (int i = 0; i < x; i++)
		{
			mas[i][j].type = TYPE_EMPTY;
			mas[i][j].count = 0;
			mas[i][j].cont = CONT_EMPTY;
		}
	}
	int i, j;
	for (int k = 0; k < m; k++)
		//ставим мины в случайных местах и увеличиваем количество рядом лежащих мин у лежащих вокруг клеток на 1
	{
		do
		{
			i = rand() % x;
			j = rand() % y;
		} while (mas[i][j].type == TYPE_MINE);
		mas[i][j].count = 0;
		mas[i][j].type = TYPE_MINE;
		masm[k] = &mas[i][j];

		if (i > 0)
		{
			mas[i - 1][j].count++;
			if (j > 0) mas[i - 1][j - 1].count++;
			if (j < y - 1) mas[i - 1][j + 1].count++;
		}
		if (j > 0) mas[i][j - 1].count++;
		if (j < y - 1) mas[i][j + 1].count++;
		if (i < x - 1)
		{
			mas[i + 1][j].count++;
			if (j > 0) mas[i + 1][j - 1].count++;
			if (j < y - 1) mas[i + 1][j + 1].count++;
		}
	}
}

void minesweeper::drawfield()
{
	glColor3f(0.9, 0.9, 0.9);
	glBegin(GL_QUADS);
	glVertex2f(0, res_y);
	glVertex2f(res_x, res_y);
	glVertex2f(res_x, res_y - 22);
	glVertex2f(0, res_y - 22);
	glEnd();

	if (lossflag) glColor3f(0.92, 0.18, 0.18);
	else if (winflag) glColor3f(0.6353, 0.9333, 0.2);
	else glColor3f(0.4, 0.4, 0.4);
	//Смайлик
	glBegin(GL_QUADS);
	glVertex2f(res_x / 2 - 24, res_y - 77);
	glVertex2f(res_x / 2 + 24, res_y - 77);
	glVertex2f(res_x / 2 + 24, res_y - 29);
	glVertex2f(res_x / 2 - 24, res_y - 29);
	glEnd();

	glColor3f(0.4, 0.4, 0.4);
	//Кнопки 9x9x10, 16x16x40, 30x16x99
	glBegin(GL_LINE_LOOP);
	glVertex2f(0, res_y - 21);
	glVertex2f(62, res_y - 21);
	glVertex2f(62, res_y);
	glVertex2f(1, res_y);
	glEnd();
	printString("9x9x10", 4, res_y - 17);
	glBegin(GL_LINE_LOOP);
	glVertex2f(62, res_y - 21);
	glVertex2f(141, res_y - 21);
	glVertex2f(141, res_y);
	glVertex2f(62, res_y);
	glEnd();
	printString("16x16x40", 65, res_y - 17);
	glBegin(GL_LINE_LOOP);
	glVertex2f(141, res_y - 21);
	glVertex2f(220, res_y - 21);
	glVertex2f(220, res_y);
	glVertex2f(141, res_y);
	glEnd();
	printString("30x16x99", 144, res_y - 17);

	glColor3f(0.9, 0.9, 0.9);
	//Таймер
	printString(cur_time_string, 5, res_y - 77);
	//"Оставшиеся" мины
	if (m - flagcount < 0) glColor3f(1, 0, 0);
	printString(mines_left, res_x - 22, res_y - 77);

	glColor3f(0.9, 0.9, 0.9);
	glBegin(GL_LINE_LOOP);
	glVertex2f(fld_lef - 1, fld_bot);
	glVertex2f(fld_rig + 1, fld_bot);
	glVertex2f(fld_rig + 1, fld_top + 1);
	glVertex2f(fld_lef, fld_top + 1);
	glEnd();

	drawcells();

	//сетка
	glColor3f(0.7, 0.7, 0.7);
	glBegin(GL_LINES);
	for (int i = 1; i < x; i++) //вертикальные линии
	{
		glVertex2f(fld_lef + i*(cell_size + 1), fld_bot);
		glVertex2f(fld_lef + i*(cell_size + 1), fld_top);
	}
	for (int i = 1; i < y; i++) //горизонтальные линии
	{
		glVertex2f(fld_lef, fld_bot + i*(cell_size + 1));
		glVertex2f(fld_rig, fld_bot + i*(cell_size + 1));
	}
	glEnd();
}

void minesweeper::drawcells()
{
	char *count = new char[2];
	for (int j = 0; j < y; j++)
	{
		for (int i = 0; i < x; i++)
		{
			switch (mas[i][j].cont)
			{
			case CONT_EMPTY:
				glColor3f(0.8, 0.8, 0.8);
				break;
			case CONT_CLICKED:
			case CONT_COUNT:
				glColor3f(0.9, 0.9, 0.9);
				break;
			case CONT_FLAG:
				glColor3f(0.188, 0.6274, 0.8784);
				break;
			case CONT_WRONGFLAG:
				glColor3f(0.124, 0.414, 0.58);
				break;
			case CONT_TRUEFLAG:
				glColor3f(0.6353, 0.9333, 0.2);
				break;
			case CONT_QM:
				glColor3f(0.933, 0.867, 0.204);
				break;
			case CONT_MINE:
				glColor3f(0.2, 0.2, 0.2);
				break;
			case CONT_EXPLOSION:
				glColor3f(0.92, 0.18, 0.18);
				break;
			}
			glBegin(GL_QUADS);
			glVertex2f(fld_lef + i*(cell_size + 1), fld_top - j*(cell_size + 1));
			glVertex2f(fld_lef + i*(cell_size + 1), fld_top - j*(cell_size + 1) - cell_size);
			glVertex2f(fld_lef + i*(cell_size + 1) + cell_size, fld_top - j*(cell_size + 1) - cell_size);
			glVertex2f(fld_lef + i*(cell_size + 1) + cell_size, fld_top - j*(cell_size + 1));
			glEnd();
			if (mas[i][j].cont == CONT_COUNT)
			{
				glColor3f(0.4, 0.4, 0.4);
				count[0] = mas[i][j].count + 0x30;
				count[1] = '\0';
				printString(count, fld_lef + i*(cell_size + 1) + 10, fld_top - j*(cell_size + 1) - 18);
				goto metka;
			}
			if (mas[i][j].cont == CONT_FLAG || mas[i][j].cont == CONT_TRUEFLAG || mas[i][j].cont == CONT_WRONGFLAG)
			{
				glColor3f(0, 0, 0);
				count[0] = 'P';
				count[1] = '\0';
				printString(count, fld_lef + i*(cell_size + 1) + 10, fld_top - j*(cell_size + 1) - 18);
				goto metka;
			}
			if (mas[i][j].cont == CONT_MINE)
			{
				glColor3f(1, 1, 1);
				count[0] = '*';
				count[1] = '\0';
				printString(count, fld_lef + i*(cell_size + 1) + 10, fld_top - j*(cell_size + 1) - 18);
				goto metka;
			}
			if (mas[i][j].cont == CONT_QM)
			{
				glColor3f(0.2, 0.2, 0.2);
				count[0] = '?';
				count[1] = '\0';
				printString(count, fld_lef + i*(cell_size + 1) + 10, fld_top - j*(cell_size + 1) - 18);
				goto metka;
			}
		metka:;
		}
	}
}

void minesweeper::leftclick(int X, int Y)
{
	if (X > fld_lef && X < fld_rig && Y > fld_bot && Y < fld_top && !lossflag && !winflag)
	{
		X = (X - fld_lef) / (cell_size + 1);
		Y = (fld_top - Y) / (cell_size + 1);
		revealCell(X, Y);
		return;
	}
	if (Y > res_y - 22)
	{
		if (X < 62) setValues(9, 9, 10);
		else if (X < 141) setValues(16, 16, 40);
		else if (X < 220) setValues(30, 16, 99);
		return;
	}
	if (Y > res_y - 77 && Y < res_y - 29 && X > res_x / 2 - 24 && X < res_x / 2 + 24) restartGame();
}

void minesweeper::rightclick(int X, int Y)
{
	if (X > fld_lef && X < fld_rig && Y > fld_bot && Y < fld_top && !lossflag && !winflag && !lossflag)
	{
		X = (X - fld_lef) / (cell_size + 1);
		Y = (fld_top - Y) / (cell_size + 1);
		changeCellState(X, Y);
		return;
	}
}

void minesweeper::revealCell(int X, int Y)
{
	if (mas[X][Y].cont == CONT_EMPTY)
	{
		if (mas[X][Y].type == TYPE_EMPTY)
		{
			cellsrevealed++;
			if (!mas[X][Y].count)
			{
				mas[X][Y].cont = CONT_CLICKED;
				if (X > 0)
				{
					revealCell(X - 1, Y);
					if (Y > 0) revealCell(X - 1, Y - 1);
					if (Y < y - 1) revealCell(X - 1, Y + 1);
				}
				if (X < x - 1)
				{
					revealCell(X + 1, Y);
					if (Y > 0) revealCell(X + 1, Y - 1);
					if (Y < y - 1) revealCell(X + 1, Y + 1);
				}
				if (Y > 0) revealCell(X, Y - 1);
				if (Y < y - 1) revealCell(X, Y + 1);
			}
			else mas[X][Y].cont = CONT_COUNT;
			checkMines();
		}
		else
		{
			lossflag = true;
			mas[X][Y].cont = CONT_EXPLOSION;
			revealMines();
		}
	}
}

void minesweeper::changeCellState(int X, int Y)
{
	if (mas[X][Y].cont == CONT_EMPTY)
	{
		mas[X][Y].cont = CONT_FLAG;
		masf[flagcount] = &mas[X][Y];
		flagcount++;
		checkMines();
		return;
	}
	if (mas[X][Y].cont == CONT_FLAG)
	{
		mas[X][Y].cont = CONT_QM;
		flagcount--;
		return;
	}
	if (mas[X][Y].cont == CONT_QM)
	{
		mas[X][Y].cont = CONT_EMPTY;
	}
}

void minesweeper::revealMines()
{
	for (int i = 0; i < m; i++)
	{
		if (masm[i]->cont == CONT_EMPTY || masm[i]->cont == CONT_QM) masm[i]->cont = CONT_MINE;
	}
	for (int i = 0; i < flagcount; i++)
	{
		masf[i]->cont = masf[i]->type == TYPE_MINE ? CONT_TRUEFLAG : CONT_WRONGFLAG;
	}
}

void minesweeper::checkMines()
{
	if (flagcount + cellsrevealed == x*y && flagcount == m)
	{
		int minesfound = 0;
		for (int i = 0; i < m && minesfound == i; i++)
		{
			for (int j = 0; j < m; j++)
			{
				if (masf[i] = masm[j])
				{
					minesfound++;
					break;
				}
			}
		}
		if (minesfound == m)
		{
			winflag = true;
			strcpy(game.mines_left, "00");
			for (int i = 0; i < m; i++) masm[i]->cont = CONT_TRUEFLAG;
		}
	}
}

void minesweeper::setValues(int nx, int ny, int nm)
{
	newX = nx;
	newY = ny;
	newM = nm;
}

void minesweeper::processTimeAndScore()
{
	if (winflag || lossflag) return;
	if (m - flagcount < 0) return;

    time_t time_delta = time(NULL) - start_time;

	time_t seconds = time_delta % 60;
	time_t minutes = time_delta / 60 % 60;

	minutes = minutes / 60;

	cur_time_string[2] = (minutes % 10) + 0x30;
	minutes = minutes / 10;
	cur_time_string[1] = (minutes % 10) + 0x30;
	minutes = minutes / 10;
	cur_time_string[0] = (minutes % 10) + 0x30;

	cur_time_string[3] = ':';

	cur_time_string[5] = (seconds % 10) + 0x30;
	seconds = seconds / 10;
	cur_time_string[4] = (seconds % 10) + 0x30;
	cur_time_string[6] = '\0';

	mines_left[1] = (m - flagcount) % 10 + 0x30;
	mines_left[0] = (m - flagcount) / 10 % 10 + 0x30;
	mines_left[2] = '\0';
}

void printString(const char *text, int x, int y)
{
	glRasterPos2f(x, y);
	while (*text) glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *text++);
}
