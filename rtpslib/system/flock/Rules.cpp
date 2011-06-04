#include <FLOCK.h>
#include<math.h>

namespace rtps 
{
    //----------------------------------------------------------------------
    Rules::Rules(std::string wpath, CL* cli_, EB::Timer* timer_)
    {
        cli = cli_;
        timer = timer_;
        std::string path;

        printf("create rules kernel\n");

        // separation
        try
        {
            path = wpath + "/rules.cl";
            k_rules= Kernel(cli, path, "rules");
        }
        catch (cl::Error er)
        {
            printf("ERROR(rules): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
#if 0
        // flockmates 
        try
        {
            path = wpath + "/flockmates.cl";
            k_flockmates= Kernel(cli, path, "flockmates");
        }
        catch (cl::Error er)
        {
            printf("ERROR(flockmates): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        // separation
        try
        {
            path = wpath + "/rule_separation.cl";
            k_rule_separation= Kernel(cli, path, "rule_separation");
        }
        catch (cl::Error er)
        {
            printf("ERROR(rule_separation): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        // alignment
        try
        {
            path = wpath + "/rule_alignment.cl";
            k_rule_alignment= Kernel(cli, path, "rule_alignment");
        }
        catch (cl::Error er)
        {
            printf("ERROR(rule_alignment): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        // cohesion
        try
        {
            path = wpath + "/rule_cohesion.cl";
            k_rule_cohesion= Kernel(cli, path, "rule_cohesion");
        }
        catch (cl::Error er)
        {
            printf("ERROR(rule_cohesion): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }

        // leader following 
        try
        {
            path = wpath + "/rule_leaderfollowing.cl";
            k_rule_leaderfollowing= Kernel(cli, path, "rule_leaderfollowing");
        }
        catch (cl::Error er)
        {
            printf("ERROR(rule_leaderfollowing): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
#endif
    }

    //----------------------------------------------------------------------
    void Rules::executeFlockmates(int num,
                    //input
                    Buffer<float4>& pos_s, 
                    Buffer<int4>& neigh_s, 
                    //output
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<GridParams>& gp,
                    Buffer<FLOCKParameters>& flockp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_rules.setArg(iarg++, pos_s.getDevicePtr());
        k_rules.setArg(iarg++, vel_s.getDevicePtr());
        k_rules.setArg(iarg++, neigh_s.getDevicePtr());
        k_rules.setArg(iarg++, sep_s.getDevicePtr());
        k_rules.setArg(iarg++, align_s.getDevicePtr());
        k_rules.setArg(iarg++, coh_s.getDevicePtr());
        k_rules.setArg(iarg++, ci_start.getDevicePtr());
        k_rules.setArg(iarg++, ci_end.getDevicePtr());
        k_rules.setArg(iarg++, gp.getDevicePtr());
        k_rules.setArg(iarg++, flockp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_rules.setArg(iarg++, clf_debug.getDevicePtr());
        k_rules.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_rules.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rules): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }

#if 0
    //----------------------------------------------------------------------
    void Rules::executeSeparation(int num,
                    //input
                    Buffer<float4>& pos_s,
                    Buffer<float4>& sep_s,
                    Buffer<int4>& neigh_s, 
                    //output
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<GridParams>& gp,
                    Buffer<FLOCKParameters>& flockp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_rule_separation.setArg(iarg++, pos_s.getDevicePtr());
        k_rule_separation.setArg(iarg++, sep_s.getDevicePtr());
        k_rule_separation.setArg(iarg++, neigh_s.getDevicePtr());
        k_rule_separation.setArg(iarg++, ci_start.getDevicePtr());
        k_rule_separation.setArg(iarg++, ci_end.getDevicePtr());
        k_rule_separation.setArg(iarg++, gp.getDevicePtr());
        k_rule_separation.setArg(iarg++, flockp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_rule_separation.setArg(iarg++, clf_debug.getDevicePtr());
        k_rule_separation.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_rule_separation.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rule_separation): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }

    //----------------------------------------------------------------------
    void Rules::executeAlignment(int num,
                    //input
                    Buffer<float4>& pos_s, 
                    Buffer<float4>& vel_s,
                    Buffer<float4>& align_s, 
                    Buffer<int4>& neigh_s, 
                    //output
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<GridParams>& gp,
                    Buffer<FLOCKParameters>& flockp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_rule_alignment.setArg(iarg++, pos_s.getDevicePtr());
        k_rule_alignment.setArg(iarg++, vel_s.getDevicePtr());
        k_rule_alignment.setArg(iarg++, align_s.getDevicePtr());
        k_rule_alignment.setArg(iarg++, neigh_s.getDevicePtr());
        k_rule_alignment.setArg(iarg++, ci_start.getDevicePtr());
        k_rule_alignment.setArg(iarg++, ci_end.getDevicePtr());
        k_rule_alignment.setArg(iarg++, gp.getDevicePtr());
        k_rule_alignment.setArg(iarg++, flockp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_rule_alignment.setArg(iarg++, clf_debug.getDevicePtr());
        k_rule_alignment.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_rule_alignment.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rule_alignment): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }

    //----------------------------------------------------------------------
    void Rules::executeCohesion(int num,
                    //input
                    Buffer<float4>& pos_s,
                    Buffer<float4>& coh_s, 
                    Buffer<int4>& neigh_s, 
                    //output
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<GridParams>& gp,
                    Buffer<FLOCKParameters>& flockp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_rule_cohesion.setArg(iarg++, pos_s.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, coh_s.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, neigh_s.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, ci_start.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, ci_end.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, gp.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, flockp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_rule_cohesion.setArg(iarg++, clf_debug.getDevicePtr());
        k_rule_cohesion.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_rule_cohesion.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rule_cohesion): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }

    //----------------------------------------------------------------------
    void Rules::executeLeaderFollowing(int num,
                    //input
                    Buffer<float4>& pos_s,
                    Buffer<float4>& vel_s,
                    Buffer<float4>& leadfoll_s,
                    Buffer<int4>& neigh_s, 
                    //output
                    Buffer<unsigned int>& ci_start,
                    Buffer<unsigned int>& ci_end,
                    //params
                    Buffer<GridParams>& gp,
                    Buffer<FLOCKParameters>& flockp,
                    //debug params
                    Buffer<float4>& clf_debug,
                    Buffer<int4>& cli_debug)
    { 
        int iarg = 0;
        k_rule_leaderfollowing.setArg(iarg++, pos_s.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, vel_s.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, leadfoll_s.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, neigh_s.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, ci_start.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, ci_end.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, gp.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, flockp.getDevicePtr());

        // ONLY IF DEBUGGING
        k_rule_leaderfollowing.setArg(iarg++, clf_debug.getDevicePtr());
        k_rule_leaderfollowing.setArg(iarg++, cli_debug.getDevicePtr());

        int local = 64;
        try
        {
            float gputime = k_rule_leaderfollowing.execute(num, local);
            if(gputime > 0)
                timer->set(gputime);

        }

        catch (cl::Error er)
        {
            printf("ERROR(rule_leaderfollowing): %s(%s)\n", er.what(), oclErrorString(er.err()));
        }
    }
#endif
    void FLOCK::cpuRules()
    {

    }

} 
