#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#define N 8
#define Space printf("                       ")
struct cnc
{
	int id;//编号
	int pos;//CNC所在的位置
	int isWork;//是否在工作
	int isShang;//是否在上下料
	int shangTime;//上下料时间
	int UpTime;//上料开始时间
	int DownTime;//下料开始时间
	int SUpTime;
	int SDownTime;
	int firstid;
	int isFirst;
	int waitTime;//等待时间
	int haveOrNo;//是否已经有熟料
	int needTime;//加工一个所需要的时间
	int remainTime;//剩余时间
	int workId;//几号刀片
	int successNum;//成功加工的件数
	int fixRand;//发生故障的概率
	int isNeedFix;//是否出问题
	int fixTime;//需要修复的时间
	int remainFixTime;//修复好剩余需要的时间
}CNC[N];//N个工作台
typedef struct node
{
	int UpTime;
	int DownTime;
	int SUpTime;
	int SDownTime;
	int id;
}Node;
typedef struct rst
{
	double xiao[N];
	int tWu;
	int count;
	int waitTime;
}Rst;
struct rgv
{
	int isClean;//是否在清洗
	int remainTime;//当前工作剩余时间
	int cleanTime;//清洗所需时间
	long int Time;//当前花费总时间
	long int SumWaitTime;//所有CNC总等待时间
	int successCount;//成品数量
	int fixNum;//CNC出问题的概率
	int nowState;//当前的状态是静止还是运动
	int sToM;//静止到运动  一个单位需要的时间
	int mToM;//运动到运动  一个单位需要的时间
	struct cnc* NeedAdd[N];//一个命令周期的顺序
	int sumCommend;//当前有多少个命令等候执行
	int needMove;//是否正在前往某个CNN
	int needWait;//是否在一个这里等待着
	int isTest;//是否在执行一个命令
	int endTime;//最后成品时间
	int testNum;//当前执行工序几
	int isShang;//是否在帮一个东西上料
	int isWait;//是否在原地等待
	int shangTime;//上料剩余的时间
	long int waitTime;//总等待时间
	int wT;
	int moveTime;//前往某个CNN剩余时间
	int nowPos;//当前的位置
	int nowCommendId;//当前命令到第几个了
}RGV;

//全局变量
struct cnc* cncTemp[N];
struct cnc* DoubleTemp1[N];
struct cnc* DoubleTemp2[N];
int Double1 = 0;
int Double2 = 0;
int BeiNum = 0;
int CleanTime;
FILE *result1,*result2,*result3,*result4,*result5;
Rst ret1,ret2;

void swap(struct cnc*x, struct cnc*y);
void Ques1();
void Ques2();
void Ques3();
void Ques3Double();
void Init3();
void Init2(int *Num);
void Init1();
void Init4(int *Num);
void RepeatRun();//重调度算法
void ProspectRun();//预调度算法
void Adjest1();//调整一次
void Adjest2();
void Adjest3();
void Adjest4();
void Adjest5();
void Calculate1(struct cnc* cncTemp[], struct cnc* cnc1Temp[],long int *sum,int l,int r);
void Calculate3(struct cnc* cncTemp[], struct cnc* cnc1Temp[],long int *sum,int l,int r);
void Calculate2(struct cnc* cncTemp[],long int *sum,int l,int r,int flag);
void CalculateTime(struct cnc* cncTemp[],struct cnc* cnc1Temp[],long int *sum);
void FindMin(struct cnc* cncTemp[],struct cnc* cnc1Temp[],long int *sum);
void Require(struct cnc *c,int id);
void Print(int d);//打印整个界面
void Pos(int x, int y);//设置光标位置，决定要在什么位置输出
double Run2();
double Run3();
double Run4();
double Run5();
void CopyFile1(FILE *fp1,FILE *fp2);

