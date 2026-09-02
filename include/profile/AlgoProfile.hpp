#pragma once
#include <iostream>
#include <chrono>
#include <string>
using hclock = std::chrono::high_resolution_clock;

class AlgoProfile
{

public:
    enum Status
    {
        UNKNOWN,
        REMOVE,
        INSERT
    };

    ~AlgoProfile()
    {
        if (ptr_ != nullptr)
            delete ptr_;
    }
    static AlgoProfile *GetInstance()
    {
        if (ptr_ == nullptr)
            ptr_ = new AlgoProfile;
        return ptr_;
    }

    static hclock::time_point CaptureClock()
    {
        return hclock::now();
    }
    void SetStatus(Status st)
    {
        status_ = st;
    }
    void SetIOTime(hclock::time_point s1, hclock::time_point s2)
    {
        if (s1 > s2)
            std::swap(s1, s2);
        IO_time_ = get_time_s(s1, s2);
    }
    void SetUpdateTrinessTime(hclock::time_point s1, hclock::time_point s2)
    {
        if (s1 > s2)
            std::swap(s1, s2);
        update_triness_time_ = get_time_s(s1, s2);
    }
    void SetRebuildTime(hclock::time_point s1, hclock::time_point s2)
    {
        if (s1 > s2)
            std::swap(s1, s2);
        double t22 = get_time_s(s1, s2);
        if(rebuild_time_==-1)
            rebuild_time_=t22;
        else 
            rebuild_time_=(rebuild_time_+t22)/2;

    }
    void SetRemoveTime(hclock::time_point s1, hclock::time_point s2)
    {
        if (s1 > s2)
            std::swap(s1, s2);
        remove_edge_time = get_time_s(s1, s2);
    }
    void SetInsertTime(hclock::time_point s1, hclock::time_point s2)
    {
        if (s1 > s2)
            std::swap(s1, s2);
        insert_edge_time = get_time_s(s1, s2);
    }
    void SetOldMaxTriness(int t)
    {
        old_max_triness_ = t;
    }
    void SetNewMaxTriness(int t)
    {
        new_max_triness_ = t;
    }
    void SetRemoveEdgeNum(int a)
    {
        remove_edge_num_ = a;
    }
    void SetInsertEdgeNum(int a)
    {
        insert_edge_num = a;
    }
    void SetFilename(const std::string &filename)
    {
        filename_ = filename;
    }
    void SetCorrect(bool c)
    {
        if (status_ == UNKNOWN)
            return;
        int correct = -1;
        if (c)
            correct = 1;
        else
            correct = 0;
        if (status_ == REMOVE)
            remove_correct_ = correct;
        else if (status_ == INSERT)
            insert_correct_ = correct;
    }
    void SetEdgeNum(int a)
    {
        edge_num = a;
    }
    void SetVertexNum(int a)
    {
        vertex_num = a;
    }
    void Print()
    {
        std::string s[] = {"s", "ms", "us"};
        printf("=======================Graph Info========================\n");
        if (filename_ != "")
        {
            printf("Graph: ");
            auto pos = filename_.find(".");
            auto begin = filename_.find("/");
            if (begin == std::string::npos)
                begin = 0;
            else begin++;
            if (pos == std::string::npos)
                printf("%s", filename_.c_str());
            else
                std::cout << filename_.substr(begin, pos - begin);
            printf("\n");
            printf("Vertex Num: %d\n", vertex_num);
            printf("Edge Num: %d\n", edge_num);
            printf("Average Degree: %.1lf\n", ((double)edge_num * 2.0) / vertex_num);
            int p = 0;
            while (p != 2 && (int)IO_time_ == 0)
            {
                p++;
                IO_time_ *= 1000;
            }
            printf("I/O Time: %.3lf%s\n", IO_time_, s[p].c_str());
        }
        printf("=======================REMOVE EDGE=======================\n");
        double total_insert_time=0;
        double total_remove_time=0;
        if(remove_edge_time!=-1) total_remove_time = update_triness_time_+remove_edge_time;
        if(insert_edge_time!=-1) total_insert_time = update_triness_time_+insert_edge_time;
        int p1=0,p2=0;
        rebuild_time_ += update_triness_time_;
        double cp_rebuild_time=rebuild_time_;
        if (remove_edge_num_ != -1)
        {
            printf("Remove Edge Num: %d\n", remove_edge_num_);
            printf("Old Max Triness: %d\n", old_max_triness_);
            printf("New Max Triness %d\n", new_max_triness_);
            double speed = cp_rebuild_time / (total_remove_time);
            while (p1 != 2 && (int)update_triness_time_ == 0)
            {
                p1++;
                update_triness_time_ *= 1000;
            }
            printf("Update Triness Time: %.3lf%s\n", update_triness_time_, s[p1].c_str());

            // printf("re %lf",rebuild_time_);

            while (p2 != 2 && (int)rebuild_time_ == 0)
            {
                p2++;
                rebuild_time_ *= 1000;
            }
            printf("Rebuild Time: %.3lf%s\n", rebuild_time_, s[p2].c_str());
            int p = 0;
            while (p != 2 && (int)total_remove_time == 0)
            {
                p++;
                total_remove_time *= 1000;
            }
            printf("Update Tree Time: %.3lf%s\n", total_remove_time, s[p].c_str());
            printf("Algorithm Valid: ");
            if (remove_correct_ != -1)
            {
                // The algorithm is valid.
                if (remove_correct_)
                    printf("Yes");
                else
                    printf("No");
            }
            else printf("UNKNOWN");
            printf("\n");
            printf("Speed: %.1lf X", speed);
            printf("\n");
        }
        printf("=======================INSERT EDGE=======================\n");
        if (insert_edge_num != -1)
        {
            printf("Insert Edge Num: %d\n", insert_edge_num);
            printf("Old Max Triness: %d\n", old_max_triness_);
            printf("New Max Triness %d\n", new_max_triness_);
            // double total_update_time = update_triness_time_+insert_edge_time;
            double speed = cp_rebuild_time / (total_insert_time);
            printf("Update Triness Time: %.3lf%s\n", update_triness_time_, s[p1].c_str());

            printf("Rebuild Time: %.3lf%s\n", rebuild_time_, s[p2].c_str());
            int p = 0;
            while (p != 2 && (int)total_insert_time == 0)
            {
                p++;
                total_insert_time *= 1000;
            }
            printf("Update Tree Time: %.3lf%s\n", total_insert_time, s[p].c_str());
            printf("Algorithm Valid: ");
            if (insert_correct_ != -1)
            {
                // The algorithm is valid.
                if (insert_correct_)
                    printf("Yes");
                else
                    printf("No");
            }
            else printf("UNKNOWN");
            printf("\n");
            printf("Speed: %.1lf X", speed);
            printf("\n");
        }
        printf("=========================================================\n");
    }

private:
    AlgoProfile(/* args */)
    {
    }
    double get_time_s(hclock::time_point s, hclock::time_point t)
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(t - s).count();
    }

private:
    static AlgoProfile *ptr_;
    int old_max_triness_ = -1;
    int new_max_triness_ = -1;
    double rebuild_time_ = -1;
    double update_triness_time_ = -1;
    double IO_time_ = -1;
    int remove_edge_num_ = -1;
    int insert_edge_num = -1;
    double insert_edge_time = -1;
    double remove_edge_time = -1;
    int remove_correct_ = -1;
    int insert_correct_ = -1;
    std::string filename_;
    int edge_num = 0;
    int vertex_num = 0;
    Status status_ = UNKNOWN;
};
