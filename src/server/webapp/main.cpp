
#include "app.h"
#include "server_options.h"

#include <primitives/cron.h>
#include <primitives/filesystem.h>
#include <primitives/log.h>

#include <Wt/WApplication.h>
#include <Wt/WServer.h>

void configure();

int WRun(int argc, char *argv[], Wt::ApplicationCreator createApplication)
{
    try
    {
        // use argv[0] as the application name to match a suitable entry
        // in the Wt configuration file, and use the default configuration
        // file (which defaults to /etc/wt/wt_config.xml unless the environment
        // variable WT_CONFIG_XML is set)
        Wt::WServer server(argv[0]);

        // WTHTTP_CONFIGURATION is e.g. "/etc/wt/wthttpd"
        server.setServerConfiguration(argc, argv, WTHTTP_CONFIGURATION);

        configure();

        server.addEntryPoint(Wt::EntryPointType::Application, createApplication);

        if (server.start())
        {
            int sig = Wt::WServer::waitForShutdown();
            std::cerr << "Shutdown (signal = " << sig << ")" << std::endl;
            server.stop();
#ifndef WT_WIN32
            if (sig == SIGHUP)
                Wt::WServer::restart(argc, argv, environ);
#endif
        }

        return 0;
    }
    catch (Wt::WServer::Exception& e)
    {
        std::cerr << e.what() << "\n";
        return 1;
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }
}

int main1(int argc, char **argv)
{
    // persistent storage
    Cron cron;
    get_cron(&cron);

    return WRun(argc, argv, [](const Wt::WEnvironment &env)
    {
        return std::make_unique<AttendanceApplication>(env);
    });
}

void setup_log(const std::string &log_level)
{
    LoggerSettings log_settings;
    log_settings.log_level = log_level;
    //log_settings.log_file = (get_root_directory() / "cppan").string();
    log_settings.simple_logger = true;
    log_settings.print_trace = true;
    initLogger(log_settings);

    // first trace message
    LOG_TRACE(logger, "----------------------------------------");
    LOG_TRACE(logger, "Starting cppan...");
}

int main_setup(int argc, char **argv)
{
    setup_utf8_filesystem();

#ifdef NDEBUG
    setup_log("INFO");
#else
    setup_log("DEBUG");
#endif

    try
    {
        return main1(argc, argv);
    }
    catch (Wt::WServer::Exception& e)
    {
        std::cerr << "wt exception: " << e.what() << "\n";
        return 1;
    }
    catch (std::exception &e)
    {
        std::cerr << "exception: " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "unknown exception" << "\n";
        return 1;
    }
}

int main(int argc, char **argv)
{
    auto r = main_setup(argc, argv);
    if (r)
    {
    }
    return r;
}

void configure()
{
    //SvcInstall();
}
