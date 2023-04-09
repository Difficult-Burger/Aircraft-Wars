#include<iostream>
#include<graphics.h>
#include<vector>
#include<conio.h>
using namespace std;
const int swidth = 600; //窗体宽度
const int sheight = 800; //窗体高度
const int enemy_v = 4; //敌机速度
const int bullet_v = 8; //我方子弹速度
const int ebullet_v = 8; //敌方子弹速度
const int hb_interv = 15; //我方子弹出现间隔帧数
const int eb_interv = 60; //敌机子弹出现间隔帧数
const unsigned int HP = 3; //我方血量
const auto CD = 1000; // 伤害冷却1000ms

bool ClickInRect(int x, int y, RECT r)
{
	return(r.left<=x && r.right>=x && r.bottom>=y && r.top<=y);
}

bool rdr(RECT r1, RECT r2)
{
	RECT r;
	r.left = r1.left - (r2.right-r2.left);
	r.right = r1.right;
	r.top = r1.top - (r2.bottom - r2.top);
	r.bottom = r1.bottom;
	return(r2.left >= r.left && r2.left <= r.right && r2.top>=r.top && r2.bottom<=r.bottom);
}

void welcome()
{
	BeginBatchDraw();
	RECT tplayr, texitr;

	//界面上的文字
	LPCTSTR title = _T("WELCOME!");
	LPCTSTR tplay = _T("PLAY");
	LPCTSTR texit = _T("EXIT");

	//设置颜色
	setbkcolor(WHITE);
	cleardevice();
	setcolor(BLACK);

	settextstyle(60, 0, _T("黑体")); //文字高度 宽度 字体

	outtextxy(swidth / 2 - textwidth(title) / 2, sheight / 8, title); //title输出

	//play 矩形坐标设置
	tplayr.left = swidth / 2 - textwidth(tplay) / 2; 
	tplayr.right = tplayr.left + textwidth(tplay);
	tplayr.top = sheight / 3;
	tplayr.bottom = tplayr.top + textheight(tplay);				 

	//exit 矩形坐标设置
	texitr.left = swidth / 2 - textwidth(texit) / 2; 
	texitr.right = texitr.left + textwidth(texit);
	texitr.top = sheight / 1.8;
	texitr.bottom = texitr.top + textheight(texit);

	outtextxy(tplayr.left, tplayr.top, tplay); //play输出
	outtextxy(texitr.left, texitr.top, texit); //exit输出
	
	EndBatchDraw();

	while (1)
	{
		ExMessage msg; //消息接收器
		getmessage(&msg, EM_MOUSE);
		if (msg.lbutton)
		{
			if (ClickInRect(msg.x,msg.y,texitr))
			{
				exit(0);
			}
			if (ClickInRect(msg.x, msg.y, tplayr))
			{
				return;
			}
		}
	}
	
	return;
}

void over(unsigned long long &kill)
{
	TCHAR *str=new TCHAR[128];
	TCHAR s[] = _T("Press Enter To Restart");

	setbkcolor(WHITE);
	cleardevice();
	settextcolor(RED);

	_stprintf_s(str,128, _T("击杀数：%llu"), kill);
	outtextxy(swidth / 2 - textwidth(str)/2, sheight / 5, str);

	settextstyle(40, 20, _T("黑体"));
	outtextxy(swidth/2-textwidth(s)/2, sheight * 0.7, s);
	while (1)
	{
		ExMessage msg;
		msg = getmessage(EM_KEY);
		if (msg.vkcode == 0x0D)
		{
			return;
		}
	}
}

class BK
{
public:
	BK(IMAGE& img) :img(img), y(-sheight){}

	void show()
	{
		y += 4;
		putimage(0, y, &img);
		if (y == 0) y = -sheight;
	}
private:
	int y;
	IMAGE img;
}; 

