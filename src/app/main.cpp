
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include "Tree.hpp"
#include "AlgoProfile.hpp"
#include "CoreTree.hpp"
#include "AY.hpp"

#if defined(__linux__)
#include <unistd.h>
void Daemon()
{
	pid_t pid, sid;
	pid = fork();
	if (pid < 0)
		exit(EXIT_FAILURE);
	if (pid > 0)
		exit(EXIT_SUCCESS);
	sid = setsid();
	if (sid < 0)
		exit(EXIT_FAILURE);
	umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	int fd = open("exp13.txt", O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd < 0)
		printf("error\n");
	dup2(fd, STDOUT_FILENO);
	dup2(fd, STDERR_FILENO);
	if (fd > STDERR_FILENO)
		close(fd);
	printf("\n");
}
#endif
void Usage(std::string process)
{
	printf("\nUsage:\n\t");
	printf("%s graph_name\n", process.c_str());
	printf("Example:\n\t");
	printf("%s Graph/Google.txt\n\n", process.c_str());
}

void test(std::string filename)
{
	omp_set_num_threads(32);
	Tree::Tree t(filename, 32);
	


	printf("start build\n");
	fflush(stdout);
	hclock::time_point t1 = hclock::now();
	t.Build();
	hclock::time_point t2 = hclock::now();

	printf("build time: %.3lf\n",Tree::get_time_s(t1, t2));
	fflush(stdout);
	printf("start\n");
	fflush(stdout);
	for (int i = 1; i <= 1024; i *= 4)
	{
		std::vector<std::pair<uint32_t, uint32_t>> tmp(t.g_.GetRd().begin(), t.g_.GetRd().begin() + i);
		hclock::time_point s1 = hclock::now();
		t.RemoveEdge(tmp);
		hclock::time_point s2 = hclock::now();
		t.InsertEdge(tmp,true);
		hclock::time_point s3 = hclock::now();
		printf("num: %d remove: %.3lf insert: %.3lf\n", i, Tree::get_time_s(s1, s2), Tree::get_time_s(s2, s3));
		fflush(stdout);
	}
	for (int i = 10; i <= 10; i++)
	{
		std::vector<std::pair<uint32_t, uint32_t>> tmp(t.g_.GetRd().begin(), t.g_.GetRd().begin() + i);
		hclock::time_point s1 = hclock::now();
		t.RemoveEdge(tmp);
		hclock::time_point s2 = hclock::now();
		t.InsertEdge(tmp,true);
		hclock::time_point s3 = hclock::now();
		printf("num: %d remove: %.3lf insert: %.3lf\n", i, Tree::get_time_s(s1, s2), Tree::get_time_s(s2, s3));
		fflush(stdout);
	}
}

void test1(std::string filename)
{
	omp_set_num_threads(32);
	Tree::Tree t(filename, 32);

	printf("start build\n");
	fflush(stdout);
	t.Build();
	printf("start\n");
	fflush(stdout);
	for (int i = 10; i <= 10; i++)
	{
		std::vector<std::pair<uint32_t, uint32_t>> tmp(t.g_.GetRd().begin(), t.g_.GetRd().begin() + i);


		t.RemoveEdge(tmp);
		t.InsertEdge(tmp);

		//正常时间
		hclock::time_point s1 = hclock::now();
		t.RemoveEdge(tmp);
		hclock::time_point s2 = hclock::now();
		t.InsertEdge(tmp);
		hclock::time_point s3 = hclock::now();

		//禁用虚拟边压缩
		t.RemoveEdge(tmp);
		hclock::time_point s4 = hclock::now();
		t.InsertEdge(tmp,false,true);
		hclock::time_point s5 = hclock::now();
		//禁用延迟传播
		t.RemoveEdge(tmp);
		hclock::time_point s6 = hclock::now();
		t.InsertEdge(tmp,true,false);
		hclock::time_point s7 = hclock::now();

		// 禁用优化
		t.RemoveEdge(tmp,false);
		hclock::time_point s8 = hclock::now();
		t.InsertEdge(tmp);
		hclock::time_point s9 = hclock::now();

		// 正常时间
		t.RemoveEdge(tmp);
		hclock::time_point s10 = hclock::now();
		t.InsertEdge(tmp);
		hclock::time_point s11 = hclock::now();

		printf("插入正常时间: %.3lf  禁用虚拟边压缩: %.3lf 禁用延迟传播: %.3lf\n", 
			Tree::get_time_s(s2,s3),Tree::get_time_s(s4,s5),Tree::get_time_s(s6,s7));

		printf("删除正常时间: %.3lf 禁用优化: %.3lf\n", Tree::get_time_s(s1,s2),Tree::get_time_s(s7,s8) );

		printf("插入正常时间: %.3lf 删除正常时间: %.3lf \n",Tree::get_time_s(s10,s11),Tree::get_time_s(s9,s10));

		fflush(stdout);
	}
}

