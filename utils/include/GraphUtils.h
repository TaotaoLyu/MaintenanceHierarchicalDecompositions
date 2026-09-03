#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <omp.h>
#include <atomic>
#include <vector>


typedef long long LL;
typedef std::vector<int> VI;
typedef std::vector<VI> VVI;


template <class T>
inline T my_compare_swap(T* ptr, T old_val, T new_val)
{
#if defined(__linux__)
	return __sync_val_compare_and_swap(ptr, old_val, new_val);
#else
	T t;
#pragma omp critical(swap)
	{
		t = *ptr;
		if (t == old_val)
			*ptr = new_val;
	}
	return t;
#endif
}

template <class T>
inline T my_fetch_add(T* ptr, T val)
{
#if defined(__linux__)
	return __sync_fetch_and_add(ptr, val);
#else
	T t;
#pragma omp critical
	{
		t = *ptr;
		*ptr += val;
	}
	return t;
#endif
}

struct WaitFreeRankDSU 
{
public:
	WaitFreeRankDSU(int n, VI& rank) : rank(rank) { init(n); }

	int find(int x);
	void init(int n);
	void joint(int u, int v);
	inline int get_pivot(int v)
	{
		return find(v);
	}

private:
	WaitFreeRankDSU();
	int n_;
	VI& rank;

	VI pa;
};



