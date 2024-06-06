#include"../include/PThread.h"
#include"../include/Fiber.h"
#include<iostream>
void test_Thread(){

	std::cout << "thread_test " << std::endl;

}


int main(){

	Sylar::PThread thread_(&test_Thread,"thread_1");
	return 0;
}