void test2(std::string filename)
{
	omp_set_num_threads(32);
	Tree::Tree t(filename, 32);

	printf("start build\n");
	fflush(stdout);
	t.Build();
	printf("start\n");
	fflush(stdout);
	for (int i = 1; i <= 32; i*=2)
	{
		std::vector<std::pair<uint32_t, uint32_t>> tmp(t.g_.GetRd().begin(), t.g_.GetRd().begin() + 10);
		t.SetThreadNum(i);
		hclock::time_point s1 = hclock::now();
		t.RemoveEdge(tmp);
		hclock::time_point s2 = hclock::now();
		t.InsertEdge(tmp,true);
		hclock::time_point s3 = hclock::now();

		printf("thread num: %d remove time: %.3lf insert time: %.3lf\n ",i,Tree::get_time_s(s1, s2),Tree::get_time_s(s2, s3));
		fflush(stdout);
	}
}


// int main()
// {
// 	Tree::Tree t("Graph/Google.txt");
// 	t.Build();
// 	std::vector<std::pair<uint32_t, uint32_t>> tmp(t.g_.GetRd().begin(), t.g_.GetRd().end());
// 	for(int i=0;i<100;++i)
// 	{
// 		std::random_device rd;
// 		std::mt19937 g(rd());
// 		// 打乱顺序
// 		std::shuffle(tmp.begin(), tmp.end(), g);
// 		std::vector<std::pair<uint32_t, uint32_t>> edg(tmp.begin(), tmp.begin()+512);
// 		t.RemoveEdge(edg);
// 		// printf("hello\n");
// 		t.InsertEdge(edg);
// 		// printf("hello\n");

// 		if(t.check()==false)
// 		{
// 			std::cout<<"No\n";
// 			return 0;
// 		}
// 		printf("%d\n",i);
// 	}
// 	std::cout<<"Yes\n";
// 	return 0;
// }
// int main()
// {
// 	// const char* arr[]={"Google","as-skitter","pokec","socfb-A-anon","web-wikipedia-growth","sx-superuser","soc-LiveJournal1","orkut","sx-stackoverflow","Friendster"};
// 	const char* arr[]={"youtube","yelp_edges","com-dblp","web-BerkStan"};

// 	// printf("hello");
// 	Daemon();
// 	// printf("world\n");
// 	for(auto e:arr)
// 	{
// 		std::string filename="Graph/";
// 		filename+=e;
// 		filename+=".txt";
// 		test1(filename);
// 		printf("\n");
// 		printf("================================\n");
// 		printf("\n");
// 		fflush(stdout);

// 	}
	

// 	std::cout << "success\n";
// 	fflush(stdout);

// 	return 0;
// }

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		Usage(argv[0]);
		return 0;
	}
	std::string filename = argv[1];
	// Daemon();

	omp_set_num_threads(32);
	hclock::time_point tp1 = hclock::now();
	Tree::Tree t(filename,32);

	printf("start build\n");
	fflush(stdout);
	hclock::time_point t1 = hclock::now();
	t.Build();
	hclock::time_point t2 = hclock::now();
	printf("start remove\n");
	fflush(stdout);
	// printf("dvg: %lf\n",t.Analyse_4());
	t.RemoveEdge(t.g_.GetRd());
	printf("complete remove\n");
	fflush(stdout);
	t.check();
	printf("start insert\n");
	fflush(stdout);
	t.InsertEdge(t.g_.GetRd(),true);
	printf("complete insert\n");
	fflush(stdout);
	t.check();
	AlgoProfile::GetInstance()->Print();

	std::cout << "success\n";
	fflush(stdout);

	return 0;
}

