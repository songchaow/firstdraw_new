#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <time.h>
#include <math.h>
#include <algorithm>
#define SPEEDOFMOVE 5                //自己移动速率
#define SPEEDOFBULLET 2.5              //己方子弹移动速率
#define SPEEDOFEBULLET 2             //敌方子弹移动速率
#define DENSITYOFBULLET 30           //己方子弹密度
#define INTERVALOFENEMY 30           //敌人初始化频率
#define INTERVALOFENEMYLATER 3000    //敌人补充频率
#define INTERVALOFENEMYMOVE 30       //敌人位置重算频率
#define INTERVALOFENEMYATTACK 100    //敌人攻击频率
#define NUMBEROFBULLET 10000         //初始子弹数
#define MINDISTANCE 13               //敌人被攻击的最小距离
#define PROBABILITYOFATTACK 3        //敌人攻击概率
#define UPNUMBER 21504               //以下对键盘上特定键的标记
#define DOWNNUMBER 21760
#define LEFTNUMBER 20992
#define RIGHTNUMBER 21248
#define ENTER 17165
#define LENGTHOFEBLOOD 100
#define LENGTHOFBLOOD 300
#define NUMOFENEMY 10
#define TOTALNUMOFENEMY 50           //敌人总数量
#define MAX_FPS 100 // 最高帧频
/*void scroll(void)
{
   blit(screen, screen, 0, 32, 0, 24, SCREEN_W / 2, SCREEN_H-32);
   rectfill(screen, 0, SCREEN_H-16, SCREEN_W / 2, SCREEN_H-1, makecol(255, 255, 255));
}*/

