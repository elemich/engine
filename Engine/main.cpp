#include "win32.h"

void TestVector()
{
	TFixedVector<int,4> fvi;
	for(int i=0;i<4;i++)
		fvi[i]=i+1;

	const int* p=fvi;

	for(int i=0;i<4;i++)
		printf("%d\n",p[i]);

	printf("-end vector test-\n");
}

void TestString()
{
	printf("-end string test-\n");
};


void TestList()
{
	TDLList<int> list;

	for(int i=0;i<10;i++)
		list+=i+1;
	for(int i=0;i>-10;i--)
		list-=i+1;

	list.IReset(true);

	do{printf("%d\n",list.Cur()->_data);}while(list++);

	int i=18;
	TDLList<int>::NODE* n=list.FindElementNode(i);

	

	printf("-end list test-\n");
}

int main()
{
	AppInterface *app = new App();

	app->Init();

	
	//TestString();
	//TestVector();
	//TestList();
	//TestSplitter();

	app->AppLoop();

	

	return 0;
}