class HERO
{
private:
	RECT rect;
	IMAGE img;
	unsigned int hp;
public:
	HERO(IMAGE& img) :img(img),hp(HP)
	{
		rect.left = swidth / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.bottom = sheight;
		rect.top = rect.bottom - img.getheight();
		/*ExMessage msg;
		if (peekmessage(&msg, EM_MOUSE))
		{
			rect.left = msg.x - img.getwidth() / 2;
			rect.top = msg.y - img.getheight() / 2;
			rect.right = rect.left + img.getwidth();
			rect.bottom = rect.top + img.getheight();
		}*/
	}
	void show()
	{
		setlinecolor(RED);
		setlinestyle(PS_SOLID| PS_ENDCAP_FLAT, 10);
		putimage(rect.left,rect.top, &img);
		line(rect.left, rect.top - 10, rect.left + (img.getwidth() * hp / HP), rect.top - 10);
	}
	void control()
	{
		ExMessage msg;
		if (peekmessage(&msg, EM_MOUSE))
		{
			rect.left = msg.x - img.getwidth() / 2;
			rect.top = msg.y - img.getheight() / 2;
			rect.right = rect.left + img.getwidth();
			rect.bottom = rect.top + img.getheight();
		}
	}

	bool hurt()
	{
		hp--;
		return (hp != 0);
	}

	RECT getrect()
	{
		return rect;
	}
};

class ENEMY
{
public:
	ENEMY(IMAGE& img,int x,IMAGE* boom) :img(img),isdead(0),boomsum(0)
	{
		selfboom[0] = boom[0];
		selfboom[1] = boom[1];
		selfboom[2] = boom[2];
		
		rect.left = x;
		rect.top = -img.getheight();
		rect.right = rect.left + img.getwidth();
		rect.bottom = 0;
	}

	bool show()
	{
		if (isdead)
		{
			if (boomsum == 3)
			{
				return 0;
			}
			putimage(rect.left, rect.top, selfboom+boomsum);
			boomsum++;
			return 1;
		}
		
		if (rect.top >= sheight)
		{
			return false;
		}
		rect.top+= enemy_v;
		rect.bottom += enemy_v;
		putimage(rect.left, rect.top, &img);
		return true;
	}

	RECT getrect()
	{
		return rect;
	}
	
	void die()
	{
		isdead = 1;
	}

private:
	IMAGE& img;
	RECT rect;
	IMAGE selfboom[3];
	bool isdead;
	int boomsum;
};

class BULLET
{
protected:
	RECT rect;
	IMAGE& img;

public:
	BULLET(IMAGE& img, RECT pr) :img(img)
	{
		rect.left = (pr.left + pr.right) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.bottom = pr.top;
		rect.top = rect.bottom - img.getheight();
	}

	bool show()
	{
		if (rect.bottom <= 0) return 0;
		rect.top -= bullet_v;
		rect.bottom -= bullet_v;
		putimage(rect.left, rect.top, &img);
		return 1;
	}
	RECT getrect()
	{
		return rect;
	}
};

class EBULLET:BULLET
{
public:
	EBULLET(IMAGE& img, RECT pr) :BULLET(img, pr)
	{
		rect.left = (pr.left + pr.right) / 2 - img.getwidth() / 2;
		rect.right = rect.left + img.getwidth();
		rect.bottom = pr.bottom;
		rect.top = rect.bottom - img.getheight();
	}
	bool show()
	{
		if (rect.bottom >=sheight) return 0;
		rect.top += ebullet_v;
		rect.bottom += ebullet_v;
		putimage(rect.left, rect.top, &img);
		return 1;
	}
	RECT getrect()
	{
		return rect;
	}
};

bool ADDENEMY(vector<ENEMY*>& es, IMAGE& enemyimg,IMAGE *boom)
{
	ENEMY* e = new ENEMY(enemyimg, abs(rand()) % (swidth - enemyimg.getwidth()),boom);
	for (auto i : es)
	{
		if (rdr(i->getrect(), e->getrect()))
		{
			delete e;
			return 0;
		}
	}
	es.push_back(e);
	return 1;
}

