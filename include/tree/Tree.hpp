#pragma once
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <iomanip>
#include <queue>
#include <chrono>
#include <iostream>
#include <mutex>
#include <memory>

#include "Graph.hpp"
#include "Trine.hpp"
#include "CohesiveModel.hpp"
#include "GraphUtils.h"
#include "AlgoProfile.hpp"


namespace Tree
{

	double get_time_s(hclock::time_point s, hclock::time_point t);

	struct TreeNode
	{
	public:
		int k = -1;	 // trine
		int pa = -1; // parent
		int tid = -1;
		std::vector<int> vertex_;
		std::vector<int> child_;
	};

	class Tree
	{
	public:
		Tree(const std::string &filename, int threadnum = 32)
			: g_(filename),
			  model_(new Trine(g_))
		{
			model_->Init(); // cal trine
			SetThreadNum(threadnum);
		}
		Tree(const Graph &pt)
			: g_(pt),
			  model_(new Trine(g_))
		{

			model_->Init();
		}

		void Build(); // readlly build

		void Merge(TreeNode *dst, TreeNode *src);

		void InsertEdge(const Graph::EdgeList &edg,bool flag=true,bool flag1=true);

		void RemoveEdge(const Graph::EdgeList &edg, bool flag=true);

		bool check();
		void SetThreadNum(int num);
	private:
		
		void MoveToNewTid(std::vector<int> &changever, std::vector<int> &old_tid, std::vector<int> &old_level);
		void AddChildren();
		std::pair<int, int> GenrateTask(int v);
		void PrintTree();
		void CreateNewNode(const std::vector<uint32_t> &change_ver);
		void CreateMoveTask(const std::vector<uint32_t> &change_ver,
							std::vector<std::vector<std::vector<int>>> &need_add,
							std::vector<std::vector<bool>> &need_erase);
		void EraseVervex(const std::vector<std::vector<bool>> &need_erase);
		void AddVertex(const std::vector<std::vector<std::vector<int>>> &need_add);
		void CleanEmptyNode();
		int FindCorrectTid(int v);
		void dfs(std::vector<int> &Jump, int t, int v);

	public:
		std::vector<TreeNode *> t_;
		std::vector<uint32_t> tid_;
	public:
		Graph g_;
		std::unique_ptr<CohesiveModel> model_;
	};
}
