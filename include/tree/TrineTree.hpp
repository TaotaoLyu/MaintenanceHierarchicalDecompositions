#pragma once
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <iomanip>
#include <queue>
#include <chrono>
#include <iostream>
#include <mutex>

#include "Graph.hpp"
#include "Trine.hpp"
#include "GraphUtils.h"
#include "AlgoProfile.hpp"


namespace TrineTree
{

	double get_time_s(hclock::time_point s, hclock::time_point t);

	struct TrineTreeNode
	{
	public:
		int k = -1;	 // trine
		int pa = -1; // parent
		int tid = -1;
		std::vector<int> vertex_;
		std::vector<int> child_;
	};

	class ParaTrineTree
	{
	public:
		ParaTrineTree(const std::string &filename, int threadnum = 32)
			: g_(filename),
			  trine_(g_)
		{
			trine_.Init(); // cal trine
			SetThreadNum(threadnum);
		}
		ParaTrineTree(const Graph &pt)
			: g_(pt),
			  trine_(g_)
		{

			trine_.Init();
		}

		void Build(); // readlly build
		double correlation(const std::vector<std::pair<double, double>> &data)
		{
			int n = data.size();
			if (n < 2)
			{
				return -5;
			}

			double sum_x = 0.0, sum_y = 0.0;
			for (const auto &p : data)
			{
				sum_x += p.first;
				sum_y += p.second;
			}

			double mean_x = sum_x / n;
			double mean_y = sum_y / n;

			double numerator = 0.0;
			double sum_sq_x = 0.0;
			double sum_sq_y = 0.0;

			for (const auto &p : data)
			{
				double dx = p.first - mean_x;
				double dy = p.second - mean_y;
				numerator += dx * dy;
				sum_sq_x += dx * dx;
				sum_sq_y += dy * dy;
			}

			double denominator = std::sqrt(sum_sq_x * sum_sq_y);
			if (denominator == 0.0) // throw std::runtime_error("分母为零，无法计算相关系数");
				return -6;
			return numerator / denominator;
		}
		void Analyse_3()
		{

			std::vector<std::pair<double, double>> node_v(t_.size());
			std::vector<std::pair<double, double>> node_v2(t_.size());
			std::vector<std::pair<double, double>> node_v3(t_.size());
			std::vector<std::pair<double, double>> node_v4(t_.size());
			std::vector<std::pair<double, double>> node_v5(t_.size());
			std::vector<std::pair<double, double>> node_v6(t_.size());
			std::vector<std::pair<double, double>> node_v7(t_.size());
			std::vector<int> paper_count(g_.GetNodeNum());
			std::ifstream fin("Graph/author_paper_count.txt");
			int id, num = 0;
			while (fin >> id >> num)
			{
				paper_count[id] = num;
			}
			fin.close();

			for (auto ptr : t_)
			{
				TrineTreeNode &node = *ptr;
				double fir = 0;
				double fir2 = 0;
				double fir3 = ptr->k;
				double fir4 = 0;
				double fir6 = 0;
				double fir7 = 0;
				double sec = 0;
				for (auto nid : node.vertex_)
					sec += paper_count[nid];
				sec /= node.vertex_.size();

				for (auto nid : node.vertex_)
					for (auto u : g_[nid])
					{
						if (trine_[u] != trine_[nid])
						{
							fir += trine_[u], fir2++;
							fir6 += trine_[u] + 1;
						}
						fir4++;
						fir7 += trine_[u] + 1;
					}

				double fir5 = fir;

				fir /= node.vertex_.size();
				fir2 /= node.vertex_.size();
				node_v.emplace_back(fir, sec);
				node_v2.emplace_back(fir2, sec);
				node_v3.emplace_back(fir3, sec);
				node_v4.emplace_back(fir4, sec);
				node_v5.emplace_back(fir5, sec);
				node_v6.emplace_back(fir6, sec);
				node_v7.emplace_back(fir7, sec);
			}

			std::ofstream fout("ans3.txt");
			fout << std::fixed << std::setprecision(3);
			fout << "node_v " << correlation(node_v) << "\n";
			fout << "node_v2 " << correlation(node_v2) << "\n";
			fout << "node_v3 " << correlation(node_v3) << "\n";
			fout << "node_v4 " << correlation(node_v4) << "\n";
			fout << "node_v5 " << correlation(node_v5) << "\n";
			fout << "node_v6 " << correlation(node_v6) << "\n";
			fout << "node_v7 " << correlation(node_v7) << "\n";

			fout.flush();
			fout.close();
		}
		void Analyse_2()
		{
			std::vector<std::vector<std::pair<double, double>>> node_v(trine_.GetMaxTrine() + 1);
			std::vector<int> node_num(trine_.GetMaxTrine() + 1);
			std::vector<double> ans(trine_.GetMaxTrine() + 1);
			std::vector<int> paper_count(g_.GetNodeNum());
			std::ifstream fin("Graph/author_paper_count.txt");
			int id, num = 0;
			while (fin >> id >> num)
			{
				paper_count[id] = num;
			}
			fin.close();

			for (auto ptr : t_)
			{
				TrineTreeNode &node = *ptr;
				double fir = 0;
				double sec = 0;
				for (auto nid : node.vertex_)
					sec += paper_count[nid];
				sec /= node.vertex_.size();

				for (auto nid : node.vertex_)
					for (auto u : g_[nid])
						if (trine_[u] != trine_[nid])
							fir += trine_[u];

				fir /= node.vertex_.size();
				node_v[ptr->k].push_back(std::make_pair(fir, sec));
				node_num[ptr->k]++;
			}
			for (int i = 0; i <= trine_.GetMaxTrine(); ++i)
			{
				std::vector<std::pair<double, double>> &vt = node_v[i];
				ans[i] = correlation(vt);
			}
			std::ofstream fout("ans2.txt");
			fout << std::fixed << std::setprecision(3);

			for (int i = 0; i <= trine_.GetMaxTrine(); ++i)
			{
				fout << i << " " << ans[i] << "\n";
			}
			fout << "\n\n\n";
			for (int i = 0; i <= trine_.GetMaxTrine(); ++i)
			{
				fout << i << " " << node_num[i] << "\n";
			}
			fout.flush();
			fout.close();
		}
		void Analyse_1()
		{

			std::vector<double> av_cok(trine_.GetMaxTrine() + 1);
			std::vector<int> k_num(trine_.GetMaxTrine() + 1);
			std::vector<int> k_sum(trine_.GetMaxTrine() + 1);
			for (int i = 0; i < g_.GetNodeNum(); ++i)
			{
				k_num[trine_[i]]++;
				for (auto u : g_[i])
				{
					if (trine_[u] != trine_[i])
						k_sum[trine_[i]] += trine_[u];
				}
			}
			for (int i = 0; i <= trine_.GetMaxTrine(); ++i)
			{
				av_cok[i] = ((double)k_sum[i]) / k_num[i];
			}
			std::ofstream fout("ans1.txt");
			fout << std::fixed << std::setprecision(3);

			for (int i = 0; i <= trine_.GetMaxTrine(); ++i)
			{
				fout << i << " " << av_cok[i] << "\n";
			}
			fout.flush();
			fout.close();
		}