bool play()
{
	bool is_play = true;
	IMAGE heroimg, enemyimg, bulletimg, bkimg,eboom[3];
	
	cleardevice();

	//加载图片
	{
		loadimage(&heroimg, _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\me1.png"));
		loadimage(&enemyimg, _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\enemy1.png"));
		loadimage(&bulletimg, _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\bullet1.png"));
		loadimage(&bkimg, _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\bk2.png"), swidth, sheight * 2);

		loadimage(&eboom[0], _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\enemy1_down2.png"));
		loadimage(&eboom[1], _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\enemy1_down3.png"));
		loadimage(&eboom[2], _T("C:\\Users\\30706\\Desktop\\飞机大战\\images\\enemy1_down4.png"));
	}

	int bsing = 30;
	unsigned long long  kill = 0;
	BK bk = BK(bkimg);
	HERO hero = HERO(heroimg);
	vector<ENEMY*> es;
	vector<BULLET*> bs;
	vector<EBULLET*> ebs;
	clock_t lasthurt = clock();
	
	for (int i = 0; i < 5; i++)
		ADDENEMY(es, enemyimg,eboom);

	while (is_play)
	{
		BeginBatchDraw();
		
		//子弹放进容器
		bsing++;
		{
			if (bsing % hb_interv == 0)
			{
				bs.push_back(new BULLET(bulletimg, hero.getrect()));
			}
			if (bsing == eb_interv)
			{
				bsing = 0;
				for (auto& i : es)
				{
					ebs.push_back(new EBULLET(bulletimg, i->getrect()));
				}
			}
		}

		bk.show();
		Sleep(3);
		flushmessage();
		Sleep(1);
		hero.control();

		//暂停功能
		if (_kbhit()) //keyboard hit ?
		{
			char c = _getch();
			if (c == 0x20)
			{
				Sleep(100);
				while (1)
				{
					if (_kbhit())
					{
						c = _getch();
						if (c == 0x20)
							break;
					}
				}
			}
		}

		hero.show();		

		auto ebsit = ebs.begin();
		while (ebsit != ebs.end())
		{
			if (rdr(hero.getrect(), (*ebsit)->getrect())) //敌方子弹与我方飞机碰撞检测
			{
				if (clock() - lasthurt >= CD)
				{
					is_play = hero.hurt();
					lasthurt = clock();
					ebsit = ebs.erase(ebsit); //敌机子弹击中我方则从容器中清除

				}
				break;
			}
			else if (!(*ebsit)->show())
			{
				ebsit = ebs.erase(ebsit); //敌机子弹出界则从容器中清除
			}
			else
			{
				ebsit++; //若敌方当前子弹没出界，则检测下一个子弹
			}
		}
		
		//我方子弹出界检测
		auto bsit = bs.begin();
		while (bsit != bs.end())
		{
			if (!(*bsit)->show())
			{
				bsit = bs.erase(bsit); //我方子弹出界则从容器中清除
			}
			else
			{
				bsit++; //若我方当前子弹没出界，则检测下一个子弹
			}
		}

		//敌机被击中检测
		auto it = es.begin();
		while (it != es.end())
		{
			if (rdr(hero.getrect(), (*it)->getrect())) //敌方飞机与我方飞机碰撞检测
			{
				if (clock() - lasthurt >= CD)
				{
					is_play = hero.hurt();
					lasthurt = clock();
				}
			}

			auto bit = bs.begin();
			while (bit != bs.end()) //为当前敌机遍历所有子弹，碰撞检测
			{
				if (rdr((*bit)->getrect(), (*it)->getrect()))
				{
					(*it)->die();
					delete(*bit);
					bs.erase(bit);
					kill++;
					break;
				}
				bit++;
			}

			if (!(*it)->show()) //子弹飞出底边检测
			{
				delete(*it);
				es.erase(it);
				it = es.begin();
			}
			it++;
		}

		//敌机补充，保证始终五架
		for (int i = 0; i < 5 - es.size(); i++) 
			ADDENEMY(es, enemyimg,eboom);

		EndBatchDraw();
	}
	over(kill);

	return 1;
}

int main()
{
	bool live = true;
	initgraph(swidth,sheight);
	while (live)
	{
		welcome();
		live=play();
	}

	return 0;
}