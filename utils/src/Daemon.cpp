#include "Daemon.hpp"

#include <cstdio>
#include <cstdlib>

#if defined(__linux__)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace Utils
{
    void StartDaemon(const std::string &log_filename)
    {
#if defined(__linux__)
        const pid_t process_id = fork();
        if (process_id < 0)
        {
            std::perror("Failed to fork daemon process");
            std::exit(EXIT_FAILURE);
        }
        if (process_id > 0)
            std::exit(EXIT_SUCCESS);

        if (setsid() < 0)
        {
            std::perror("Failed to create daemon session");
            std::exit(EXIT_FAILURE);
        }

        umask(0);
        const int log_fd = open(log_filename.c_str(),
                                O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (log_fd < 0)
        {
            std::perror("Failed to open daemon log file");
            std::exit(EXIT_FAILURE);
        }

        close(STDIN_FILENO);
        if (dup2(log_fd, STDOUT_FILENO) < 0 ||
            dup2(log_fd, STDERR_FILENO) < 0)
        {
            std::exit(EXIT_FAILURE);
        }
        if (log_fd > STDERR_FILENO)
            close(log_fd);

        std::printf("[Daemon] Started.\n");
        std::fflush(stdout);
#else
        std::fprintf(stderr,
                     "Daemon mode is supported only on Linux.\n");
        std::exit(EXIT_FAILURE);
#endif
    }
}