const char* str1 = "VERSION:1.0    This program is produced by WSC. Press any key to go back.";
const char* str2 = "Sorry to display all of this in English, for the allegro library is not compatible with Chinese.";
int initcom = 0, numofenemy = 0, totalnum = 0, k, blood = 100, numofbullet = NUMBEROFBULLET, counter1 = 0, counter2 = 0, counter3 = 0, counter4 = 0, enemystate = 0, state = 0;//k存储键盘状态，counter实现for内一个函数的“打稀”的运行，enemystate为敌人初始化开关
unsigned int xofme = 50, yofme = 50, t;//定义自己的初始位置，t为
bool isPressed[4] = {false,false,false,false};
enum DIRECTION {UP,LEFT,DOWN,RIGHT};
ALLEGRO_DISPLAY* display;
ALLEGRO_MOUSE_STATE mouse_state;
struct bullet             //存储子弹数据，并指向下一个子弹
{
	unsigned int state;
	double x;
	double y;
	double speedx;
	double speedy;
	struct bullet* next;
};
struct ebullet
{
	unsigned int state;
	double x;
	double y;
	double speedx;
	double speedy;
	struct ebullet* next;
};
struct enemy              //存储敌人数据，并指向下一个敌人
{
	int state;
	int interval;
	int x; //关键
	int y;
	unsigned int blood;
	double speedx;
	double speedy;
	struct enemy* next;
};
int randadd()
{
	int s = rand() + 90;
	if (s < 5000) return -1;
	else if (s >= 5000 && s < 10000) return -1;
	else if (s >= 10000 && s < 15000) return 1;
	else if (s > -15000 && s < 20000) return 1;
	else if (s >= 20000 && s < 25000) return 0;
	else if (s >= 25000 && s < 30000) return 0;
	else return 0;
}
/*int randspeed(struct enemy* p)                //均等可能的产生不同的速度大小
{
	int speed;
	return 0;
}*/
void randspeedx(struct enemy* p) //利用上面的速度大小计算敌人x方向速度，考虑边界情况
{
	int speed0 = p->speedx + randadd();
	unsigned int speed = abs(speed0);
	if (speed >= 3) speed0 = (speed0 >= 0 ? 3 : -3);
	speed = abs(speed0);
	if (p->x < 10)
		p->speedx = (double)speed;
	else if (p->x > al_get_display_width(display) - 10)
		p->speedx = -(double)speed;
	else p->speedx = (double)speed0;
}
void randspeedy(struct enemy* p) //利用上面的速度大小计算敌人y方向速度，考虑边界情况
{                                //并赋值到enemy
	int speed0 = p->speedy + randadd();
	unsigned int speed = abs(speed0);
	if (speed >= 3) speed0 = speed0 >= 0 ? 3 : -3;
	speed = abs(speed0);
	if (p->y < 10)
		p->speedy = (double)speed;
	else if (p->y > al_get_display_height(display) - 10)
		p->speedy = -(double)speed;
	else p->speedy = (double)speed0;
}
int randint(int a, int b)
{
	int c = (rand() + 90) % (b - a + 1) + a;
	return c;
}
void timedelay()                //用于延时，暂时未用
{
	int i;
	for (i = 0; i != 10000; i++);
}
int whichx()
{
	if (!isPressed[LEFT] && isPressed[RIGHT]) return (xofme = xofme + SPEEDOFMOVE);
	if (isPressed[LEFT] && !isPressed[RIGHT]) return (xofme = xofme - SPEEDOFMOVE);
	return xofme;
}
int whichy()
{
	if(!isPressed[UP] && isPressed[DOWN]) return (yofme += SPEEDOFMOVE);
	if (isPressed[UP] && !isPressed[DOWN]) return (yofme -= SPEEDOFMOVE);
	return (yofme);
}
struct enemy* heade;
struct enemy* reare;
struct enemy* createenemy()
{
	void moveofenemy(struct enemy*);
	struct enemy* p;
	if (numofenemy < NUMOFENEMY&&totalnum < TOTALNUMOFENEMY)
		for (;;)                  //补充新敌人
		{
			counter2++;
			if (initcom == 0 && counter2%INTERVALOFENEMY != 0) break;
			if (initcom == 1 && counter2%INTERVALOFENEMYLATER != 0) break;
			p = (struct enemy*)malloc(sizeof(struct bullet));
			p->x = (float)(rand() + 90) / 32857 * al_get_display_width(display);
			p->y = (float)(rand() + 90) / 32857 * al_get_display_height(display);
			p->interval = randint(2, 4)*INTERVALOFENEMYATTACK;
			p->state = 1;
			p->blood = 100;
			p->speedx = 0;
			p->speedy = 0;
			p->next = NULL;
			if (heade == NULL) heade = p;
			else reare->next = p;
			reare = p;
			totalnum++;
			if (++numofenemy == NUMOFENEMY) { initcom = 1; break; }
		}
	//if (counter3++%INTERVALOFENEMYMOVE == 0)
		for (p = heade; p != NULL;)
		{
			if (p->state != 1) { p = p->next; continue; }
			randspeedx(p); randspeedy(p);
			moveofenemy(p); p = p->next;
		}
	return heade;
}
void moveofenemy(struct enemy* en)
{
	en->x += en->speedx;
	en->y += en->speedy;
}
/*void delbullet(struct bullet* p)
{
	p->speedx=p->next->speedx;
	p->speedy=p->next->speedy;
	p->state=p->next->state;
	p->x=p->next->x;
	p->y=p->next->y;
	p->next=p->next->next;
}*/
struct bullet* rear = NULL;
struct bullet* head;
struct bullet* createbullet(int x, int y, double movex, double movey)
{
	struct bullet* p;
	p = (struct bullet*)malloc(sizeof(struct bullet));
	p->x = x;
	p->y = y;
	p->state = 1;
	p->speedy = SPEEDOFBULLET * movey;
	p->speedx = SPEEDOFBULLET * movex;
	p->next = NULL;
	if (head == NULL) head = p;
	else rear->next = p;
	rear = p;
	return head;
}
struct ebullet* headeb;
struct ebullet* reareb;
struct ebullet* createebullet(int x, int y, double anglex, double angley) //被敌人调用来创建子弹
{
	struct ebullet* p;
	p = (struct ebullet*)malloc(sizeof(struct ebullet));
	p->x = x;
	p->y = y;
	p->state = 1;
	p->speedx = SPEEDOFEBULLET * anglex;
	p->speedy = SPEEDOFEBULLET * angley;
	p->next = NULL;
	if (headeb == NULL) headeb = p;
	else reareb->next = p;
	reareb = p;
	return headeb;
}
double getmovex() //返回鼠标和自己位置连线的余切
{
	int l, h;
	double movex;

	l = mouse_state.x - xofme;
	h = mouse_state.y - yofme;
	movex = (double)l / sqrt((double)(l*l + h * h));
	return movex;
}
double getmovey() //返回鼠标和自己位置连线的正切
{
	int l, h;
	double movey;
	l = mouse_state.x - xofme;
	h = mouse_state.y - yofme;
	movey = (double)h / sqrt((double)(l*l + h * h));
	return movey;
}
double getanglextome(struct enemy* p) //计算敌人与自己的夹角
{
	double l = (double)xofme - (double)(p->x), h = (double)yofme - (double)(p->y);
	double anglex = l / sqrt(l*l + h * h);
	return anglex;
}
double getangleytome(struct enemy* p)
{
	double l = (double)xofme - (double)(p->x), h = (double)yofme - (double)(p->y);
	double angley = h / sqrt(l*l + h * h);
	return angley;
}
void moveofbullet(struct bullet* p) //根据已求得的速度计算子弹坐标
{
	p->x += p->speedx;
	p->y += p->speedy;
}
double getbulletdistance(struct bullet* p, struct enemy* q)
{
	unsigned int x1 = p->x, y1 = p->y, x2 = q->x, y2 = q->y;
	double l = sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
	return l;
}
double getenbulletdistance(struct ebullet* p) //计算敌人与自己的距离
{
	unsigned int x = p->x, y = p->y;
	double l = sqrt((xofme - x)*(xofme - x) + (yofme - y)*(yofme - y));
	return l;
}
/*void delenemy(struct enemy* p)
{
	p->blood=p->next->blood; //将下一个结点的信息拷贝到p
	p->speedx=p->next->speedx;
	p->speedy=p->next->speedy;
	p->x=p->next->x;
	p->y=p->next->y;
	p->state=p->next->state;
	p->next=p->next->next;
}*/
void enblooddown(struct enemy* q, struct bullet* p) //监视距离并减少敌人血量的函数 参数为指向敌人的指针，和我方子弹的指针
{
	if (q->state == 1 && q->blood >= 5 && p->state == 1 && getbulletdistance(p, q) < MINDISTANCE)
	{
		q->blood -= 5; p->state = 0;//delbullet(p);
	}
	if (q->state == 1 && q->blood < 5 && p->state == 1 && getbulletdistance(p, q) < MINDISTANCE)
	{
		q->state = 0; p->state = 0; numofenemy--;
	}
}
void enemyscan() //监视每个子弹与每个敌人的距离
{
	struct bullet* p; struct enemy* q;
	for (p = head; p != NULL; p = p->next)
		for (q = heade; q != NULL; q = q->next)
			enblooddown(q, p);
}
void ebulletmove(struct ebullet* p)
{
	p->x += p->speedx; p->y += p->speedy;
}
void enemyattack(struct enemy* p) //实现每个敌人的随机攻击
{
	if (randint(1, PROBABILITYOFATTACK) == 1)
		createebullet(p->x, p->y, getanglextome(p), getangleytome(p));
}
void enemyattackscan() //管理每个敌人的随机攻击
{
	struct enemy* p;
	counter4++;
	for (p = heade; p != NULL; p = p->next)
		if (counter4%p->interval == 0 && p->state) enemyattack(p);
}
void myblooddown(struct ebullet* p)
{
	if (getenbulletdistance(p) <= 5 && p->state) { blood -= 5; p->state = 0; }
}
void mybloodscan() //监控自己血量的减少
{
	struct ebullet* p;
	for (p = headeb; p != NULL; p = p->next)
		myblooddown(p);
}
void menu(ALLEGRO_FONT* font)
{
	al_draw_text(font, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 5, ALLEGRO_ALIGN_CENTRE, "A GAME MADE BY WSC");
	al_draw_text(font, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 5+100, ALLEGRO_ALIGN_CENTRE, "START GAME RIGHT NOW");
	al_draw_text(font, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 5+200, ALLEGRO_ALIGN_CENTRE, "EXIT");
	al_draw_text(font, al_map_rgb(0, 0, 0), al_get_display_width(display) / 2, al_get_display_height(display) / 5 + 300, ALLEGRO_ALIGN_CENTRE, "ABOUT");
}
void statedisplay(ALLEGRO_FONT* font)
{
	char str_buff[200];
	sprintf_s(str_buff, 200, "BULLET:%d", numofbullet);
	al_draw_text(font, al_map_rgb(0, 0, 0), 0, 40, ALLEGRO_ALIGN_LEFT,str_buff);
	sprintf_s(str_buff, 200, "BLOOD:%d", blood);
	al_draw_text(font, al_map_rgb(0, 0, 0), 0, 80, ALLEGRO_ALIGN_LEFT, str_buff);
	al_draw_rectangle(400 - LENGTHOFBLOOD / 2, 5, 400 + LENGTHOFBLOOD / 2, 25, al_map_rgb(0, 0, 0), 1);//绘制血量矩形
	al_draw_filled_rectangle(400 - LENGTHOFBLOOD / 2, 5, 400 - LENGTHOFBLOOD / 2 + (float)blood / 100 * LENGTHOFBLOOD, 25, al_map_rgb(255, 0, 0));
}
void choosingretangular(int n)
{
	int screen_w = al_get_display_width(display), screen_h = al_get_display_height(display);
	al_draw_filled_rectangle(screen_w / 2 - 100, screen_h / 2 - 200 + 100 * n, screen_w / 2 + 100, screen_h / 2 + 100 * n - 150, al_map_rgb(255, 0, 0));
}
void showabout(ALLEGRO_FONT* font)
{
	int screen_w = al_get_display_width(display), screen_h = al_get_display_height(display);
	al_clear_to_color(al_map_rgb(255, 255, 255));
	al_draw_text(font, al_map_rgb(0, 0, 0), screen_w / 2, screen_h / 2, ALLEGRO_ALIGN_CENTRE, str1);
	al_draw_text(font, al_map_rgb(0, 0, 0), screen_w / 2, screen_h / 2+40, ALLEGRO_ALIGN_CENTRE, str2);

}
void drawmybullet()
{
	struct bullet* p;
	for (p = head; p != NULL;)
	{
		if (p->state != 1) { p = p->next; continue; }
		//if(counter1%DENSITYOFBULLET==0) //timedelay;
		moveofbullet(p);    //计算子弹移动后坐标
		al_draw_filled_circle(p->x, p->y, 5, al_map_rgb(0, 255, 0));
		p = p->next;         //绘制下一个子弹
	}
}
void drawenemy(ALLEGRO_FONT* font)
{
	struct enemy* ennum;
	for (ennum = createenemy(); ennum != NULL;)//初始化或生成补充的敌人
	{
		if (ennum->state != 1) { ennum = ennum->next; continue; }
		al_draw_filled_circle(ennum->x, ennum->y, 5, al_map_rgb(0, 0, 255)); //绘制敌人
		al_draw_rectangle(ennum->x - LENGTHOFEBLOOD / 2, ennum->y - 10, ennum->x + LENGTHOFEBLOOD / 2, ennum->y - 5, al_map_rgb(0, 0, 0), 1); //绘制血量边框
		al_draw_filled_rectangle(ennum->x - LENGTHOFEBLOOD / 2, ennum->y - 10, ennum->x - 50 + (float)ennum->blood / 100 * LENGTHOFEBLOOD, ennum->y - 5, al_map_rgb(255, 0, 0)); //绘制血量
		char num_str[10];
		sprintf_s(num_str, 10, "%d", ennum->blood);
		al_draw_text(font, al_map_rgb(0, 0, 0), ennum->x, ennum->y - 10, ALLEGRO_ALIGN_CENTRE, num_str);
		ennum = ennum->next;
	}

}
void drawebullet()
{
	struct ebullet *ebul;
	for (ebul = headeb; ebul != NULL; ebul = ebul->next)
	{
		if (!ebul->state) continue;
		ebulletmove(ebul);
		al_draw_filled_circle(ebul->x, ebul->y, 5, al_map_rgb(255, 255, 0));
	}
}
int main()
{
	int result = al_init();
	if (result == 0)
		return 1;
	al_init_font_addon();	// initialize the font addon
	al_init_ttf_addon();	// initialize the ttf (True Type Font) addon
	al_init_primitives_addon();
	al_install_keyboard();
	al_install_mouse();
	// ALLEGRO_BITMAP *bmp = al_create_bitmap(800, 600);
	display = al_create_display(800, 600);
	ALLEGRO_FONT *font = al_load_ttf_font("CALIBRI.TTF", 25, 0);
	int screen_w = al_get_display_width(display), screen_h = al_get_display_height(display);
	ALLEGRO_EVENT_QUEUE * keyboard_queue = al_create_event_queue();
	al_register_event_source(keyboard_queue, al_get_keyboard_event_source());
	ALLEGRO_EVENT_QUEUE* mouse_queue = al_create_event_queue();
	ALLEGRO_EVENT keyboard_event, mouse_event;
	al_register_event_source(mouse_queue, al_get_mouse_event_source());
	for (;;)
	{
		int i = 1;
		
		if (state == 1) { al_clear_to_color(al_map_rgb(255, 255, 255)); al_draw_text(font, al_map_rgb(0, 0, 0), screen_w / 2, screen_h / 2, ALLEGRO_ALIGN_CENTER, "YOU WIN!"); al_flip_display(); al_wait_for_event(keyboard_queue,&keyboard_event); }
		if (state == 2) { al_clear_to_color(al_map_rgb(255, 255, 255)); al_draw_text(font, al_map_rgb(0, 0, 0), screen_w / 2, screen_h / 2, ALLEGRO_ALIGN_CENTER, "YOU LOSE!"); al_flip_display(); al_wait_for_event(keyboard_queue, &keyboard_event); }
		for (;;)
		{
			al_clear_to_color(al_map_rgb(255, 255, 255));
			choosingretangular(i);
			menu(font);
			al_flip_display();
			al_wait_for_event(keyboard_queue, &keyboard_event);
			if (keyboard_event.type == ALLEGRO_EVENT_KEY_DOWN)
			{
				k = keyboard_event.keyboard.keycode;
				if (k == ALLEGRO_KEY_UP) if (i == 1) i = 3; else i--;
				if (k == ALLEGRO_KEY_DOWN) if (i == 3) i = 1; else i++;
				if (k == ALLEGRO_KEY_ENTER)
					switch (i)
					{
					case 1: i = 0; break;      //i=0标记游戏开始
					case 2: i = -2; break;     //i=-2标记退出
					case 3: showabout(font); al_flip_display(); al_wait_for_event(keyboard_queue, &keyboard_event);;  //i=-1标记关于
					}
				if (i == 0 || i == -2) break;
			}
			
		}
		ALLEGRO_TIMER* loop_ticker = al_create_timer(1 / static_cast<double>(MAX_FPS));
		al_start_timer(loop_ticker);
		ALLEGRO_EVENT_QUEUE* tick_queue = al_create_event_queue();
		al_register_event_source(tick_queue, al_get_timer_event_source(loop_ticker));
		// main game loop
		ALLEGRO_EVENT  tick_event_holder;
		for (;;) {
			al_wait_for_event(tick_queue, &tick_event_holder);
			al_flush_event_queue(tick_queue);
			if (i == -2) break;
			if (blood <= 0) { state = 2; break; }
			if (initcom&&numofenemy == 0) { state = 1; break; }
			
			if (al_get_next_event(keyboard_queue, &keyboard_event))
			{
				// change keyboard state
				if (keyboard_event.type == ALLEGRO_EVENT_KEY_DOWN)
				{
					switch (keyboard_event.keyboard.keycode)
					{
					case ALLEGRO_KEY_UP:
						isPressed[UP] = true; break;
					case ALLEGRO_KEY_DOWN:
						isPressed[DOWN] = true; break;
					case ALLEGRO_KEY_LEFT:
						isPressed[LEFT] = true; break;
					case ALLEGRO_KEY_RIGHT:
						isPressed[RIGHT] = true; break;
					default:break;
					}
				}
				else if (keyboard_event.type == ALLEGRO_EVENT_KEY_UP)
				{
					switch (keyboard_event.keyboard.keycode)
					{
					case ALLEGRO_KEY_UP:
						isPressed[UP] = false; break;
					case ALLEGRO_KEY_DOWN:
						isPressed[DOWN] = false; break;
					case ALLEGRO_KEY_LEFT:
						isPressed[LEFT] = false; break;
					case ALLEGRO_KEY_RIGHT:
						isPressed[RIGHT] = false; break;
					default:break;
					}
				}

			}
			al_clear_to_color(al_map_rgb(255, 255, 255));
			al_draw_filled_circle(whichx(), whichy(), 5, al_map_rgb(255, 0, 0));
			//k = 0;
			statedisplay(font);//显示状态
			drawmybullet();//绘制己方子弹
			drawenemy(font);   //绘制敌人
			drawebullet(); //绘制敌方子弹
			enemyscan();      //刷新敌人信息
			enemyattackscan();
			al_get_mouse_state(&mouse_state);
			al_draw_filled_circle(mouse_state.x, mouse_state.y, 5, al_map_rgb(0, 0, 0)); //指明鼠标位置
			counter1++;
			mybloodscan();
			
			if (mouse_state.buttons & 1)
			{
				if (counter1%DENSITYOFBULLET == 0 && numofbullet >= 1)
				{
					createbullet(xofme, yofme, getmovex(), getmovey());
					numofbullet--;
				}
			}
			al_flip_display();
		}
		if (i == -2) break;
	}
	system("exit");
	return 0;
}
