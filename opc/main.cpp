#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/program_options.hpp>
#include <array>
#include <ctime>
#include <iostream>
#include <experimental/filesystem>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "common/MSocket.h"
#include "common/SysLogger.h"
#include "opp/Discover.h"
#include <unordered_map>
using boost::asio::ip::udp;

class FindOrganization: public MSocket::MessageHandler
{
public:
    struct OrganizationIP
    {
        union
        {
            int32_t ip;
            struct
            {
               uint8_t ipp1, ipp2, ipp3, ipp4;
            };
        };
        int32_t port;
    };

    FindOrganization(MSocket* pSocket, int32_t ip, const char* name)
        :mSocket(pSocket)
	    ,mIP(ip)
    {
        strncpy(mName, name, 32-1-6);
    }

    void OnMessage(const Discover& discover) override
    {
        for(auto& orgaPair : mOrganizations)
        {
            auto& organizationIP = orgaPair.second;
            if(organizationIP.ip == discover.ip && organizationIP.port == discover.port)
            {
                syslog(LOG_NOTICE | LOG_USER, "%s　find duplicate　organization %d.%d.%d.%d:%d, %m"
                    , __FUNCTION__, organizationIP.ipp1, organizationIP.ipp2, organizationIP.ipp3
                    , organizationIP.ipp4, organizationIP.port
                );
                return;
            }
        }

        Discover response;
        response.magic = -1;
        response.ip = mIP;
        response.port = 0; //TODO
        strncpy(response.name, mName, sizeof(response.name)-1);
        mSocket->Send(response);
    }

    bool FiltOut(const Discover& discover) override
    {
        return discover.magic < 0;
    }

    void WaitOrganizationExit()
    {
	int32_t status = 0;
        auto pid = waitpid(-1, &status, WNOHANG);
        if(pid > 0)
        {
            mOrganizations.erase(pid);
        }
        else
        {
            syslog(LOG_ERR | LOG_USER, "%s　find illegal　pid %d, %m", __FUNCTION__, pid);
        }
    }
private:
    MSocket* mSocket{nullptr};
    std::unordered_map<pid_t, OrganizationIP> mOrganizations;
    int32_t mIP{-1};
    char mName[32];
};

namespace fs = std::experimental::filesystem;
int main(int argc, char** argv)
{
    try
    {
        fs::path progPath(argv[0]);
        auto progName = progPath.filename().string();

        using namespace boost::program_options;
        options_description opts("opc options");
        opts.add_options()
                ("help,h","print this help information.")
                ("name,n",value<std::string>(), "name")
                ("ip,l", value<std::string>(), "tcp ip")
                ("mip,i", value<std::string>(), "multicast ip")
                ("mport,p", value<uint16_t>(), "multicast port");

        variables_map vm;
        store(parse_command_line(argc,argv,opts),vm);

        if(vm.count("help"))
        {
            std::cout<<opts<<std::endl;
            return 0;
        }

        if(!(vm.count("name") && vm.count("ip") && vm.count("mip") && vm.count("mport")))
        {
            std::cout<<opts<<std::endl;
            return -1;
        }

        boost::asio::io_context io_context;

        // Initialise the server before becoming a daemon. If the process is
        // started from a shell, this means any errors will be reported back to the
        // user.
        auto name = vm["name"].as<std::string>();
        auto ip = vm["ip"].as<std::string>();
        auto mip = vm["mip"].as<std::string>();
        auto mport = vm["mport"].as<uint16_t>();
        MSocket socket(io_context
            , boost::asio::ip::make_address(ip) //listen address
            , boost::asio::ip::make_address(mip) //multicast address
            , mport); //multicast port

        boost::asio::ip::tcp::resolver resolver(io_context);
	    boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(ip, std::to_string(22)).begin();

        FindOrganization server(&socket, endpoint.address().to_v4().to_uint(), name.c_str());

        // Register signal handlers so that the daemon may be shut down. You may
        // also want to register for other signals, such as SIGHUP to trigger a
        // re-read of a configuration file.
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait(
            [&](boost::system::error_code /*ec*/, int /*signo*/)
            {
                io_context.stop();
            });

        boost::asio::signal_set childSignals(io_context, SIGCHLD);
        childSignals.async_wait(
            [&](boost::system::error_code /*ec*/, int signo)
            {
                server.WaitOrganizationExit();
            });
        // Inform the io_context that we are about to become a daemon. The
        // io_context cleans up any internal resources, such as threads, that may
        // interfere with forking.
        io_context.notify_fork(boost::asio::io_context::fork_prepare);

        // Fork the process and have the parent exit. If the process was started
        // from a shell, this returns control to the user. Forking a new process is
        // also a prerequisite for the subsequent call to setsid().
        SysLogger logGuard("dfc-opc", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
        if (pid_t pid = fork())
        {
            if (pid > 0)
            {
                // We're in the parent process and need to exit.
                //
                // When the exit() function is used, the program terminates without
                // invoking local variables' destructors. Only global variables are
                // destroyed. As the io_context object is a local variable, this means
                // we do not have to call:
                //
                //   io_context.notify_fork(boost::asio::io_context::fork_parent);
                //
                // However, this line should be added before each call to exit() if
                // using a global io_context object. An additional call:
                //
                //   io_context.notify_fork(boost::asio::io_context::fork_prepare);
                //
                // should also precede the second fork().
                exit(0);
            }
            else
            {
                syslog(LOG_ERR | LOG_USER, "First fork failed: %m");
                return 1;
            }
        }

        // Make the process a new session leader. This detaches it from the
        // terminal.
        setsid();

        // A process inherits its working directory from its parent. This could be
        // on a mounted filesystem, which means that the running daemon would
        // prevent this filesystem from being unmounted. Changing to the root
        // directory avoids this problem.
        chdir("/");

        // The file mode creation mask is also inherited from the parent process.
        // We don't want to restrict the permissions on files created by the
        // daemon, so the mask is cleared.
        umask(0);

        // A second fork ensures the process cannot acquire a controlling terminal.
        if (pid_t pid = fork())
        {
            if (pid > 0)
            {
                exit(0);
            }
            else
            {
                syslog(LOG_ERR | LOG_USER, "Second fork failed: %m");
                return 1;
            }
        }

        // Close the standard streams. This decouples the daemon from the terminal
        // that started it.
        close(0);
        close(1);
        close(2);

        // We don't want the daemon to have any standard input.
        if (open("/dev/null", O_RDONLY) < 0)
        {
            syslog(LOG_ERR | LOG_USER, "Unable to open /dev/null: %m");
            return 1;
        }

        // Send standard output to a log file.
        const char* output = "/tmp/asio.daemon.out";
        const int flags = O_WRONLY | O_CREAT | O_APPEND;
        const mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        if (open(output, flags, mode) < 0)
        {
            syslog(LOG_ERR | LOG_USER, "Unable to open output file %s: %m", output);
            return 1;
        }

        // Also send standard error to the same log file.
        if (dup(1) < 0)
        {
            syslog(LOG_ERR | LOG_USER, "Unable to dup output descriptor: %m");
            return 1;
        }

        // Inform the io_context that we have finished becoming a daemon. The
        // io_context uses this opportunity to create any internal file descriptors
        // that need to be private to the new process.
        io_context.notify_fork(boost::asio::io_context::fork_child);

        // The io_context can now be used normally.
        syslog(LOG_INFO | LOG_USER, "opc Daemon started");
        io_context.run();
        syslog(LOG_INFO | LOG_USER, "opc Daemon stopped");
    }
    catch (std::exception& e)
    {
        syslog(LOG_ERR | LOG_USER, "Exception: %s", e.what());
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
