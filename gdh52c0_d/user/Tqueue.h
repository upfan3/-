#ifndef  __QUEUE_H
#define  __QUEUE_H

#include "valtype.h"
#include <string.h>


template<class T>
class Queue
{
private:
	
	
	

public:
	T *base;// 指向底层存储数组的指针（队列的实际数据缓冲区
  u16 read;
  u16 write;
  u16 maxsize;
	explicit Queue();
  explicit Queue(u16 _size);
	virtual ~Queue();
	bool push(const T& value);
	bool pop();
	T& top();// 获取队首元素（非const版本）
	const T& top()const;
	u16 size()const;// 获取当前队列中的元素个数
	void clear();
	void destory();

	bool IsFull()const;
	bool IsEmpty()const;
//   protected:
};


template<class T>
Queue<T>::~Queue()
{
	destory();
}
 
template<class T>
void Queue<T>::destory()
{
	if (base != NULL)
	{
		delete[] base;
		base = NULL;
	}
}
 
template<class T>
void Queue<T>::clear()
{
	read = 0;
	write = 0;
	
}
 
template<class T>
// write > read：写指针在 read 指针 “后面”（未绕环），此时队列数据个数 = write - read（直接计算差值）；
// write < read：写指针已绕环到 read 指针 “前面”（如 write 在索引 0，read 在索引 3），此时队列数据个数 = (write + maxsize) - read（需补全环形的长度）
u16 Queue<T>::size()const
{
	return (write - read + maxsize) % maxsize;
}
 
template<class T>
const T& Queue<T>::top()const
{
	return base[read];
}
 
template<class T>
T& Queue<T>::top()
{
	return base[read];
}
 
template<class T>
bool Queue<T>::IsFull()const
{
	return (write + 1) % maxsize == read;
}
 
template<class T>
bool Queue<T>::IsEmpty()const
{
	return read == write;
}
 
template<class T>
Queue<T>::Queue()
{
//	T *tmp = new T[12];
//	assert_param(tmp != NULL);
//	base = tmp;
//	maxsize=128;
//	memset(base, 0, maxsize*sizeof(T));
//	read = 0;
//	write = 0;
}
 

template<class T>
Queue<T>::Queue(u16 _size)
{
	//T *tmp = new T[_size];
	 T *tmp=(T*)pvPortMalloc(_size*sizeof(T));
	
	assert_param(tmp != NULL);
	base = tmp;
	maxsize=_size;
	memset(base, 0, maxsize*sizeof(T));
	read = 0;
	write = 0;
}
 


template<class T>
bool Queue<T>::push(const T& value)
{
	if (IsFull())
	{
		//cout << "the queue is already full,can not push anymore!" << endl;
		return false;
	}
	base[write] = value;//将元素存入写指针指向的位置
	write = (write + 1) % maxsize; //写指针循环递增（处理队列环绕）
	return true;
}
 
template<class T>
bool Queue<T>::pop()
{
	if (IsEmpty())
	{
//		cout << "the queue is already empty,it can not pop anymore!" << endl;
		return false;
	}
	read = (read + 1) % maxsize;
	return true;
}



typedef struct {
	uint32_t gpio_periph;
	uint32_t pin;
} Ts_gpio ;

typedef struct  {
uint8_t nvic_irq; 
uint8_t nvic_irq_pre_priority;
uint8_t nvic_irq_sub_priority;

} Ts_nvic;
//定义了一个 “接收两个通用指针参数、返回通用指针” 的函数指针类型
typedef void *(*PTRFUN1)(void *,void *); 
typedef u8 (*PTRFUN)(void *);

#endif