// Tree t("Graph/Google.txt");
// Tree t("Graph/gp.txt");
// Tree t("Graph/g5.txt");
// Tree t("Graph/orkut.txt");
// Tree t("Graph/pokec.txt");
// Tree t("Graph/as-skitter.txt");  //no
// Tree t("Graph/Friendster.txt");  //no
// Tree t("Graph/socfb-A-anon.txt");
// Tree t("Graph/sx-superuser.txt");
// Tree t("Graph/web-wikipedia-growth.txt");
// Tree t("Graph/soc-LiveJournal1.txt"); //bad
// Tree t("Graph/sx-stackoverflow.txt");

double avg_deg(int n, ll m, ll in, ll out, int num)
{
	return double(in) / num;
}

double cond(int n, ll m, ll in, ll out, int num)
{
	return 1. - double(out) / (in + out);
}

double density(int n, ll m, ll in, ll out, int num)
{
	return double(in) / (ll(num) * (num - 1));
}

double cut_ratio(int n, ll m, ll in, ll out, int num)
{
	if (n == num)
		return 1.0;
	return 1 - double(out) / num / (n - num);
}

double modularity(int n, ll m, ll in, ll out, int num)
{
	double mm = m * 2.;
	double tmp = double(in + out) / mm;
	return in / mm - tmp * tmp;
}

double sin_modularity(int n, ll m, ll in, ll out, int num)
{
	ll out2 = out;
	ll in2 = 2LL * m - in - out * 2;
	return modularity(n, m, in, out, num) + modularity(n, m, in2, out2, num);
}
double GetMaxScore(std::vector<CompRecord> vt)
{
	double ret = -1;
	for (auto e : vt)
		ret = std::max(ret, e.score);

	return ret;
}
int GetMax(std::vector<CompRecord> vt)
{
	double density=-1;
	int ret=0;
	for(auto e:vt)
	{
		if(e.score>density)
		{
			ret=e.v_num;
			density=e.score;
		}
		else if(e.score==density)
		{
			ret=std::max(ret,e.v_num);
		}
	}
	return ret;
}



void PrintAns(std::vector<CompRecord> vt, std::string file)
{
	std::ofstream fout(file);
	fout << "k score\n";
	std::map<int, double> ump;
	for (auto e : vt)
	{
		ump[e.k] = std::max(ump[e.k], e.score);
	}
	for (auto &e : ump)
	{
		fout << e.first << " " << e.second << "\n";
	}
	fout.close();
}
// int main(int argc, char *argv[])
// {
// 	if (argc != 2)
// 	{
// 		Usage(argv[0]);
// 		return 0;
// 	}
// 	std::string filename = argv[1];
// 	// Daemon();

// 	omp_set_num_threads(32);
// 	hclock::time_point tp1 = hclock::now();
// 	Tree::Tree t(filename);

// 	Graph gh(filename);
// 	ParaCoreTree ct(gh);
// 	t.Build();
// 	ct.build();

// 	auto ans1=__compute_metric_para(t,density);
// 	auto ans2=__compute_metric_para(ct,density);
// 	// auto ans1=get_cc_para(t);
// 	// auto ans2=get_cc_para(ct);
// 	// auto ans3=__compute_metric_para(t,cut_ratio);
// 	// auto ans4=__compute_metric_para(ct,cut_ratio);
// 	printf("ans1: %lf\n",GetMaxScore(ans1));
// 	printf("ans1: %d\n",GetMax(ans1));
// 	printf("ans2: %lf\n",GetMaxScore(ans2));
// 	printf("ans2: %d\n",GetMax(ans2));
// 	// printf("ans3: %lf\n",GetMaxScore(ans3));
// 	// printf("ans4: %lf\n",GetMaxScore(ans4));
// 	// PrintAns(ans1,"friend_trine.txt");
// 	// PrintAns(ans2,"frined_core.txt");
// 	// PrintAns(ans3,"g5_trine1.txt");
// 	// PrintAns(ans4,"g5_core2.txt");
// 	printf("success\n");
// }

// int main()
// {
// 	Daemon();
// 	Tree::Tree t("Graph/yelp_edges.txt");
// 	t.Build();
// 	printf("good\n");
// 	fflush(stdout);
// 	t.Analyse_5("Graph/yelp_fans_cnt.txt");
// 	// t.Analyse_5("Graph/yelp_review_cnt.txt");
// 	return 0;
// }