#pragma once
#include<vector>
#include"CoreTree.hpp"
#include"TrineTree.hpp"
#include"GraphUtils.h"
#include"ParaGraph.hpp"

struct CompRecord {
	int k;
	int id_max_rk;
	double score;
	int v_num=0;
};
using ll=long long;
using VLL=std::vector<long long>;
typedef double score_f(int n, ll m, ll in, ll out, int num);
std::vector<CompRecord> __compute_metric_para(ParaCoreTree & pt, score_f f);

std::vector<CompRecord> __compute_metric_para(TrineTree::ParaTrineTree & pt, score_f f);

std::vector<CompRecord> get_cc_para(ParaCoreTree & pt);
std::vector<CompRecord> get_cc_para(TrineTree::ParaTrineTree & pt);