int main(void)
{
	srand((unsigned)time(NULL));
	Ques1();
	
	return 0;
}
double Run5()
{
	int i = 0, j = 0, temp = 0,item = 0,count = 0;
	//刚运转，给八个都加上料
	for(i = 0;i < N;++i)
	{
		Print(0);
		Sleep(10);
		temp = (N-i-1)*CNC[i].shangTime;
		item = CNC[i].shangTime;
		//把这个CNC初始化为上料
		CNC[i].isWork = 1;
		CNC[i].remainTime = CNC[i].needTime;
		//把当前上料时间加进去
		CNC[i].UpTime = RGV.Time;
		//其他的需要等待当前的上料时间
		if(CNC[i].id%2 == 1)//当前编号是奇数，则还要加上走路的时间
		{
			if(CNC[i].id != 1)
			{
				temp+=(RGV.sToM*(N-i-1));
				item+=RGV.sToM;
			}
		}
		for(j =  0;j < i;++j)
			CNC[j].remainTime-=item;
		RGV.wT+=temp;
		RGV.Time = RGV.Time+item;
	}
	for(i = 0;i < N;++i)
	{
		RGV.NeedAdd[i] = &CNC[i];
		if(CNC[i].remainTime < 0)  //若有些CNC在我给其他加料的时候已经完成  则会出现负数  加上这个相反数就行了
		{
			RGV.wT +=(CNC[i].remainTime*-1);
			CNC[i].waitTime+=(CNC[i].remainTime*-1);
		}
	}
	RGV.nowPos = 4;
	RGV.isClean = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 8;//总共是八个正常工作
	temp = 0;
	Adjest4();//计算出一种方案  放到第一个位置上
	while (RGV.Time <= 28800)//28800
	{
		//Print(1);
		//Sleep(1);
		RGV.Time++;
		for (i = 0;i < N;++i)
		{
			if (CNC[i].isNeedFix == 0)//没坏  
			{
				if (CNC[i].isShang == 1)//如果我正在上料，因为RGC完成上料后会更新
				{
					CNC[i].waitTime++;
					RGV.wT++;
					continue;
				}
				if(CNC[i].isWork == 1 && CNC[i].remainTime > 0)
				{
					--CNC[i].remainTime;	
				}
				else if(CNC[i].isWork == 1 && CNC[i].remainTime <= 0)
				{
					//正好这一秒完成工作
					CNC[i].remainTime = 0;
					CNC[i].isWork = 0;
				}
				else if(CNC[i].isWork == 0)
				{
					//没有工作了  等待时间++
					CNC[i].waitTime++;
					RGV.wT++;
				}
			}
			else if(CNC[i].isNeedFix == 1 && CNC[i].remainFixTime > 1)
			{
				CNC[i].remainFixTime--;
			}
			else if(CNC[i].isNeedFix == 1 && CNC[i].remainFixTime <= 1)
			{
				//修好了  通知云端  可以安排我工作啦
				CNC[i].isNeedFix = 0;
				RGV.NeedAdd[RGV.sumCommend] = &CNC[i];
				RGV.sumCommend++;
			}
		}

		//RGV会一直处于有命令状态   判断当前RGV的状态
		if(RGV.isClean == 1 && RGV.cleanTime > 0)//如果我在清洗   那就等清洗完
		{
			RGV.cleanTime--;
			RGV.isTest = 0;
		}
		else if(RGV.isClean == 1 && RGV.cleanTime <= 0)//清洗完了  重排一次状态
		{
			RGV.isClean = 0;
			RGV.cleanTime = CleanTime;
			RGV.endTime = RGV.Time;
				//计算当前工作的台数
				count = 0;
				for(i = 0;i < N;++i)
					if(CNC[i].isWork == 1)
						count++;
			RGV.successCount++;
			Adjest4();
		}
		else if(RGV.needMove == 1 && RGV.moveTime > 0)
		{
			//如果正在移动 且时间大于0
			RGV.moveTime--;
		}
		else if(RGV.needMove == 1 && RGV.moveTime == 0)
		{//已经移动到了
			RGV.needMove = 0;
			RGV.nowPos =  RGV.NeedAdd[RGV.nowCommendId]->pos;
			//确定是否要等待
			if(RGV.NeedAdd[RGV.nowCommendId]->remainTime == 0)//如果已经完成  则不需要等待，直接上料  否则就等待
			{
				//计算当前的这个CNC出故障的概率   判断当前的这个CNC是否出故障
				RGV.needWait = 0;
				RGV.isShang = 1;
				RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
				//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
				fprintf(result3,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
				//已经把前一个产品时间存了   更新现在加工的那个时间
				RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
				RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
			}
			else
			{
				RGV.needWait =  1;
				RGV.isShang = 0;
				RGV.waitTime = RGV.NeedAdd[RGV.nowCommendId]->remainTime;
			}	
		}
		else if(RGV.needWait == 1 && RGV.waitTime > 0)
		{
			RGV.waitTime--;
		}
		else if(RGV.needWait == 1 && RGV.waitTime == 0)
		{
			//等待时间已经到了  则开始上料
			RGV.needWait = 0;
			RGV.isShang =  1;
			RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
			//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
			fprintf(result3,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
			//已经把前一个产品时间存了   更新现在加工的那个时间
			RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
			RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
		}
		else if(RGV.isShang ==  1 && RGV.shangTime > 0)//在上料了
		{
			RGV.shangTime--;
		}
		else if(RGV.isShang == 1 && RGV.shangTime == 0)
		{
			//上下料完成
			RGV.isShang = 0;
			RGV.isClean = 1;
			RGV.cleanTime = CleanTime;
			

			//告知那个CNC  你已经被上了新的生料
			RGV.NeedAdd[RGV.nowCommendId]->isShang = 0;
			RGV.NeedAdd[RGV.nowCommendId]->isWork = 1;
			RGV.NeedAdd[RGV.nowCommendId]->remainTime = RGV.NeedAdd[RGV.nowCommendId]->needTime;	
		}
	}
	printf("\n共完成%d个成品\n",RGV.successCount);
	printf("所有CNC一共等待了%d秒   最后成品时间为%d秒  无效时间为%d秒 最后成品出炉时工作的CNC台数为%d台\n",RGV.wT,RGV.endTime,28800-RGV.endTime,count);
	for(i = 0;i < N;++i)
		printf("%d号CNC的效率为%.2lf%%\n",CNC[i].id,(1-CNC[i].waitTime/28800.0)*100);
	return 1;
}
double Run3()
{
	int i = 0, j = 0, temp = 0,item = 0,count = 0;
	//刚运转，给八个都加上料
	for(i = 0;i < N;++i)
	{
		Print(0);
		Sleep(10);
		temp = (N-i-1)*CNC[i].shangTime;
		item = CNC[i].shangTime;
		//把这个CNC初始化为上料
		CNC[i].isWork = 1;
		CNC[i].remainTime = CNC[i].needTime;
		//把当前上料时间加进去
		CNC[i].UpTime = RGV.Time;
		//其他的需要等待当前的上料时间
		if(CNC[i].id%2 == 1)//当前编号是奇数，则还要加上走路的时间
		{
			if(CNC[i].id != 1)
			{
				temp+=(RGV.sToM*(N-i-1));
				item+=RGV.sToM;
			}
		}
		for(j =  0;j < i;++j)
			CNC[j].remainTime-=item;
		RGV.wT+=temp;
		RGV.Time = RGV.Time+item;
	}
	for(i = 0;i < N;++i)
	{
		RGV.NeedAdd[i] = &CNC[i];
		if(CNC[i].remainTime < 0)  //若有些CNC在我给其他加料的时候已经完成  则会出现负数  加上这个相反数就行了
		{
			RGV.wT +=(CNC[i].remainTime*-1);
			CNC[i].waitTime+=(CNC[i].remainTime*-1);
		}
	}
	RGV.nowPos = 4;
	RGV.isClean = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 8;//总共是八个正常工作
	temp = 0;
	Adjest4();//计算出一种方案  放到第一个位置上
	while (RGV.Time <= 28800)//28800
	{
		//Print(1);
		//Sleep(1);
		RGV.Time++;
		for (i = 0;i < N;++i)
		{
			if (CNC[i].isNeedFix == 0)//没坏  
			{
				if (CNC[i].isShang == 1)//如果我正在上料，因为RGC完成上料后会更新
				{
					CNC[i].waitTime++;
					RGV.wT++;
					continue;
				}
				if(CNC[i].isWork == 1 && CNC[i].remainTime > 0)
				{
					--CNC[i].remainTime;	
				}
				else if(CNC[i].isWork == 1 && CNC[i].remainTime <= 0)
				{
					//正好这一秒完成工作
					CNC[i].remainTime = 0;
					CNC[i].isWork = 0;
				}
				else if(CNC[i].isWork == 0)
				{
					//没有工作了  等待时间++
					CNC[i].waitTime++;
					RGV.wT++;
				}
			}
			else if(CNC[i].isNeedFix == 1 && CNC[i].remainFixTime > 1)
			{
				CNC[i].remainFixTime--;
			}
			else if(CNC[i].isNeedFix == 1 && CNC[i].remainFixTime <= 1)
			{
				//修好了  通知云端  可以安排我工作啦
				CNC[i].isNeedFix = 0;
				RGV.NeedAdd[RGV.sumCommend] = &CNC[i];
				RGV.sumCommend++;
			}
		}

		//RGV会一直处于有命令状态   判断当前RGV的状态
		if(RGV.isClean == 1 && RGV.cleanTime > 0)//如果我在清洗   那就等清洗完
		{
			RGV.cleanTime--;
			RGV.isTest = 0;
		}
		else if(RGV.isClean == 1 && RGV.cleanTime <= 0)//清洗完了  重排一次状态
		{
			RGV.isClean = 0;
			RGV.cleanTime = CleanTime;
			RGV.endTime = RGV.Time;
				//计算当前工作的台数
				count = 0;
				for(i = 0;i < N;++i)
					if(CNC[i].isWork == 1)
						count++;
			RGV.successCount++;
			Adjest4();
		}
		else if(RGV.needMove == 1 && RGV.moveTime > 0)
		{
			//如果正在移动 且时间大于0
			RGV.moveTime--;
		}
		else if(RGV.needMove == 1 && RGV.moveTime == 0)
		{//已经移动到了
			RGV.needMove = 0;
			RGV.nowPos =  RGV.NeedAdd[RGV.nowCommendId]->pos;
			//确定是否要等待
			if(RGV.NeedAdd[RGV.nowCommendId]->remainTime == 0)//如果已经完成  则不需要等待，直接上料  否则就等待
			{
				//计算当前的这个CNC出故障的概率   判断当前的这个CNC是否出故障
				if(rand()%100 < RGV.NeedAdd[RGV.nowCommendId]->fixRand)//发生故障，则修改这台CNC的状态 并修改目前RGV中可运行的数目
				{
					//把当前编号  故障开始时间   故障结束时间
					RGV.NeedAdd[RGV.nowCommendId]->isNeedFix = 1;
					RGV.NeedAdd[RGV.nowCommendId]->remainFixTime = rand()%RGV.NeedAdd[RGV.nowCommendId]->fixTime+RGV.NeedAdd[RGV.nowCommendId]->fixTime;
					

					//RGV.NeedAdd[RGV.nowCommendId]->remainFixTime = RGV.NeedAdd[RGV.nowCommendId]->fixTime;
					fprintf(result4,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.Time,RGV.Time+RGV.NeedAdd[RGV.nowCommendId]->remainFixTime);

					RGV.NeedAdd[RGV.nowCommendId] = RGV.NeedAdd[RGV.sumCommend-1];//把可运行的最后一个与故障机器交换
					RGV.sumCommend--;
					Adjest4();
					continue;
				}
				RGV.needWait = 0;
				RGV.isShang = 1;
				RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
				//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
				fprintf(result3,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
				//已经把前一个产品时间存了   更新现在加工的那个时间
				RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
				RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
			}
			else
			{
				RGV.needWait =  1;
				RGV.isShang = 0;
				RGV.waitTime = RGV.NeedAdd[RGV.nowCommendId]->remainTime;
			}	
		}
		else if(RGV.needWait == 1 && RGV.waitTime > 0)
		{
			RGV.waitTime--;
		}
		else if(RGV.needWait == 1 && RGV.waitTime == 0)
		{
			//等待时间已经到了  则开始上料
			RGV.needWait = 0;
			RGV.isShang =  1;
			RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
			//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
			fprintf(result3,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
			//已经把前一个产品时间存了   更新现在加工的那个时间
			RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
			RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
		}
		else if(RGV.isShang ==  1 && RGV.shangTime > 0)//在上料了
		{
			RGV.shangTime--;
		}
		else if(RGV.isShang == 1 && RGV.shangTime == 0)
		{
			//上下料完成
			RGV.isShang = 0;
			RGV.isClean = 1;
			RGV.cleanTime = CleanTime;
			

			//告知那个CNC  你已经被上了新的生料
			RGV.NeedAdd[RGV.nowCommendId]->isShang = 0;
			RGV.NeedAdd[RGV.nowCommendId]->isWork = 1;
			RGV.NeedAdd[RGV.nowCommendId]->remainTime = RGV.NeedAdd[RGV.nowCommendId]->needTime;	
		}
	}
	printf("\n共完成%d个成品\n",RGV.successCount);
	printf("所有CNC一共等待了%d秒   最后成品时间为%d秒  无效时间为%d秒 最后成品出炉时工作的CNC台数为%d台\n",RGV.wT,RGV.endTime,28800-RGV.endTime,count);
	for(i = 0;i < N;++i)
		printf("%d号CNC的效率为%.2lf%%\n",CNC[i].id,(1-CNC[i].waitTime/28800.0)*100);
	return 1;
}
void ProspectRun()
{
	int i = 0, j = 0, temp = 0,item = 0;
	//刚运转，给八个都加上料
	//给1号加，把1号初始化
	for(i = 0;i < N;++i)
	{
		Print(0);
		Sleep(100);
		temp = (N-i-1)*CNC[i].shangTime;
		item = CNC[i].shangTime;
		//把这个CNC初始化为上料
		CNC[i].isWork = 1;
		CNC[i].remainTime = CNC[i].needTime;
		//把当前上料时间加进去
		CNC[i].UpTime = RGV.Time;
		//其他的需要等待当前的上料时间
		if(CNC[i].id%2 == 1)//当前编号是奇数，则还要加上走路的时间
		{
			if(CNC[i].id != 1)
			{
				temp+=(RGV.sToM*(N-i-1));
				item+=RGV.sToM;
			}
		}
		for(j =  0;j < i;++j)
			CNC[j].remainTime-=item;
		RGV.waitTime+=temp;
		RGV.Time = RGV.Time+item;
	}
	for(i = 0;i < N;++i)
	{
		if(CNC[i].remainTime < 0)  //若有些CNC在我给其他加料的时候已经完成  则会出现负数  加上这个相反数就行了
		{
			RGV.waitTime +=(CNC[i].remainTime*-1);
			CNC[i].waitTime+=(CNC[i].remainTime*-1);
		}
	}
	RGV.nowPos = 4;
	RGV.isClean = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	
	temp = 0;
	while (RGV.Time <= 28800)//28800
	{
		//Print(0);
		//Sleep(1);
		RGV.Time++;
		for (i = 0;i < N;++i)
		{
			//一定几率出故障
			if (CNC[i].isNeedFix == 0 && CNC[i].remainTime != 0 && rand() % 100 == RGV.fixNum)//之前的CNC没坏、剩余工作时间不为0，并且这一秒坏了
			{
				CNC[i].isNeedFix = 1;
				CNC[i].remainTime = 0;
			}
			if (CNC[i].isNeedFix == 0)//没坏  
			{
				if (CNC[i].isShang == 1)//如果我正在上料，因为RGC完成上料后会更新
					continue;
				--CNC[i].remainTime;//时间减1
				if (CNC[i].remainTime <= 0)//如果剩余时间为0  则发出请求
				{
					CNC[i].remainTime = 0;
					//请求上料   如果它没过来给自己加 就把waitTime加一
					Pos(12,10);
					printf("%d完成啦",CNC[i].id);
					if (CNC[i].isWork == 1)//本来在工作，但是现在没工作了
					{
						CNC[i].isWork = 0;//当前是没工作了
						//上传数据到云端
						cncTemp[BeiNum++] = &CNC[i];
					}
					//已经是没工作了，又过了一秒，则告诉RGV我等待了1秒钟
					++RGV.waitTime;
					CNC[i].waitTime++;
				}
				else
				{
					CNC[i].isWork = 1;
				}
			}
		}
		//上面的每一个完成后，会发出请求加入到命令列表中
		if(RGV.sumCommend == 0 && BeiNum == 0)//没有收到命令 云端没有命令  等待
		{
			RGV.waitTime++;
		}
		else if(RGV.sumCommend == 0 && BeiNum != 0)//当前没有命令  但是目前云端有命令了   填充命令
		{
			Adjest2();
			RGV.isTest = 1;//在准备工作时开始分配各项任务   而在工作时仅仅判断任务是否完成
			temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
			if (temp < 0)
				temp = -1 * temp;
			if(temp == 0)//和我在同一个位置  直接给上料
			{
				RGV.needMove = 1;
				RGV.moveTime = 0;
			}
			else if(temp == 1)//一个位置  则移动过去，再上料
			{
				RGV.needMove = 1;//需要移动  计算移动时间
				RGV.moveTime = RGV.sToM;
			}
			else
			{
				RGV.needMove = 1;
				RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
			}
		}
		else
		{
			//如果有命令
			//1、当前是在清洗，则时间减一  2、清洗正好完成，则调度一次   
			//3、本来没有在执行一个命令（即移动），则开始第一个命令  4、如果在执行移动命令  则移动时间减一
			//5、如果在执行等待命令，则等待时间减一
			//printf("%d\n",RGV.sumCommend);
			if(RGV.isClean == 1 && RGV.cleanTime > 0)
			{
				RGV.cleanTime--;
				RGV.isTest = 0;
			}
			else if(RGV.isClean == 1 && RGV.cleanTime <= 0)
			{
				RGV.isClean = 0;
				RGV.cleanTime = CleanTime;
				RGV.successCount++;
				RGV.sumCommend--;
				//清洗完成，判断是不是需要调整
				if(RGV.nowCommendId == RGV.sumCommend-1)//当前任务列表的任务都已经完成了
				{
					if(BeiNum == 0)//云端也没数据  则等候吧
					{
						RGV.isTest = 0;
						RGV.nowCommendId = 0;
						RGV.sumCommend = 0;
						continue;
					}
					else
					{
						Adjest2();//把备选的任务复制回来
					}
				}
				else//任务列表还有任务，则继续下一个任务
				{
					RGV.nowCommendId++;//往后继续下一个任务
				}
				RGV.isTest = 1;//在准备工作时开始分配各项任务   而在工作时仅仅判断任务是否完成
				temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					RGV.needMove = 1;
					RGV.moveTime = 0;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					RGV.needMove = 1;//需要移动  计算移动时间
					RGV.moveTime = RGV.sToM;
				}
				else
				{
					RGV.needMove = 1;
					RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
				}
			}
			else if(RGV.isTest == 0)
			{
				//本来是没有任务的  没清洗 没移动，现在给它加上任务
				RGV.isTest = 1;
				temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					RGV.needMove = 1;
					RGV.moveTime = 0;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					RGV.needMove = 1;//需要移动  计算移动时间
					RGV.moveTime = RGV.sToM;
				}
				else
				{
					RGV.needMove = 1;
					RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
				}
			}
			else if(RGV.isTest == 1)
			{
				if(RGV.needMove == 1 && RGV.moveTime > 0)
				{
					//如果正在移动 且时间大于0
					RGV.moveTime--;
				}
				else if(RGV.needMove == 1 && RGV.moveTime == 0)
				{//已经移动到了
					RGV.needMove = 0;
					RGV.nowPos =  RGV.NeedAdd[RGV.nowCommendId]->pos;
					//确定是否要等待
					if(RGV.NeedAdd[RGV.nowCommendId]->remainTime == 0)//如果已经完成  则不需要等待，直接上料  否则就等待
					{
						RGV.needWait = 0;
						RGV.isShang = 1;
						RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
						//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
						fprintf(result1,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
						RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
					}
					else
					{
						RGV.needWait =  1;
						RGV.isShang = 0;
						RGV.waitTime = RGV.NeedAdd[RGV.nowCommendId]->remainTime;
					}	
				}
				else if(RGV.needWait == 1 && RGV.waitTime > 0)
				{
					RGV.waitTime--;
				}
				else if(RGV.needWait == 1 && RGV.waitTime == 0)
				{
					//等待时间已经到了  则开始上料
					RGV.needWait = 0;
					RGV.isShang =  1;
					RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
				}
				else if(RGV.isShang ==  1 && RGV.shangTime > 0)//在上料了
				{
					RGV.shangTime--;
				}
				else if(RGV.isShang == 1 && RGV.shangTime == 0)
				{
					//上下料完成
					RGV.isShang = 0;
					RGV.isClean = 1;
					RGV.cleanTime = CleanTime;
					

					//告知那个CNC  你已经被上了新的生料
					RGV.NeedAdd[RGV.nowCommendId]->isShang = 0;
					RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
					RGV.NeedAdd[RGV.nowCommendId]->isWork = 1;
					RGV.NeedAdd[RGV.nowCommendId]->remainTime = RGV.NeedAdd[RGV.nowCommendId]->needTime;	
				}
			}
		}
	}
	printf("\n共完成%d个成品\n",RGV.successCount);
	//printf("所有CNC一共等待了%d秒,总平均等待时间为%.2lf\n",RGV.waitTime,RGV.waitTime*1.0/N);
	for(i = 0;i < N;++i)
		printf("%d号CNC的效率为%.2lf%%\n",CNC[i].id,CNC[i].waitTime/28800.0*100);
}
double Run4()
{
	int i = 0,index = 0,j = 0,temp = 0,count1 = 0,count2 = 0,item = 0,s = 0,count;
	Node node;
	struct cnc*ll;
	for(i = 0;i < N;++i)
	{
		if(CNC[i].workId == 1)//是一号刀片
		{
			temp = (N-j)*CNC[i].shangTime;//计算总等待时间
			item = CNC[i].shangTime;
			CNC[i].isWork = 1;
			CNC[i].remainTime = CNC[i].needTime;
			//把当前上料时间加进去
			CNC[i].UpTime = RGV.Time;
			//其他的需要等待当前的上料时间
			if(CNC[i].id %2 == 1)//期间可能经历了一次运动
			{
				if(CNC[i].id != 1)//是一号 则没有运动
				{
					temp += (N-j)*RGV.sToM;
					item += RGV.sToM;
				}
			}
			for(index =  0;index < i;++index)
				if(CNC[index].workId == 1)//是一号刀片，则把它的工作过剩余时间减少
					CNC[j].remainTime-=item;
			RGV.wT+=temp;
			RGV.Time = RGV.Time+item;
		}
	}
	count =  0;
	//把里面的2号刀片加到备选表里
	for(i = 0;i < N;++i)
	{
		if(CNC[i].workId == 2)
			DoubleTemp2[count1++] = &CNC[i];
		else
			DoubleTemp1[count2++] = &CNC[i];
	}
	Double2 = count1;
	Double1 = count2;
	for(i = 0;i < N;++i)
	{
		if(CNC[i].workId == 1 && CNC[i].remainTime < 0)//若有些CNC在我给其他加料的时候已经完成  则会出现负数  加上这个相反数就行了
		{
			RGV.wT +=(CNC[i].remainTime*-1);
			CNC[i].waitTime+=(CNC[i].remainTime*-1);
		}
	}
	RGV.nowPos = 4;
	RGV.isClean = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	RGV.testNum = 0;//没执行
	temp = 0;
	Adjest5();
	while (RGV.Time <= 28800)//28800
	{
		//Print(0);
		//Sleep(1);
		RGV.Time++;
		//1、先判断是否有1号完成，有的话就加入到列表1中去，2号完成也是加入到列表2去
		//2、再调度小车  
		//如果当前在清洗  则继续
		//如果没在清洗在路上，则继续
		//如果没在路上
		//如果现在有一号发出请求  找出一个方案去换料。如果没有2，就原地等待
		for (i = 0;i < N;++i)
		{
			if (CNC[i].isNeedFix == 0)//没坏  
			{
				if (CNC[i].isShang == 1)//如果我正在上料，因为RGC完成上料后会更新
				{
					CNC[i].waitTime++;
					RGV.wT++;
					continue;
				}
				if(CNC[i].isWork == 1 && CNC[i].remainTime > 0)
				{
					--CNC[i].remainTime;	
				}
				else if(CNC[i].isWork == 1 && CNC[i].remainTime <= 0)
				{
					//正好这一秒完成工作
					CNC[i].remainTime = 0;
					CNC[i].isWork = 0;
				}
				else if(CNC[i].isWork == 0)
				{
					//没有工作了  等待时间++
					CNC[i].waitTime++;
					RGV.wT++;
				}
			}
			else if(CNC[i].isNeedFix == 1 && CNC[i].remainFixTime > 1)
			{
				CNC[i].remainFixTime--;
			}
			else if(CNC[i].isNeedFix == 1 && CNC[i].remainFixTime <= 1)
			{
				//修好了  通知云端  可以安排我工作啦
				CNC[i].isNeedFix = 0;
				if(CNC[i].workId == 1)//1号刀片
				{
					DoubleTemp1[Double1++] = &CNC[i];
				}
				else
				{
					DoubleTemp2[Double2++] = &CNC[i];
				}
			}
		}

		//小车执行完两个命令后才会开始重新排序    先1后2

		if(RGV.isClean == 1 && RGV.cleanTime > 0)
		{
			RGV.cleanTime--;
			RGV.isTest = 0;
		}
		else if(RGV.isClean == 1 && RGV.cleanTime <= 0)
		{
			RGV.isClean = 0;
			RGV.cleanTime = CleanTime;
			RGV.endTime = RGV.Time;
				//计算当前工作的台数
			count = 0;
			for(i = 0;i < N;++i)
				if(CNC[i].isWork == 1)
					count++;
			RGV.successCount++;

			Adjest5();
		}
		else if(RGV.isTest == 0)
		{
			//本来是没有任务的  没清洗 没移动，现在给它加上任务
			RGV.isTest = 1;
			temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
			if (temp < 0)
				temp = -1 * temp;
			if(temp == 0)//和我在同一个位置  直接给上料
			{
				RGV.needMove = 1;
				RGV.moveTime = 0;
			}
			else if(temp == 1)//一个位置  则移动过去，再上料
			{
				RGV.needMove = 1;//需要移动  计算移动时间
				RGV.moveTime = RGV.sToM;
			}
			else
			{
				RGV.needMove = 1;
				RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
			}
		}
		else if(RGV.isTest == 1)
		{
			if(RGV.needMove == 1 && RGV.moveTime > 0)
			{
				//如果正在移动 且时间大于0
				RGV.moveTime--;
			}
			else if(RGV.needMove == 1 && RGV.moveTime == 0)
			{//已经移动到了
				RGV.needMove = 0;
				RGV.nowPos =  RGV.NeedAdd[RGV.nowCommendId]->pos;
			    //确定是否要等待
				if(RGV.NeedAdd[RGV.nowCommendId]->remainTime == 0)//如果已经完成  则不需要等待，直接上料  否则就等待
				{
					//准备上料的时候需要看是否故障

					//计算当前的这个CNC出故障的概率   判断当前的这个CNC是否出故障
					if(rand()%100 < RGV.NeedAdd[RGV.nowCommendId]->fixRand)//发生故障，则修改这台CNC的状态 并修改目前RGV中可运行的数目
					{
						//把当前编号  故障开始时间   故障结束时间
						RGV.NeedAdd[RGV.nowCommendId]->isNeedFix = 1;
						RGV.NeedAdd[RGV.nowCommendId]->remainFixTime = rand()%RGV.NeedAdd[RGV.nowCommendId]->fixTime+RGV.NeedAdd[RGV.nowCommendId]->fixTime;
						//RGV.NeedAdd[RGV.testNum-1]->remainFixTime = RGV.NeedAdd[RGV.testNum-1]->fixTime;
						fprintf(result5,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.Time,RGV.Time+RGV.NeedAdd[RGV.nowCommendId]->remainFixTime);
						if(RGV.testNum == 1)
						{
							//1号错误啦
							//把它删除
							item = 0;
							for(index = 0;index < Double1;++index)
							{
								if(DoubleTemp1[index] == RGV.NeedAdd[RGV.nowCommendId])
								{
									item = index;
									break;
								}
							}
							ll = DoubleTemp1[item];
							DoubleTemp1[item] = DoubleTemp1[Double1-1];
							Double1--;
							Adjest5();
							continue;

						}
						else
						{
							item = 0;
							for(index = 0;index < Double2;++index)
							{
								if(DoubleTemp2[index] == RGV.NeedAdd[RGV.nowCommendId])
								{
									item = index;
									break;
								}
							}
							ll = DoubleTemp2[item];
							DoubleTemp2[item] = DoubleTemp2[Double2-1];
							Double2--;
							Adjest5();
							continue;
						}
					}
					if(RGV.NeedAdd[RGV.nowCommendId]->workId == 1)
					{
						//拷贝信息到node里
						node.UpTime = RGV.NeedAdd[RGV.nowCommendId]->UpTime;
						node.id = RGV.NeedAdd[RGV.nowCommendId]->id;
						node.DownTime = RGV.Time;
						RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
					}
					else//2号工序
					{
						//如果是初始情况  则这个CNC没有加工的东西  不需要写入文件
						if(RGV.NeedAdd[RGV.nowCommendId]->isFirst == 0)
						{
							//不需要写入
							RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
							RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
							RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
							RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
						}
						else
						{
							fprintf(result4,"%d\t%d\t%d\t%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->firstid,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.NeedAdd[RGV.nowCommendId]->DownTime,RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->SUpTime,RGV.Time);
							//RGV.NeedAdd[RGV.nowCommendId]->SDownTime = RGV.Time;
							//把原来的写入以后  再把新节点情况复制过来
							RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
							RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
							RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
							RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
						}
						
					}
					RGV.needWait = 0;
					RGV.isShang = 1;
					RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
					
					//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
					RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
				}
				else
				{
					RGV.needWait =  1;
					RGV.isShang = 0;
					RGV.waitTime = RGV.NeedAdd[RGV.nowCommendId]->remainTime;
				}	
			}
			else if(RGV.needWait == 1 && RGV.waitTime > 0)
			{
				RGV.waitTime--;
			}
			else if(RGV.needWait == 1 && RGV.waitTime == 0)
			{
				//等待时间已经到了  则开始上料
				if(RGV.NeedAdd[RGV.nowCommendId]->workId == 1)
				{
					//拷贝信息到node里
					node.UpTime = RGV.NeedAdd[RGV.nowCommendId]->UpTime;
					node.id = RGV.NeedAdd[RGV.nowCommendId]->id;
					node.DownTime = RGV.Time;
					RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
				}
				else
				{
					//如果是初始情况  则这个CNC没有加工的东西  不需要写入文件
					if(RGV.NeedAdd[RGV.nowCommendId]->isFirst == 0)
					{
						RGV.NeedAdd[RGV.nowCommendId]->isFirst = 1;
						//不需要写入
						RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
						RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
						RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
						RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
					}
					else
					{
						fprintf(result4,"%d\t%d\t%d\t%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->firstid,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.NeedAdd[RGV.nowCommendId]->DownTime,RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->SUpTime,RGV.Time);
						//把原来的写入以后  再把新节点情况复制过来
						RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
						RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
						RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
						RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
					}
					
				}
				RGV.needWait = 0;
				RGV.isShang = 1;
				RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
				
				//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
				RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
			}
			else if(RGV.isShang ==  1 && RGV.shangTime > 0)//在上料了
			{
				RGV.shangTime--;
			}
			else if(RGV.isShang == 1 && RGV.shangTime == 0)
			{
				//上下料完成
				//告知那个CNC  你已经被上了新的生料
				RGV.NeedAdd[RGV.nowCommendId]->isShang = 0;
				RGV.NeedAdd[RGV.nowCommendId]->isWork = 1;
				RGV.NeedAdd[RGV.nowCommendId]->remainTime = RGV.NeedAdd[RGV.nowCommendId]->needTime;


				//确定这是几号工序
				if(RGV.testNum ==1)//1号  则要去2号工序了
				{
					RGV.isShang = 0;
					RGV.isClean = 0;
					RGV.isTest = 0;
					RGV.cleanTime = CleanTime;
					RGV.nowCommendId++;
					RGV.testNum = 2;//你要去做2号工序了
					
				}
				else
				{//2号  则开始清洗
					if(RGV.NeedAdd[RGV.nowCommendId]->isFirst == 0)
					{
						RGV.NeedAdd[RGV.nowCommendId]->isFirst = 1;
						RGV.isShang = 0;
						Adjest5();
						continue;
					}
					RGV.isShang = 0;
					RGV.isClean = 1;
					RGV.cleanTime = CleanTime;
				}
			}
		}
	}
	for(i = 0;i < N;++i)
	{
		ret2.xiao[i] = (1-CNC[i].waitTime/28800.0)*100;
	}
	ret2.waitTime = RGV.wT;
	ret2.count = RGV.successCount;
	ret2.tWu = 28800-RGV.endTime;
	return RGV.wT*0.5+(28800-RGV.endTime)*8*0.5;
}
double Run2()
{
	//先给所有的1号工作台加上生料  然后在原地等候
	//只要有一号发出请求，则去给它换料
	int i = 0,index = 0,j = 0,temp = 0,count = 0,item = 0,s = 0;
	Node node;
	
	for(i = 0;i < N;++i)
	{
		if(CNC[i].workId == 1)//是一号刀片
		{
			//j++;//
			temp = (N-j)*CNC[i].shangTime;//计算总等待时间
			item = CNC[i].shangTime;
			CNC[i].isWork = 1;
			CNC[i].remainTime = CNC[i].needTime;
			//把当前上料时间加进去
			CNC[i].UpTime = RGV.Time;
			//其他的需要等待当前的上料时间
			if(CNC[i].id %2 == 1)//期间可能经历了一次运动
			{
				if(CNC[i].id != 1)//是一号 则没有运动
				{
					temp += (N-j)*RGV.sToM;
					item += RGV.sToM;
				}
			}
			for(index =  0;index < i;++index)
				if(CNC[index].workId == 1)//是一号刀片，则把它的工作过剩余时间减少
					CNC[j].remainTime-=item;
			RGV.waitTime+=temp;
			RGV.Time = RGV.Time+item;
		}
	}
	count =  0;
	//把里面的2号刀片加到备选表里
	for(i = 0;i < N;++i)
	{
		if(CNC[i].workId == 2)
			DoubleTemp2[count++] = &CNC[i];
	}
	Double2 = count;
	for(i = 0;i < N;++i)
	{
		if(CNC[i].workId == 1 && CNC[i].remainTime < 0)//若有些CNC在我给其他加料的时候已经完成  则会出现负数  加上这个相反数就行了
		{
			RGV.waitTime +=(CNC[i].remainTime*-1);
			CNC[i].waitTime+=(CNC[i].remainTime*-1);
		}
	}
	RGV.nowPos = 4;
	RGV.isClean = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	RGV.testNum = 0;//没执行
	temp = 0;
	while (RGV.Time <= 28800)//28800
	{
		//Print(0);
		//Sleep(1);
		RGV.Time++;
		//1、先判断是否有1号完成，有的话就加入到列表1中去，2号完成也是加入到列表2去
		//2、再调度小车  
		//如果当前在清洗  则继续
		//如果没在清洗在路上，则继续
		//如果没在路上
		//如果现在有一号发出请求  找出一个方案去换料。如果没有2，就原地等待
		for (i = 0;i < N;++i)
		{
			//一定几率出故障
			if (CNC[i].isNeedFix == 0 && CNC[i].remainTime != 0 && rand() % 100 == RGV.fixNum)//之前的CNC没坏、剩余工作时间不为0，并且这一秒坏了
			{
				CNC[i].isNeedFix = 1;
				CNC[i].remainTime = 0;
			}
			if (CNC[i].isNeedFix == 0)//没坏  
			{
				if (CNC[i].isShang == 1)//如果我正在上料，因为RGC完成上料后会更新
					continue;
				--CNC[i].remainTime;//时间减1
				if (CNC[i].remainTime <= 0)//如果剩余时间为0  则发出请求
				{
					CNC[i].remainTime = 0;
					//请求上料   如果它没过来给自己加 就把waitTime加一
					if(CNC[i].workId == 1)
					{
						//Pos(12,10);
						//printf("%d完成啦，是%d号刀片",CNC[i].id,CNC[i].workId);
					}
					else
					{
						//Pos(12,15);
						//printf("%d完成啦，是%d号刀片",CNC[i].id,CNC[i].workId);
					}
					if (CNC[i].isWork == 1)//本来在工作，但是现在没工作了
					{
						CNC[i].isWork = 0;//当前是没工作了
						//RGV.NeedAdd[RGV.sumCommend++] = &CNC[i];//往我后面加就是了，当我检测到本来是没命令到接受命令，则对当前列表排序
						
						if(CNC[i].workId == 1)
							DoubleTemp1[Double1++] = &CNC[i];
						else
							DoubleTemp2[Double2++] = &CNC[i];
							
					}
					//已经是没工作了，又过了一秒，则告诉RGV我等待了1秒钟
					++RGV.waitTime;
					CNC[i].waitTime++;
				}
				else
				{
					CNC[i].isWork = 1;
				}
			}
		}

		//小车执行完两个命令后才会开始重新排序    先1后2

		
		//上面的每一个完成后，会发出请求加入到命令列表中
		
		if(RGV.sumCommend == 0 && RGV.isClean == 0 && Double1 != 0 && Double2 != 0)//出现二号，我才去找1号
		{
			Adjest3();//有2号 也有1号
		}
		else if(RGV.sumCommend == 0 && RGV.isClean == 0)//没有收到命令 且云端没有空闲的二号  等待
		{
			RGV.waitTime++;
		}
		else
		{
			//如果有命令
			//1、当前是在清洗，则时间减一  2、清洗正好完成，则调度一次   
			//3、本来没有在执行一个命令（即移动），则开始第一个命令  4、如果在执行移动命令  则移动时间减一
			//5、如果在执行等待命令，则等待时间减一
			//printf("%d\n",RGV.sumCommend);
			
			//这里证明我命令列表是有命令的，执行那个？  
			//如果在清洗，啥也不做
			if(RGV.isClean == 1 && RGV.cleanTime > 0)
			{
				RGV.cleanTime--;
				RGV.isTest = 0;
			}
			else if(RGV.isClean == 1 && RGV.cleanTime <= 0)
			{
				RGV.isClean = 0;
				RGV.cleanTime = CleanTime;
				RGV.successCount++;
				RGV.sumCommend--;
				//清洗完成了   肯定是完成了2号工序  所以需要向云端再读一个数了
				RGV.endTime = RGV.Time;
				//计算当前工作的台数
				count = 0;
				for(i = 0;i < N;++i)
					if(CNC[i].isWork == 1)
						count++;
				//查过云端后如果没有两个配套任务  则等待  如果有  请求分配				
				if(Double2 != 0 && Double1 != 0)
				{
					Adjest3();
				}
				else
				{
					RGV.nowCommendId = 0;
					RGV.sumCommend = 0;
					RGV.testNum = 0;
					continue;
				}
				RGV.isTest = 1;//在准备工作时开始分配各项任务   而在工作时仅仅判断任务是否完成
				RGV.testNum = 1;
				temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					RGV.needMove = 1;
					RGV.moveTime = 0;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					RGV.needMove = 1;//需要移动  计算移动时间
					RGV.moveTime = RGV.sToM;
				}
				else
				{
					RGV.needMove = 1;
					RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
				}
			}
			else if(RGV.isTest == 0)
			{
				//本来是没有任务的  没清洗 没移动，现在给它加上任务
				RGV.isTest = 1;
				temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					RGV.needMove = 1;
					RGV.moveTime = 0;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					RGV.needMove = 1;//需要移动  计算移动时间
					RGV.moveTime = RGV.sToM;
				}
				else
				{
					RGV.needMove = 1;
					RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
				}
			}
			else if(RGV.isTest == 1)
			{
				if(RGV.needMove == 1 && RGV.moveTime > 0)
				{
					//如果正在移动 且时间大于0
					RGV.moveTime--;
				}
				else if(RGV.needMove == 1 && RGV.moveTime == 0)
				{//已经移动到了
					RGV.needMove = 0;
					RGV.nowPos =  RGV.NeedAdd[RGV.nowCommendId]->pos;
					//确定是否要等待
					if(RGV.NeedAdd[RGV.nowCommendId]->remainTime == 0)//如果已经完成  则不需要等待，直接上料  否则就等待
					{
						if(RGV.NeedAdd[RGV.nowCommendId]->workId == 1)
						{
							//拷贝信息到node里
							
							//RGV.NeedAdd[RGV.nowCommendId]->DownTime = RGV.Time;
							node.UpTime = RGV.NeedAdd[RGV.nowCommendId]->UpTime;
							node.id = RGV.NeedAdd[RGV.nowCommendId]->id;
							node.DownTime = RGV.Time;
							RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
						}
						else
						{
							//如果是初始情况  则这个CNC没有加工的东西  不需要写入文件
							if(RGV.NeedAdd[RGV.nowCommendId]->isFirst == 0)
							{
								//不需要写入
								RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
								RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
								RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
								RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
							}
							else
							{
								fprintf(result2,"%d\t%d\t%d\t%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->firstid,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.NeedAdd[RGV.nowCommendId]->DownTime,RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->SUpTime,RGV.Time);
								//RGV.NeedAdd[RGV.nowCommendId]->SDownTime = RGV.Time;
								//把原来的写入以后  再把新节点情况复制过来
								RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
								RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
								RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
								RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
							}
							
						}
						RGV.needWait = 0;
						RGV.isShang = 1;
						RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
						
						//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
						RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
					}
					else
					{
						RGV.needWait =  1;
						RGV.isShang = 0;
						RGV.waitTime = RGV.NeedAdd[RGV.nowCommendId]->remainTime;
					}	
				}
				else if(RGV.needWait == 1 && RGV.waitTime > 0)
				{
					RGV.waitTime--;
				}
				else if(RGV.needWait == 1 && RGV.waitTime == 0)
				{
					//等待时间已经到了  则开始上料
					if(RGV.NeedAdd[RGV.nowCommendId]->workId == 1)
					{
						//拷贝信息到node里
						
						//RGV.NeedAdd[RGV.nowCommendId]->DownTime = RGV.Time;
						node.UpTime = RGV.NeedAdd[RGV.nowCommendId]->UpTime;
						node.id = RGV.NeedAdd[RGV.nowCommendId]->id;
						node.DownTime = RGV.Time;
						RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
					}
					else
					{
						//如果是初始情况  则这个CNC没有加工的东西  不需要写入文件
						if(RGV.NeedAdd[RGV.nowCommendId]->isFirst == 0)
						{
							RGV.NeedAdd[RGV.nowCommendId]->isFirst = 1;
							//不需要写入
							RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
							RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
							RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
							RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
						}
						else
						{
							fprintf(result2,"%d\t%d\t%d\t%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->firstid,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.NeedAdd[RGV.nowCommendId]->DownTime,RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->SUpTime,RGV.Time);
							//RGV.NeedAdd[RGV.nowCommendId]->SDownTime = RGV.Time;
							//把原来的写入以后  再把新节点情况复制过来
							RGV.NeedAdd[RGV.nowCommendId]->firstid = node.id;
							RGV.NeedAdd[RGV.nowCommendId]->UpTime = node.UpTime;
							RGV.NeedAdd[RGV.nowCommendId]->DownTime = node.DownTime;
							RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
						}
						
					}
					RGV.needWait = 0;
					RGV.isShang = 1;
					RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
					
					//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
					RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
				}
				else if(RGV.isShang ==  1 && RGV.shangTime > 0)//在上料了
				{
					RGV.shangTime--;
				}
				else if(RGV.isShang == 1 && RGV.shangTime == 0)
				{
					//上下料完成
					//告知那个CNC  你已经被上了新的生料
					RGV.NeedAdd[RGV.nowCommendId]->isShang = 0;
					RGV.NeedAdd[RGV.nowCommendId]->isWork = 1;
					RGV.NeedAdd[RGV.nowCommendId]->remainTime = RGV.NeedAdd[RGV.nowCommendId]->needTime;


					//确定这是几号工序
					if(RGV.testNum ==1)//1号  则要去2号工序了
					{
						RGV.isShang = 0;
						RGV.isClean = 0;
						RGV.isTest = 0;
						RGV.cleanTime = CleanTime;
						RGV.nowCommendId++;
						RGV.testNum = 2;//你要去做2号工序了
						/*
						if(RGV.sumCommend > 1)//之前给你分配了两道工序  则比较自己的和云端的
						{
							//有任务  先计算自己到下一个2号工序所花费的时间。再从云端找出最好的一个  用自己的时间和那个时间比较
							temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
							if (temp < 0)
								temp = -1 * temp;
							if(temp == 0)//和我在同一个位置  直接给上料
							{
								RGV.needMove = 1;
								RGV.moveTime = 0;
							}
							else if(temp == 1)//一个位置  则移动过去，再上料
							{
								RGV.needMove = 1;//需要移动  计算移动时间
								RGV.moveTime = RGV.sToM;
							}
							else
							{
								RGV.needMove = 1;
								RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
							}
						}
						*/
					}
					else
					{//2号  则开始清洗
						if(RGV.NeedAdd[RGV.nowCommendId]->isFirst == 0)
						{
							RGV.NeedAdd[RGV.nowCommendId]->isFirst = 1;
							Adjest3();
							continue;
						}
						RGV.isShang = 0;
						RGV.isClean = 1;
						RGV.cleanTime = CleanTime;
					}
				}
			}
		}
	}
	//printf("\n共完成%d个成品\n",RGV.successCount);
	//printf("所有CNC一共等待了%d秒   最后成品时间为%d秒  无效时间为%d秒 最后成品出炉时工作的CNC台数为%d台\n",RGV.waitTime,RGV.endTime,28800-RGV.endTime,count);
	//for(i = 0;i < N;++i)
	//	printf("%d号CNC的效率为%.2lf%%\n",CNC[i].id,CNC[i].waitTime/28800.0*100);
	for(i = 0;i < N;++i)
	{
		ret2.xiao[i] = (1-CNC[i].waitTime/28800.0)*100;
	}
	ret2.waitTime = RGV.waitTime;
	ret2.count = RGV.successCount;
	ret2.tWu = 28800-RGV.endTime;
	return RGV.waitTime*0.5+(28800-RGV.endTime)*8*0.5;
}

void RepeatRun()
{
	int i = 0, j = 0, temp = 0,item = 0;
	//刚运转，给八个都加上料
	//给1号加，把1号初始化
	for(i = 0;i < N;++i)
	{
		Print(0);
		Sleep(100);
		temp = (N-i-1)*CNC[i].shangTime;//计算总等待时间
		item = CNC[i].shangTime;
		CNC[i].isWork = 1;
		CNC[i].remainTime = CNC[i].needTime;
		//把当前上料时间加进去
		CNC[i].UpTime = RGV.Time;
		//其他的需要等待当前的上料时间
		if(CNC[i].id %2 == 1)//期间可能经历了一次运动
		{
			if(CNC[i].id != 1)//是奇数  但是不是一号 表示它需要额外加走路时间
			{
				temp += (N-i-1)*RGV.sToM;
				item += RGV.sToM;
			}
		}
		for(j =  0;j < i;++j)
			CNC[j].remainTime-=item;
		RGV.waitTime+=temp;
		RGV.Time = RGV.Time+item;//下一次开始的时间就是目前的时间加上这次上料的时间  以及路上的时间
	}
	for(i = 0;i < N;++i)
	{
		if(CNC[i].remainTime < 0)//若有些CNC在我给其他加料的时候已经完成  则会出现负数  加上这个相反数就行了
		{
			RGV.waitTime +=(CNC[i].remainTime*-1);
			CNC[i].waitTime+=(CNC[i].remainTime*-1);
		}
	}
	RGV.nowPos = 1;
	RGV.isClean = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.sumCommend = 0;
	temp = 0;
	while (RGV.Time <= 28800)//28800
	{
		//Print(0);
		//Sleep(1);
		RGV.Time++;
		//1、每台CNC是否出问题
		//2、计算CNN是否在工作  若没坏，且（有熟料了，或者没熟料但是剩余时间为0，则发出需求指令）
		//3、计算当前这一秒哪些会发出请求
		//4、做出调整，决定给谁上料
		//5、上料的时候判断CNC是否是有熟料的
		//若当前在执行命令列表的一个命令，且已经完成系列工作，则命令后移
		//如果在清洗，则继续清洗，直到结束
		for (i = 0;i < N;++i)
		{
			//一定几率出故障
			if (CNC[i].isNeedFix == 0 && CNC[i].remainTime != 0 && rand() % 100 == RGV.fixNum)//之前的CNC没坏、剩余工作时间不为0，并且这一秒坏了
			{
				CNC[i].isNeedFix = 1;
				CNC[i].remainTime = 0;
			}
			if (CNC[i].isNeedFix == 0)//没坏  
			{
				if (CNC[i].isShang == 1)//如果我正在上料，因为RGC完成上料后会更新
					continue;
				--CNC[i].remainTime;//时间减1
				if (CNC[i].remainTime <= 0)//如果剩余时间为0  则发出请求
				{
					CNC[i].remainTime = 0;
					//请求上料   如果它没过来给自己加 就把waitTime加一
					Pos(12,10);
					printf("%d完成啦",CNC[i].id);
					if (CNC[i].isWork == 1)//本来在工作，但是现在没工作了
					{
						CNC[i].isWork = 0;//当前是没工作了
						Require(&CNC[i],1);
					}
					//已经是没工作了，又过了一秒，则告诉RGV我等待了1秒钟
					++RGV.waitTime;
					CNC[i].waitTime++;
				}
				else
				{
					CNC[i].isWork = 1;
				}
			}
		}
		//上面的每一个完成后，会发出请求加入到命令列表中
		if(RGV.sumCommend == 0)//没有收到命令  等待
		{
			;
			//RGV.waitTime++;
		}
		else
		{
			//如果有命令
			//1、当前是在清洗，则时间减一  2、清洗正好完成，则调度一次   
			//3、本来没有在执行一个命令（即移动），则开始第一个命令  4、如果在执行移动命令  则移动时间减一
			//5、如果在执行等待命令，则等待时间减一
			//printf("%d\n",RGV.sumCommend);
			if(RGV.isClean == 1 && RGV.cleanTime > 0)
			{
				RGV.cleanTime--;
				RGV.isTest = 0;
			}
			else if(RGV.isClean == 1 && RGV.cleanTime <= 0)
			{
				RGV.isClean = 0;
				RGV.cleanTime = CleanTime;
				Adjest1();
				RGV.isTest = 1;//在准备工作时开始分配各项任务   而在工作时仅仅判断任务是否完成
				temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					RGV.needMove = 1;
					RGV.moveTime = 0;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					RGV.needMove = 1;//需要移动  计算移动时间
					RGV.moveTime = RGV.sToM;
				}
				else
				{
					RGV.needMove = 1;
					RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
				}
			}
			else if(RGV.isTest == 0)
			{
				//本来是没有任务的  没清洗 没移动，现在给它加上任务
				RGV.isTest = 1;
				temp = RGV.NeedAdd[RGV.nowCommendId]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					RGV.needMove = 1;
					RGV.moveTime = 0;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					RGV.needMove = 1;//需要移动  计算移动时间
					RGV.moveTime = RGV.sToM;
				}
				else
				{
					RGV.needMove = 1;
					RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
				}
			}
			else if(RGV.isTest == 1)
			{
				if(RGV.needMove == 1 && RGV.moveTime > 0)
				{
					//如果正在移动 且时间大于0
					RGV.moveTime--;
				}

				else if(RGV.needMove == 1 && RGV.moveTime == 0)
				{//已经移动到了
					if(RGV.NeedAdd[RGV.nowCommendId]->workId == 1)//1号刀片
					{
						RGV.NeedAdd[RGV.nowCommendId]->DownTime = RGV.Time;
					}
					else
					{
						//如果是刚开始的时候，当前时间则为
						RGV.NeedAdd[RGV.nowCommendId]->SUpTime = RGV.Time;
					}
					RGV.needMove = 0;
					RGV.nowPos =  RGV.NeedAdd[RGV.nowCommendId]->pos;
					//确定是否要等待
					if(RGV.NeedAdd[RGV.nowCommendId]->remainTime == 0)//如果已经完成  则不需要等待，直接上料  否则就等待
					{
						RGV.needWait = 0;
						RGV.isShang = 1;
						RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
						//这个时候  就是把当前的熟料下来的时间  即需要写入文件了
						fprintf(result1,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
						RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
						RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
					}
					else
					{
						RGV.needWait =  1;
						RGV.isShang = 0;
						RGV.waitTime = RGV.NeedAdd[RGV.nowCommendId]->remainTime;
					}	
				}
				else if(RGV.needWait == 1 && RGV.waitTime > 0)
				{
					RGV.waitTime--;
				}
				else if(RGV.needWait == 1 && RGV.waitTime == 0)
				{
					//等待时间已经到了  则开始上料
					RGV.needWait = 0;
					RGV.isShang =  1;
					RGV.NeedAdd[RGV.nowCommendId]->isShang = 1;
					fprintf(result1,"%d\t%d\t%d\n",RGV.NeedAdd[RGV.nowCommendId]->id,RGV.NeedAdd[RGV.nowCommendId]->UpTime,RGV.Time);
					RGV.shangTime = RGV.NeedAdd[RGV.nowCommendId]->shangTime;
					RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
				}
				else if(RGV.isShang ==  1 && RGV.shangTime > 0)//在上料了
				{
					RGV.shangTime--;
				}
				else if(RGV.isShang == 1 && RGV.shangTime == 0)
				{
					//上下料完成
					RGV.isShang = 0;
					RGV.isClean = 1;
					RGV.cleanTime = CleanTime;
					RGV.successCount++;

					//告知那个CNC  你已经被上了新的生料
					RGV.NeedAdd[RGV.nowCommendId]->isShang = 0;
					//RGV.NeedAdd[RGV.nowCommendId]->UpTime = RGV.Time;
					RGV.NeedAdd[RGV.nowCommendId]->isWork = 1;
					RGV.NeedAdd[RGV.nowCommendId]->remainTime = RGV.NeedAdd[RGV.nowCommendId]->needTime;

					RGV.NeedAdd[RGV.nowCommendId] = RGV.NeedAdd[RGV.sumCommend - 1];//把最后一个移到这里来，然后把最后一个删掉  总命令数减1
					RGV.NeedAdd[RGV.sumCommend - 1] = NULL;
					RGV.sumCommend--;
					
				}
			}
		}
	}
	printf("\n共完成%d个成品\n",RGV.successCount);
	//printf("所有CNC一共等待了%d秒,总平均等待时间为%.2lf\n",RGV.waitTime,RGV.waitTime*1.0/N);
	printf("所有CNC一共等待了%d秒\n",RGV.waitTime);
	for(i = 0;i < N;++i)
		printf("%d号CNC的效率为%.2lf%%\n",CNC[i].id,100-CNC[i].waitTime/28800.0*100);
}
void Adjest5()
{
	long int sum = 32768;
	int i = 0,temp,index1,index2,p = 0,j,nowPos = RGV.nowPos;
	if(Double1+Double2 == 0)//全故障了 那我算出哪个机器最先修好，并移动过去
	{
		;
	}
	else
	{
		index1 = 0;
		index2 = 0;
		for(i = 0;i < Double1;++i)
		{
			for(j = 0;j < Double2;++j)
			{
				//计算去i  再去j的时间
				//计算去一号的时间
				temp = DoubleTemp1[i]->pos - nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					p = DoubleTemp1[i]->shangTime+DoubleTemp1[i]->remainTime;
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					if(DoubleTemp1[i]->remainTime - RGV.sToM >=0)
					{
						p = DoubleTemp1[i]->shangTime + DoubleTemp1[i]->remainTime;
					}
					else
					{
						p = DoubleTemp1[i]->shangTime + RGV.sToM;
					}
					
				}
				else
				{
					if(DoubleTemp1[i]->remainTime - RGV.sToM - RGV.mToM*(temp - 1) >=0)
					{
						p = DoubleTemp1[i]->shangTime + DoubleTemp1[i]->remainTime;
					}
					else
					{
						p = DoubleTemp1[i]->shangTime + RGV.sToM + RGV.mToM*(temp - 1);
					}
				}
				nowPos = DoubleTemp1[i]->pos;
				//p是去第一个位置的时间 如果这个时间大于第二个的剩余时间 
				//把p加上移动的时间   如果你给1号加料用了70秒  我本来只需要30秒就完成了
				temp = DoubleTemp2[j]->pos - nowPos; //需要去的那个CNC与自己的距离
				if (temp < 0)
					temp = -1 * temp;
				if(temp == 0)//和我在同一个位置  直接给上料
				{
					if(p > DoubleTemp2[j]->remainTime)
					{
						//我先完成  等小车完成可以立即加料
						p += DoubleTemp2[j]->shangTime;
					}
					else
					{
						//CNC还没结束   那我p的时间应该就是它的剩余时间加下料时间
						p = DoubleTemp2[j]->remainTime+DoubleTemp2[j]->shangTime;
					}
				}
				else if(temp == 1)//一个位置  则移动过去，再上料
				{
					p+=RGV.sToM;//到的时候是p
					if(p > DoubleTemp2[j]->remainTime)
					{
						//我先完成  等小车完成可以立即加料
						p += DoubleTemp2[j]->shangTime;
					}
					else
					{
						//CNC还没结束   那我p的时间应该就是它的剩余时间加下料时间
						p = DoubleTemp2[j]->remainTime+DoubleTemp2[j]->shangTime;
					}					
				}
				else
				{
					p+=RGV.sToM + RGV.mToM*(temp - 1);
					if(p > DoubleTemp2[j]->remainTime)
					{
						//我先完成  等小车完成可以立即加料
						p += DoubleTemp2[j]->shangTime;
					}
					else
					{
						//CNC还没结束   那我p的时间应该就是它的剩余时间加下料时间
						p = DoubleTemp2[j]->remainTime+DoubleTemp2[j]->shangTime;
					}
				}

				if(p < sum)
				{
					sum = p;
					index1 = i;
					index2 = j;
				}
			}
		}
		//找到最好的两个了
		RGV.sumCommend = 2;
		RGV.nowCommendId = 0;
		RGV.NeedAdd[0] = DoubleTemp1[index1];
		RGV.NeedAdd[1] = DoubleTemp2[index2];

	}
	RGV.testNum = 1;
	//上面保证给出序列
	temp = RGV.NeedAdd[RGV.testNum-1]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
	if (temp < 0)
		temp = -1 * temp;
	if(temp == 0)//和我在同一个位置  直接给上料
	{
		RGV.needMove = 1;
		RGV.moveTime = 0;
	}
	else if(temp == 1)//一个位置  则移动过去，再上料
	{
		RGV.needMove = 1;//需要移动  计算移动时间
		RGV.moveTime = RGV.sToM;
	}
	else
	{
		RGV.needMove = 1;
		RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
	}
	RGV.isTest = 1;//开始工作
	RGV.testNum = 1;
}
void Adjest4()
{
	long int sum = 32768;
	struct cnc* llll;
	//struct cnc* cncTemp[N],*Cnc[N];//保存当前的最后排列   计算完后把它复制回来
	int i = 0,temp,index,p = 0,j = 0;
	//对目前可以工作的CNC进行全排列  算出最少的一个排个序就行了
	if(RGV.sumCommend == 0)//全故障了 那我算出哪个机器最先修好，并移动过去
	{
		temp = 32768;
		index = 0;
		for(i = 0;i < N;++i)
		{
			if(RGV.NeedAdd[i]->remainFixTime < temp)
			{
				index = i;
				temp = RGV.NeedAdd[i]->remainFixTime;//这个最先修好
			}
		}
		llll = RGV.NeedAdd[index];
		RGV.NeedAdd[index] = RGV.NeedAdd[0];
		RGV.NeedAdd[0] = RGV.NeedAdd[index];
	}
	else
	{
		index = 0;
		for(i = 0;i < RGV.sumCommend;++i)
		{
			p = 0;
			//计算去一号的时间
			temp = RGV.NeedAdd[i]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
			if (temp < 0)
				temp = -1 * temp;
			if(temp == 0)//和我在同一个位置  直接给上料
			{
				p = RGV.NeedAdd[i]->shangTime+RGV.NeedAdd[i]->remainTime;
			}
			else if(temp == 1)//一个位置  则移动过去，再上料
			{
				if(RGV.NeedAdd[i]->remainTime - RGV.sToM >=0)
				{
					p = RGV.NeedAdd[i]->shangTime + RGV.NeedAdd[i]->remainTime;
				}
				else
				{
					p = RGV.NeedAdd[i]->shangTime + RGV.sToM;
				}
				
			}
			else
			{
				if(RGV.NeedAdd[i]->remainTime - RGV.sToM - RGV.mToM*(temp - 1) >=0)
				{
					p = RGV.NeedAdd[i]->shangTime + RGV.NeedAdd[i]->remainTime;
				}
				else
				{
					p = RGV.NeedAdd[i]->shangTime + RGV.sToM + RGV.mToM*(temp - 1);
				}
			}
			if(p < sum)
			{
				sum = p;
				index = i;
			}
		}
		//找到最好的一个了
		llll = RGV.NeedAdd[index];
		RGV.NeedAdd[index] = RGV.NeedAdd[0];
		RGV.NeedAdd[0] = llll;
	}
	

	//上面保证给出序列
	temp = RGV.NeedAdd[0]->pos - RGV.nowPos; //需要去的那个CNC与自己的距离
	if (temp < 0)
		temp = -1 * temp;
	if(temp == 0)//和我在同一个位置  直接给上料
	{
		RGV.needMove = 1;
		RGV.moveTime = 0;
	}
	else if(temp == 1)//一个位置  则移动过去，再上料
	{
		RGV.needMove = 1;//需要移动  计算移动时间
		RGV.moveTime = RGV.sToM;
	}
	else
	{
		RGV.needMove = 1;
		RGV.moveTime = RGV.sToM + RGV.mToM*(temp - 1);//需要移动这么久
	}
	RGV.isTest = 1;//开始工作
	
}
void Adjest3()
{
	//肯定可以保证有两个命令   我需要全排列，计算两个序列全排列之后的值  以小的为主
	struct cnc* Item1[N],*Item2[N],*TT[N];
	long int sum = 32768;
	int flag = 0;
	int i = 0;
	if(Double1 ==0 || Double2 == 0)
		return;
	for(i = 0;i < Double1;++i)
		Item1[i] = DoubleTemp1[i];
	for(i = 0;i < Double2;++i)
		Item2[i] = DoubleTemp2[i];
	Calculate2(TT,&sum,0,Double1,1);
	//完成后  给安排两个命令
	RGV.NeedAdd[0] = TT[0];
	RGV.NeedAdd[1] = TT[1];
	//把这两个命令从原来那里删掉
	for(i = 0;i < Double1;++i)
		if(DoubleTemp1[i] == TT[0])
		{
			//找到了这次处理的是哪一个
			DoubleTemp1[i] = DoubleTemp1[Double1-1];
			DoubleTemp1[Double1-1] = NULL;
		}
	Double1--;
	for(i = 0;i < Double2;++i)
		if(DoubleTemp2[i] == TT[1])
		{
			//找到了这次处理的是哪一个
			DoubleTemp2[i] = DoubleTemp2[Double2-1];
			DoubleTemp2[Double2-1] = NULL;
		}
	Double2--;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 2;
	RGV.isClean = 0;
	RGV.isShang = 0;
	RGV.isTest = 0;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.testNum = 1;
}
int JueDuiZhi(int a,int b)
{
	if(a > b)
		return a-b;
	else
		return b-a;
}
void FindMin(struct cnc* cncTemp[],struct cnc* cnc1Temp[],long int *sum)
{
	//计算这种cnc1Temp情况下的等待时间  如果比前面的好  则更新cncTemp
	long int temp = 0;
	int nowPos = RGV.nowPos,item = 0;//分别计算从当前位置到下一个位置需要等待的时间
	int index = 0,time = 0;
	int num = 0;//目前在等待的有多少个
	for(index = 0;index < RGV.sumCommend;++index)
	{
		//计算目前位置移动到下一个地点的时间
		item = cnc1Temp[index]->pos - nowPos;
		if(item < 0)
			item = -1*item;
		if(item == 0)
		{
			//当前可以直接上料，则除了该CNC，其余的都乘以这个上料时间
			temp = temp + (RGV.sumCommend-index-1)*(cnc1Temp[index]->shangTime + CleanTime);
		}
		else if(temp == 1)
		{
			//需要移动一个单位 
			temp = temp + (RGV.sumCommend-index)*RGV.sToM+(RGV.sumCommend-index-1)*(cnc1Temp[index]->shangTime+ CleanTime);
		}
		else 
		{
			temp = temp + (RGV.sumCommend-index)*(RGV.sToM + RGV.mToM*(item - 1))+(RGV.sumCommend-index-1)*(cnc1Temp[index]->shangTime+ CleanTime);
		}
		nowPos = cnc1Temp[index]->pos;
	}
	if(temp < *sum)
	{
		*sum = temp;
		for(index = 0;index < RGV.sumCommend;++index)
		{
			cncTemp[index] = cnc1Temp[index];
			//printf("%d ",cncTemp[index]->id);
		}
		//printf("\t%d\t\n",*sum);
	}
}
void CalculateTime(struct cnc* cncTemp[],struct cnc* cnc1Temp[],long int *sum)
{
	//计算
	;
}
/*
void CalculateTime(struct cnc* cncTemp[],struct cnc* cnc1Temp[],long int *sum)
{
	//计算自己按照这个顺序给它们上一次料需要的时间
	long int temp = 0,s  = 0;
	int nowPos = RGV.nowPos,item = 0,xx,mm;
	int index = 0,i = 0;
	struct cnc PP[N];
	//把cnc1中的值拷贝进来
	for(index = 0;index < RGV.sumCommend;++index)
	{
		PP[index].isWork = cnc1Temp[index]->isWork;//是否工作
		PP[index].remainTime = cnc1Temp[index]->remainTime;
		PP[index].shangTime = cnc1Temp[index]->shangTime;
		PP[index].pos = cnc1Temp[index]->pos;
	}

	for(index = 0;index < RGV.sumCommend;++index)
	{
		item = JueDuiZhi(nowPos,PP[index].pos);//得到位置差
		//计算需要等待的时间
		//temp 是总等待时间   s是当前花费的时间 因为需要花费时间来确定是否要等待
		xx = 0;
		
		if(item == 0)
		{
			//当前可以直接上料，则除了该CNC，其余的都乘以这个上料时间
			mm = 0;
			xx = 0;
			s+=0;
		}
		else if(temp == 1)
		{
			//需要移动一个单位 
			mm = RGV.sToM;
			xx = (RGV.sumCommend-index)*RGV.sToM;
			s+=RGV.sToM;
		}
		else 
		{
			mm = RGV.sToM + RGV.mToM*(item - 1);
			xx = (RGV.sumCommend-index)*(RGV.sToM + RGV.mToM*(item - 1));
			s += RGV.sToM + RGV.mToM*(item - 1);
		}
		nowPos = PP[index].pos;
		//需要看一下所有的CNC期间是否等待了
		xx = 0;
		temp+=mm*(RGV.sumCommend-index);//总共有这么多机器等待了mm秒
		for(i = index;i < RGV.sumCommend;++i)
		{
			if(PP[i].isWork == 1)//本来这个在工作
			{
				if(PP[i].remainTime > mm)//你要50秒结束 我30秒到了
				{
					PP[i].remainTime -= mm;//剩下20秒工作时间
				}
				else
				{
					//你移动要40秒  我10秒钟就完成了工作
					temp+=(mm-PP[i].remainTime);
					//temp+=(mm-PP[i].remainTime);
					PP[i].isWork = 0;
					PP[i].remainTime = 0 ;
				}
			}
			else
			{//本来你没工作  那就直接加上吧
				temp += mm;
				PP[i].remainTime = 0;
			}
		}
		//temp += xx;//更新走路带来的等待时间
		xx = 0;
		if(PP[index].remainTime == 0)//可以直接上料
		{
			//那就无需等待
			xx = 0;
		}
		else
		{
			//我在等待期间，其余的都要等 或者把工作时间减少
			xx = PP[index].remainTime;
			s+=PP[index].remainTime;
			PP[index].remainTime = 0;
		}
		temp+=xx;
		//在多少秒后  其他CNC的状态	
		for(i = index;i < RGV.sumCommend;++i)
		{
			if(PP[i].isWork == 1)//本来这个在工作
			{
				if(PP[i].remainTime > xx)//你要50秒结束 我30秒到了
				{
					PP[i].remainTime -= xx;//剩下20秒工作时间
				}
				else
				{
					//你移动要40秒  我10秒钟就完成了工作
					temp+=(xx-PP[i].remainTime);
					PP[i].isWork = 0;
					PP[i].remainTime = 0 ;
				}
			}
			else
			{//本来你没工作  那就直接加上吧
				temp += xx;
				PP[i].remainTime = 0;
			}
		}
		
		//开始上下料了  计算上下料时间
		xx = PP[index].shangTime;
		for(i = index;i < RGV.sumCommend;++i)
		{
			if(PP[i].isWork == 1)//本来这个在工作
			{
				if(PP[i].remainTime > xx)//你要50秒结束 我30秒到了
				{
					PP[i].remainTime -= xx;//剩下20秒工作时间
				}
				else
				{
					//你移动要40秒  我10秒钟就完成了工作
					temp+=(xx-PP[i].remainTime);
					PP[i].isWork = 0;
					PP[i].remainTime = 0 ;
				}
			}
			else
			{//本来你没工作  那就直接加上吧
				temp += xx;
				PP[i].remainTime = 0;
			}
		}

		//上下料完成了，清洗时间   index及以前的都不用管了
		xx = CleanTime;
		for(i = index+1;i < RGV.sumCommend;++i)
		{
			if(PP[i].isWork == 1)//本来这个在工作
			{
				if(PP[i].remainTime > xx)//你要50秒结束 我30秒到了
				{
					PP[i].remainTime -= mm;//剩下20秒工作时间
				}
				else
				{
					//你移动要40秒  我10秒钟就完成了工作
					temp+=(xx-PP[i].remainTime);
					PP[i].isWork = 0;
					PP[i].remainTime = 0 ;
				}
			}
			else
			{//本来你没工作  那就直接加上吧
				temp += xx;
				PP[i].remainTime = 0;
			}
		}
		//清洗结束了
	}
	if(temp < *sum)
	{
		*sum = temp;
		for(index = 0;index < RGV.sumCommend;++index)
		{
			//printf("%d ",cnc1Temp[index]->id);
			cncTemp[index] = cnc1Temp[index];
		}
		for(index = 0 ;index < RGV.sumCommend;++index)
			printf("%d ",cncTemp[index]->id);
		printf("\n");
	}
	
}
*/
void Calculate3(struct cnc* cncTemp[], struct cnc* cnc1Temp[],long int *sum,int l,int r)
{
	int i;
	
	if (l == r - 1)
	{
		//for (i = 0;i < r;++i)
		//	printf("%d",cncTemp[i]->id);
		//putchar('\n');
		//这里一次的全排列已经完成，计算当前的cncTemp从前往后需要等待的时间
		
		//计算当前方案的时间并与上一个比较
		CalculateTime(cncTemp,cnc1Temp,sum);
		return;
	}
	else
	{
		for (i = l;i < r;++i)
		{
			swap(cnc1Temp[i], cnc1Temp[l]);
			Calculate3(cncTemp,cnc1Temp, sum, l + 1, r);
			swap(cnc1Temp[i], cnc1Temp[l]);
		}
	}
}
void Calculate2(struct cnc* cncTemp[],long int *sum,int l,int r,int flag)
{
	int i;	
	long int temp = 0;
	int nowPos = RGV.nowPos,item = 0;//分别计算从当前位置到下一个位置需要等待的时间
	int index = 0,time = 0,x = 0;
	int num = 0;//目前在等待的有多少个
	int len;
	if (l == r - 1)
	{
		if(flag == 1)//表明第一次进来，则把第二个序列做全排列
		{
			Calculate2(cncTemp,sum,0,Double2,2);
		}
		else
		{
			if(Double1 <= Double2)
				len = Double1;
			else
				len = Double2;
			//要计算了；把两个序列中按照较短间隔算
			for(index = 0;index < len;++index)//需要算几对
			{
				//先去1中的第一个地方  再去2中的第一个地方
				item = JueDuiZhi(DoubleTemp1[index]->pos,nowPos);
				if(item == 0)
				{
					temp = temp + ((len-index-1)*2+1)*(DoubleTemp1[index]->shangTime + CleanTime);
				}
				else if(temp == 1)
				{
					//需要移动一个单位 
					temp = temp + ((len-index)*2)*RGV.sToM+((len-index-1)*2+1)*(DoubleTemp1[index]->shangTime+ CleanTime);
				}
				else 
				{
					temp = temp + ((len-index)*2)*(RGV.sToM + RGV.mToM*(item - 1))+((len-index-1)*2+1)*(DoubleTemp1[index]->shangTime+ CleanTime);
				}
				nowPos = DoubleTemp1[index]->pos;//到达了1号位置  再计算去对应的B位置需要多久
				item = JueDuiZhi(DoubleTemp2[index]->pos,nowPos);
				if(item == 0)
				{
					temp = temp + ((len-index-1)*2)*(DoubleTemp1[index]->shangTime + CleanTime);
				}
				else if(temp == 1)
				{
					//需要移动一个单位 
					temp = temp + ((len-index-1)*2+1)*RGV.sToM+((len-index-1)*2)*(DoubleTemp2[index]->shangTime+ CleanTime);
				}
				else 
				{
					temp = temp + ((len-index-1)*2)*(RGV.sToM + RGV.mToM*(item - 1))+((len-index)*2+1)*(DoubleTemp1[index]->shangTime+ CleanTime);
				}
				nowPos = DoubleTemp2[index]->pos;//移到了对应的第二个位置
			}
			if(temp < *sum)
			{
				*sum = temp;
				//把对应的写进去
				for(index = 0;index < len;++index)
				{
					cncTemp[2*index] = DoubleTemp1[index];
					cncTemp[2*index+1] = DoubleTemp2[index];
				}
			}
		}
		return;
	}
	else
	{
		for (i = l;i < r;++i)
		{
			if(flag == 1)
			{
				swap(DoubleTemp1[i], DoubleTemp1[l]);
				Calculate2(cncTemp, sum, l + 1, r,flag);
				swap(DoubleTemp1[i], DoubleTemp1[l]);
			}
			else
			{
				swap(DoubleTemp2[i], DoubleTemp2[l]);
				Calculate2(cncTemp, sum, l + 1, r,flag);
				swap(DoubleTemp2[i], DoubleTemp2[l]);
			}
		}
	}
}
void Adjest2()
{
	//把备选的那个序列中的拷贝过来，并做一次全排列
	int i;
	long int sum = 32768;
	struct cnc* cnc1Temp[N],*cnc2Temp[N];
	if(BeiNum == 0)//目前没有发出指令
	{
		//直接让它等待
		RGV.nowCommendId = 0;
		RGV.sumCommend = 0;
		return;
	}
	for(i = 0;i < BeiNum;++i)
	{
		cnc2Temp[i] = cncTemp[i];
		cnc1Temp[i] = cncTemp[i];
	}
	Calculate1(cnc2Temp,cnc1Temp, &sum,0,BeiNum);//跑全排列得到最优的一个  然后把新的复制回来  最好的保存在cnc2Temp里面
	for(i = 0;i < BeiNum;++i)
		RGV.NeedAdd[i] = cnc2Temp[i];
	RGV.sumCommend = BeiNum;
	RGV.nowCommendId = 0;
	BeiNum = 0;
}
void Adjest1()
{
	//做一个全排列得到当前最优的一个方案
	//先把当前的需求列表拷贝过来。
	int i;
	long int sum = 32768;
	struct cnc* cnc1Temp[N];
	if(RGV.sumCommend  == 1)
		return;
	//struct cnc 
	RGV.nowCommendId = 0;
	for (i = 0;i < RGV.sumCommend;++i)
	{
		cncTemp[i] = RGV.NeedAdd[i];//拷贝信息
		cnc1Temp[i] = RGV.NeedAdd[i];
	}
	//计算一个最佳的
	Calculate1(cncTemp,cnc1Temp, &sum,0,RGV.sumCommend);//跑全排列得到最优的一个  然后把新的复制回来
	for(i = 0;i < RGV.sumCommend;++i)
		RGV.NeedAdd[i] = cncTemp[i];
}

void Calculate1(struct cnc* cncTemp[],struct cnc* cnc1Temp[], long int *sum, int l, int r)
{
	int i;
	
	if (l == r - 1)
	{
		//for (i = 0;i < r;++i)
		//	printf("%d",cncTemp[i]->id);
		//putchar('\n');
		//这里一次的全排列已经完成，计算当前的cncTemp从前往后需要等待的时间
		FindMin(cncTemp,cnc1Temp,sum);
		return;
	}
	else
	{
		for (i = l;i < r;++i)
		{
			swap(cnc1Temp[i], cnc1Temp[l]);
			Calculate1(cncTemp,cnc1Temp, sum, l + 1, r);
			swap(cnc1Temp[i], cnc1Temp[l]);
		}
	}
}
void swap(struct cnc*x, struct cnc*y)
{
	struct cnc t;
	t = *x;
	*x = *y;
	*y = t;
}
void Pos(int x, int y)//设置光标位置
{//要注意这里的x和y与我们数组的x和y是反的
	COORD pos;
	HANDLE hOutput;
	pos.X = x;
	pos.Y = y;
	hOutput = GetStdHandle(STD_OUTPUT_HANDLE);//返回标准的输入、输出或错误的设备的句柄，也就是获得输入、输出/错误的屏幕缓冲区的句柄
	SetConsoleCursorPosition(hOutput, pos);
}
void Print(int d)
{
	int i = 0;
	if(d == 0)
	{
		system("cls");
		//初始页面
		Pos(RGV.nowPos*4,2);
		printf("■");
		for(i = 0;i < N;++i)
		{
			if(CNC[i].id % 2 == 0)
			{
				//偶数行
				Pos((i+2)/2*4,1);
				Space;
				Pos((i+2)/2*4,1);
				if(CNC[i].isWork == 0)//没工作
					printf("■");
				else
					printf("□");
			}
			else
			{
				Pos((i+2)/2*4,3);
				Space;
				Pos((i+2)/2*4,3);
				if(CNC[i].isWork == 0)//没工作
					printf("■");
				else
					printf("□");
			}
		}
	}
	else
	{
		//初始页面
		printf("\n\n\n");
		//printf("■");
		for(i = 0;i < N;++i)
		{
			if(CNC[i].id % 2 == 0)
			{
				//偶数行
				if(CNC[i].isWork == 0)//没工作
					printf("■   ");
				else
					printf("□   ");
			}
		}
		printf("\n");
		for(i = 0;i < RGV.nowPos;++i)
			printf("  ");
		printf("■\n");
		for(i = 0;i < N;++i)
		{
			if(CNC[i].id % 2 == 1)
			{
				//偶数行
				if(CNC[i].isWork == 0)//没工作
					printf("■   ");
				else
					printf("□   ");
			}
		}
		printf("\n");
	}
}
void Require(struct cnc *c,int id)
{
	//如果目前发出需求的CNC不在这里面  添加进来  否则无视
	//直接加到后面
	if(id == 1)
	{
		RGV.NeedAdd[RGV.sumCommend++] = c;
		return;
	}
	//否则，证明是第二种
	if(RGV.sumCommend == 0)
	{
		//前面的命令序列已经完成，则重新Adjest
		Adjest2();
	}
}
void Ques3Double()
{
	double Value = 99999,tempValue;
	int id1[N],Id[N];
	int z,x,c,v,b,n,m,l,i,flag = 0;
	FILE *result3Last;
	/*
	result4 = fopen("result4.txt","w");
	result5 = fopen("result5.txt","w");
	Init4(id1);//此时已经是一种刀片分配了  计算一次
	tempValue = Run4();
	fclose(result4);
	fclose(result5);
	*/
	
	//把所有刀片的组合弄出来
	for(z = 0;z < 2;++z)
	{
		id1[0] = z+1;
		for(x = 0;x < 2;++x)
		{
			id1[1] = x+1;
			for(c = 0;c < 2;++c)
			{
				id1[2] = c+1;
				for(v = 0;v < 2;++v)
				{
					id1[3] = v+1;
					for(b = 0;b < 2;++b)
					{
						id1[4] = b+1;
						for(n = 0;n < 2;++n)
						{
							id1[5] = n+1;
							for(m = 0;m < 2;++m)
							{
								id1[6] = m+1;
								for(l = 0;l < 2;++l)
								{
									id1[7] = l+1;
									flag = 0;
									for(i = 0;i < N;++i)
										if(id1[i] == 1)
											flag++;
									if(flag <= 1 || flag >= N-1)
										continue;
									result4 = fopen("result4.txt","w");
									result5 = fopen("result5.txt","w");
									Init4(id1);//此时已经是一种刀片分配了  计算一次
									tempValue = Run4();
									fclose(result4);
									fclose(result5);
									if(tempValue < Value)//比之前更好   把当前的方案记录下来
									{
										//写入文件
										result4 = fopen("result4.txt","r");
										result3Last = fopen("result3Last.txt","w");
										CopyFile1(result4,result3Last);
										fclose(result4);
										fclose(result3Last);
										result5 = fopen("result5.txt","r");
										result3Last = fopen("result3Last1.txt","w");
										CopyFile1(result5,result3Last);
										fclose(result5);
										fclose(result3Last);
										for(i = 0;i < N;++i)
										{
											Id[i] = id1[i];
											ret1.xiao[i] = ret2.xiao[i];
										}
										ret1.waitTime = ret2.waitTime;
										ret1.count = ret2.count;
										ret1.tWu = ret2.tWu;
										Value = tempValue;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	result3Last = fopen("result3Last.txt","a");
	fprintf(result3Last,"\n八种刀片的分配方案为：");
	for(i = 0;i < N;++i)
		fprintf(result3Last,"%d\t",Id[i]);
	fprintf(result3Last,"\n八个CNC的效率：");
	for(i = 0;i < N;++i)
		fprintf(result3Last,"%lf%%\t",ret1.xiao[i]);
	fprintf(result3Last,"\n此方案下的成品数目：%d\t总等待时间为：%d\t无效时间为：%d\n",ret1.count,ret1.waitTime,ret1.tWu);
	fclose(result3Last);
	
}
void Ques3()
{
	//单刀片调度方案
	result3 = fopen("result3.txt","w");
	result4 = fopen("result4.txt","w");
	Init3();
	Run3();
	fclose(result3);
	fclose(result4);
}
void Init3()
{
	int i = 0;
	int sToM, mToM, oneTime, firstTime, secondTime, oddTime, evenTime, cleanTime;
	FILE *fp = fopen("data.txt", "r");
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);//从静止到移动一个单元时间 移动状态移动一个单元 CNN加工一道工序 CNN加工第一道工序时间  CNN加工第二道工序时间  奇数CNN上下料时间 偶数CNN上下料时间 RGV清洗时间
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	RGV.isClean = 1;
	RGV.remainTime = 0;
	RGV.cleanTime = 1;
	CleanTime = cleanTime;
	RGV.Time = 0;
	RGV.successCount = 0;
	RGV.fixNum = 101;//表示不会出故障
	RGV.nowState = 0;//静止
	RGV.sToM = sToM;
	RGV.mToM = mToM;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.isTest = 0;
	RGV.isShang = 0;
	RGV.isWait = 0;
	RGV.shangTime = 0;
	RGV.moveTime = 0;
	RGV.waitTime = 0;
	RGV.nowPos = 1;
	RGV.SumWaitTime = 0;
	RGV.wT = 0;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	for (i = 0;i < N;++i)
	{
		CNC[i].id = i + 1;
		CNC[i].pos = (i + 2) / 2;
		CNC[i].isWork = 0;
		CNC[i].haveOrNo = 0;
		CNC[i].needTime = oneTime;
		CNC[i].remainTime = 0;
		CNC[i].workId = 1;
		
		CNC[i].waitTime = 0;
		CNC[i].isShang = 0;
		CNC[i].UpTime = 0;
		CNC[i].DownTime = 0;
		CNC[i].SUpTime = 0;
		CNC[i].SDownTime = 0;
		CNC[i].firstid = 0;
		CNC[i].remainFixTime = 0;
		CNC[i].successNum = 0;

		CNC[i].fixRand = 1;
		CNC[i].isNeedFix = 0;
		CNC[i].fixTime = 600;
		

		if (CNC[i].id % 2 == 0)//偶数
		{
			CNC[i].workId =  2;
			CNC[i].shangTime = evenTime;
		}
		else
		{
			CNC[i].shangTime = oddTime;
		}
		//CNC[i].workId = Num[i];
		RGV.NeedAdd[i] = &CNC[i];
		RGV.sumCommend++;
	}
	fclose(fp);
	Double1 = 0;
	Double2 = 0;
}
void Ques2()
{
	double Value = 99999,tempValue;
	int id1[N],Id[N];
	int z,x,c,v,b,n,m,l,i,flag = 0;
	FILE *result2Last;
	result2 = fopen("result2.txt","w");
	//把所有刀片的组合弄出来
	for(z = 0;z < 2;++z)
	{
		id1[0] = z+1;
		for(x = 0;x < 2;++x)
		{
			id1[1] = x+1;
			for(c = 0;c < 2;++c)
			{
				id1[2] = c+1;
				for(v = 0;v < 2;++v)
				{
					id1[3] = v+1;
					for(b = 0;b < 2;++b)
					{
						id1[4] = b+1;
						for(n = 0;n < 2;++n)
						{
							id1[5] = n+1;
							for(m = 0;m < 2;++m)
							{
								id1[6] = m+1;
								for(l = 0;l < 2;++l)
								{
									id1[7] = l+1;
									flag = 0;
									for(i = 0;i < N;++i)
										if(id1[i] == 1)
											flag++;
									if(flag <= 1 || flag >= N-1)
										continue;
									result2 = fopen("result2.txt","w");
									Init2(id1);//此时已经是一种刀片分配了  计算一次
									tempValue = Run4();
									fclose(result2);
									if(tempValue < Value)//比之前更好   把当前的方案记录下来
									{
										//写入文件
										result2 = fopen("result2.txt","r");
										result2Last = fopen("result2Last.txt","w");
										CopyFile1(result2,result2Last);
										fclose(result2);
										fclose(result2Last);
										for(i = 0;i < N;++i)
										{
											Id[i] = id1[i];
											ret1.xiao[i] = ret2.xiao[i];
										}
										ret2.waitTime = ret1.waitTime;
										ret1.count = ret2.count;
										ret1.tWu = ret2.tWu;
										Value = tempValue;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	result2Last = fopen("result2Last.txt","a");
	fprintf(result2Last,"\n八种刀片的分配方案为：");
	for(i = 0;i < N;++i)
		fprintf(result2Last,"%d\t",Id[i]);
	fprintf(result2Last,"\n八个CNC的效率：");
	for(i = 0;i < N;++i)
		fprintf(result2Last,"%lf%%\t",ret1.xiao[i]);
	fprintf(result2Last,"\n此方案下的成品数目：%d\t总等待时间为：%d\t无效时间为：%d\n",ret1.count,ret1.waitTime,ret1.tWu);
	fclose(result2Last);
}
void CopyFile1(FILE *fp1,FILE *fp2)
{
	char c;
	while((c = fgetc(fp1)) != EOF)
	{
		fputc(c,fp2);
	}
}
void Init4(int *Num)
{
	int i = 0;
	int sToM, mToM, oneTime, firstTime, secondTime, oddTime, evenTime, cleanTime;
	FILE *fp = fopen("data.txt", "r");
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);//从静止到移动一个单元时间 移动状态移动一个单元 CNN加工一道工序 CNN加工第一道工序时间  CNN加工第二道工序时间  奇数CNN上下料时间 偶数CNN上下料时间 RGV清洗时间
	//printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",sToM,mToM,oneTime,firstTime,secondTime,oddTime,evenTime,cleanTime);
	//fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	//fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	RGV.isClean = 1;
	RGV.remainTime = 0;
	RGV.cleanTime = 1;
	CleanTime = cleanTime;
	RGV.Time = 0;
	RGV.successCount = 0;
	RGV.fixNum = 101;//表示不会出故障
	RGV.nowState = 0;//静止
	RGV.sToM = sToM;
	RGV.mToM = mToM;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.isTest = 0;
	RGV.isShang = 0;
	RGV.isWait = 0;
	RGV.shangTime = 0;
	RGV.moveTime = 0;
	RGV.waitTime = 0;
	RGV.nowPos = 1;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	RGV.wT = 0;
	for (i = 0;i < N;++i)
	{
		CNC[i].id = i + 1;
		CNC[i].pos = (i + 2) / 2;
		CNC[i].isWork = 0;
		CNC[i].haveOrNo = 0;
		CNC[i].needTime = oneTime;
		CNC[i].remainTime = 0;
		CNC[i].workId = 1;
		CNC[i].isNeedFix = 0;
		CNC[i].fixTime = 0;
		CNC[i].waitTime = 0;
		CNC[i].isShang = 0;
		CNC[i].UpTime = 0;
		CNC[i].DownTime = 0;
		CNC[i].SUpTime = 0;
		CNC[i].SDownTime = 0;
		CNC[i].firstid = 0;
		CNC[i].isFirst = 1;
		CNC[i].fixRand = 1;
		CNC[i].isNeedFix = 0;
		CNC[i].fixTime = 600;


		if (CNC[i].id % 2 == 0)//偶数
		{
			CNC[i].workId =  2;
			CNC[i].shangTime = evenTime;
		}
		else
		{
			CNC[i].shangTime = oddTime;
		}
		//CNC[i].workId = Num[i];
		RGV.NeedAdd[i] = &CNC[i];
		RGV.sumCommend++;
	}
	fclose(fp);
	Double1 = 0;
	Double2 = 0;
}
void Init2(int * Num)
{
	int i = 0;
	int sToM, mToM, oneTime, firstTime, secondTime, oddTime, evenTime, cleanTime;
	FILE *fp = fopen("data.txt", "r");
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);//从静止到移动一个单元时间 移动状态移动一个单元 CNN加工一道工序 CNN加工第一道工序时间  CNN加工第二道工序时间  奇数CNN上下料时间 偶数CNN上下料时间 RGV清洗时间
	//printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",sToM,mToM,oneTime,firstTime,secondTime,oddTime,evenTime,cleanTime);
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	RGV.isClean = 1;
	RGV.remainTime = 0;
	RGV.cleanTime = 1;
	CleanTime = cleanTime;
	RGV.Time = 0;
	RGV.successCount = 0;
	RGV.fixNum = 101;//表示不会出故障
	RGV.nowState = 0;//静止
	RGV.sToM = sToM;
	RGV.mToM = mToM;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.isTest = 0;
	RGV.isShang = 0;
	RGV.isWait = 0;
	RGV.shangTime = 0;
	RGV.moveTime = 0;
	RGV.waitTime = 0;
	RGV.nowPos = 1;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	for (i = 0;i < N;++i)
	{
		CNC[i].id = i + 1;
		CNC[i].pos = (i + 2) / 2;
		CNC[i].isWork = 0;
		CNC[i].haveOrNo = 0;
		CNC[i].needTime = oneTime;
		CNC[i].remainTime = 0;
		CNC[i].workId = 1;
		CNC[i].isNeedFix = 0;
		CNC[i].fixTime = 0;
		CNC[i].waitTime = 0;
		CNC[i].isShang = 0;
		CNC[i].UpTime = 0;
		CNC[i].DownTime = 0;
		CNC[i].SUpTime = 0;
		CNC[i].SDownTime = 0;
		CNC[i].firstid = 0;
		
		if (CNC[i].id % 2 == 0)//偶数
		{
			//CNC[i].workId =  2;
			CNC[i].shangTime = evenTime;
		}
		else
		{
			CNC[i].shangTime = oddTime;
		}
		CNC[i].workId = Num[i];
		RGV.NeedAdd[i] = &CNC[i];
		RGV.sumCommend++;
	}
	fclose(fp);
	Double1 = 0;
	Double2 = 0;
}
void Ques1()
{//智能调度算法
	result3 = fopen("result1.txt","w");
	result4 = fopen("result4.txt","w");
	Init3();
	Run5();
	fclose(result3);
	fclose(result4);


	/*
	result1 = fopen("result1.txt","w");
	Init1();
	Print(0);

	RepeatRun();
	//ProspectRun();
	fclose(result1);
	*/
}
void Init1()
{
	//把RGV初始化
	//把每个CNC初始化
	int i = 0;
	int sToM, mToM, oneTime, firstTime, secondTime, oddTime, evenTime, cleanTime;
	FILE *fp = fopen("data.txt", "r");
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);//从静止到移动一个单元时间 移动状态移动一个单元 CNN加工一道工序 CNN加工第一道工序时间  CNN加工第二道工序时间  奇数CNN上下料时间 偶数CNN上下料时间 RGV清洗时间
	//printf("%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",sToM,mToM,oneTime,firstTime,secondTime,oddTime,evenTime,cleanTime);
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	fscanf(fp, "%d%d%d%d%d%d%d%d", &sToM, &mToM, &oneTime, &firstTime, &secondTime, &oddTime, &evenTime, &cleanTime);
	RGV.isClean = 1;
	RGV.remainTime = 0;
	RGV.cleanTime = 1;
	CleanTime = cleanTime;
	RGV.Time = 0;
	RGV.successCount = 0;
	RGV.fixNum = 101;//表示不会出故障
	RGV.nowState = 0;//静止
	RGV.sToM = sToM;
	RGV.mToM = mToM;
	RGV.needMove = 0;
	RGV.needWait = 0;
	RGV.isTest = 0;
	RGV.isShang = 0;
	RGV.isWait = 0;
	RGV.shangTime = 0;
	RGV.moveTime = 0;
	RGV.waitTime = 0;
	RGV.nowPos = 1;
	RGV.nowCommendId = 0;
	RGV.sumCommend = 0;
	for (i = 0;i < N;++i)
	{
		CNC[i].id = i + 1;
		CNC[i].pos = (i + 2) / 2;
		CNC[i].isWork = 0;
		CNC[i].haveOrNo = 0;
		CNC[i].needTime = oneTime;
		CNC[i].remainTime = 0;
		CNC[i].workId = 1;
		CNC[i].isNeedFix = 0;
		CNC[i].fixTime = 0;
		CNC[i].waitTime = 0;
		CNC[i].isShang = 0;
		CNC[i].UpTime = 0;
		CNC[i].DownTime = 0;
		if (CNC[i].id % 2 == 0)//偶数
			CNC[i].shangTime = evenTime;
		else
			CNC[i].shangTime = oddTime;
		RGV.NeedAdd[i] = &CNC[i];
		RGV.sumCommend++;
	}
	fclose(fp);
}