		double Analyse_4()
		{
			double ans=-1;
			for(int i=0;i<t_.size();++i)
			{
				if(t_[i]->k==trine_.GetMaxTrine())
				{
					int ed=0;
					std::unordered_set<int> ust;
					for(auto e:t_[i]->vertex_) ust.insert(e);
					for(auto u:t_[i]->vertex_)
					{
						for(auto v:g_[u])
						{
							if(ust.count(v))
								ed++;
						}
					}
					ans=std::max(ans,(double)ed/t_[i]->vertex_.size());
				}
			}
			return ans;
		}
		void Analyse_5(std::string filename)
		{
			printf("max trine: %d\n", trine_.GetMaxTrine());
			std::ifstream fin(filename);
			std::unordered_map<int,int> ump;
			std::unordered_map<int,int> subtree_size;
			printf("Analyse_5\n");
			fflush(stdout);
			int a,b;
			while (fin>>a>>b)
			{
				ump[a]=b;
			}
			int node_num=t_.size();
			for(int i=trine_.GetMaxTrine();i>=0;--i)
			{
				for(int j=0;j<node_num;++j)
				{
					if(t_[j]->k==i)
					{
						int num=1;
						for(auto ch:t_[j]->child_)
							num+=subtree_size[ch];
						subtree_size[j]=num;
					}
					
				}
			}
			std::unordered_map<int,int> to_bigid; //[k,id]
			std::vector<double> score_vt(t_.size());
			for(int i=0;i<node_num;++i)
			{
				double sc=0;
				for(auto e:t_[i]->vertex_)
					sc+=ump[e];

				score_vt[i]=sc/t_[i]->vertex_.size();
			}

			for(int i=0;i<node_num;++i)
			{
				int curk=t_[i]->k;
				if(!to_bigid.count(curk))
				{
					to_bigid[curk]=i;
					continue;
				}
				if(subtree_size[i]>subtree_size[to_bigid[curk]])
				{
					to_bigid[curk]=i;
				}
				else if(subtree_size[i]==subtree_size[to_bigid[curk]])
				{
					if(score_vt[i]>score_vt[to_bigid[curk]])
						to_bigid[curk]=i;
				}
			}
			std::unordered_map<int,int> bigger;
			std::unordered_map<int,int> smaller;
			std::unordered_map<int,int> equal;
			for(int i=0;i<node_num;++i)
			{
				int curk=t_[i]->k;
				double maxscore=score_vt[to_bigid[curk]];
				if(score_vt[i]>maxscore)
					bigger[curk]++;
				else if(score_vt[i]<maxscore)
					smaller[curk]++;
				else
					equal[curk]++;
			}
			for(int i=0;i<trine_.GetMaxTrine();++i)
			{
				if(!bigger.count(i)&&!smaller.count(i)&&!equal.count(i)) continue;

				double total=bigger[i]+smaller[i]+equal[i];
				printf("k: %d bigger: %.2lf%% equal: %.2lf%% smaller: %.2lf%% equal num: %d\n",
					i, (bigger[i]/total)*100,(equal[i]/total)*100,(smaller[i]/total)*100 ,equal[i]);
			}


		}

		void Merge(TrineTreeNode *dst, TrineTreeNode *src);

		void InsertEdge(const EdgListT &edg,bool flag=true,bool flag1=true);

		void RemoveEdge(const EdgListT &edg, bool flag=true);

		bool check();
		void SetThreadNum(int num);
	private:
		
		void MoveToNewTid(std::vector<int> &changever, std::vector<int> &old_tid, std::vector<int> &old_trine);
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
		std::vector<TrineTreeNode *> t_;
		std::vector<uint32_t> tid_;
	public:
		Graph g_;
		Trine trine_;
	};
}
