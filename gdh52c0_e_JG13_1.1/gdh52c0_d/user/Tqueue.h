#ifndef  __QUEUE_H
#define  __QUEUE_H

#include "valtype.h"
#include <string.h>


template<class T>
class Queue
{
private:
	
	
	

public:
	T *base;
  u16 read;
  u16 write;
  u16 maxsize;
	explicit Queue();
  explicit Queue(u16 _size);
	virtual ~Queue();
	bool push(const T& value);
	bool pop();
	T& top();
	const T& top()const;
	u16 size()const;
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
	base[write] = value;
	write = (write + 1) % maxsize;
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

typedef void *(*PTRFUN1)(void *,void *); 
typedef u8 (*PTRFUN)(void *);

#endif

