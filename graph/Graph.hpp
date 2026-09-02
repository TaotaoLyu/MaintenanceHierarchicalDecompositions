#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <random>
#include <unordered_set>
#include <set>
#include "AlgoProfile.hpp"
typedef std::pair<uint32_t, uint32_t> EdgT;
// EdgT=(u,v)
typedef std::vector<EdgT> EdgListT;
// typedef std::vector<std::vector<uint32_t> > AdjListT;

// std::vector<std::pair<uint32_t, uint32_t >> removeedg;
class Graph
{
public:
	Graph(const std::string &filename)
		: edge_num_(0),
		  node_num_(0)
	{
		AlgoProfile::GetInstance()->SetFilename(filename);
		auto s1 = AlgoProfile::CaptureClock();
		std::fstream fs(filename);
		std::cout << filename << "\n";
		uint32_t u, v;

		while (fs >> u >> v)
		{
			edge_num_++;
			edge_.insert(std::make_pair(std::min(u, v), std::max(u, v)));
			node_num_ = std::max(node_num_, std::max(u, v));
			// if ( rand() % 100000 == 10000 && removeedg.size() < 10)
			// if (/*rand() % 5 == 1 && */removeedg.size() < 56)
			// {
			// 	printf("edge: %d  %d\n",u,v);
			// 	removeedg.push_back(std::make_pair(std::min(u, v), std::max(u, v)));
			// }
		}
		productRemoveEdge();
		node_num_++;
		g_.resize(node_num_);
		deg_.resize(node_num_);
		for (auto pq : edge_)
		{
			g_[pq.first].push_back(pq.second);
			g_[pq.second].push_back(pq.first);
		}
		auto s2 = AlgoProfile::CaptureClock();
		AlgoProfile::GetInstance()->SetIOTime(s1, s2);
		AlgoProfile::GetInstance()->SetEdgeNum(edge_num_);


		CalDegree();
		int tmp=0;
		for(auto e:deg_)
			if(e!=0) tmp++;		
		AlgoProfile::GetInstance()->SetVertexNum(tmp);
		printf("input graph success\n");
		edge_.clear();
		std::cout.flush();
	}
	void productRemoveEdge()
	{
		std::vector<std::pair<int, int>> edt(edge_.begin(), edge_.end());
		std::random_device rd;
		std::mt19937 g(rd());

		// 打乱顺序
		std::shuffle(edt.begin(), edt.end(), g);

		// 选取前10个（如果不够10个就取全部）
		size_t n = std::min<size_t>(10, edt.size());
		removeedg=std::vector<std::pair<uint32_t, uint32_t>>(edt.begin(), edt.begin() + n);

		// 输出结果（可选）
		// printf("selected edge:\n");
		// for (auto &p : removeedg)
		// {
		// 	std::cout << p.first << ", " << p.second << std::endl;
		// }
	}
	std::vector<std::pair<uint32_t, uint32_t>> &GetRd() { return removeedg; }

	void CalDegree()
	{
		for (uint32_t i = 0; i < node_num_; ++i)
			deg_[i] = (uint32_t)g_[i].size();
	}
	const std::vector<uint32_t> &GetDeg()
	{
		return deg_;
	}
	void InsertEdge(uint32_t u, uint32_t v)
	{
		// if (edge_.count(std::make_pair(std::min(u, v), std::max(u, v))))
		// 	return;

		deg_[u]++, g_[u].push_back(v);
		deg_[v]++, g_[v].push_back(u);
		edge_num_++;
	}
	void RemoveEdge(uint32_t u, uint32_t v)
	{
		// if (edge_.count(std::make_pair(std::min(u, v), std::max(u, v))) == 0)
		// 	return;

		for (auto p : {u, v})
		{
			uint32_t q = u + v - p;
			deg_[p]--;
			for (int i = 0; i < g_[p].size(); i++)
				if (g_[p][i] == q)
				{
					std::swap(g_[p][i], g_[p][g_[p].size() - 1]);
					g_[p].pop_back();
					break;
				}
		}
		edge_num_--;
		// edge_.erase(std::make_pair(std::min(u, v), std::max(u, v)));
	}

	uint32_t GetNodeNum() { return node_num_; }
	uint32_t GetEdgeNum() { return edge_num_; }
	const std::vector<uint32_t> &operator[](uint32_t x) { return g_[x]; }
	~Graph() = default;

public:
	std::vector<std::vector<uint32_t>> g_;
	std::vector<uint32_t> deg_; //[deg,node_id]
	std::set<std::pair<uint32_t, uint32_t>> edge_;
	uint32_t node_num_;
	uint32_t edge_num_;
	std::vector<std::pair<uint32_t, uint32_t>> removeedg;
};