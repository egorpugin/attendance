#pragma sw require header pub.egorpugin.primitives.tools.sqlpp11.sqlite2cpp-0.3.2

#define PRIMITIVES_VERSION "-0.3.2"

void build(Solution &s)
{
    auto &support = s.addStaticLibrary("support");
    {
        auto &t = support;
        t += cpp26;
        t.PackageDefinitions = true;
        t += "src/support/.*"_rr;
        t.Public += "src/support"_id;

        t.Public += "org.sw.demo.boost.stacktrace"_dep;
        t.Public += "pub.egorpugin.primitives.filesystem"_dep;
        t.Public += "pub.egorpugin.primitives.hash"_dep;
        t.Public += "pub.egorpugin.primitives.templates"_dep;
        t.Public += "pub.egorpugin.primitives.log"_dep;

                        //- UNICODE
    }

    auto &webapp = s.addExecutable("server.webapp");
    {
        auto &t = webapp;
        t += cpp26;
        t.PackageDefinitions = true;
        t += "src/server/webapp/.*"_rr;
        t += "src/server/webapp"_id;

        t += support;
        t += "org.sw.demo.sqlite3"_dep;
        t += "org.sw.demo.rbock.sqlpp11"_dep;
        t += "org.sw.demo.emweb.wt.http"_dep;
        t += "pub.egorpugin.primitives.date_time"_dep;
        t += "pub.egorpugin.primitives.cron"_dep;
        t += "pub.egorpugin.primitives.sw.main"_dep;

        gen_sqlite2cpp("pub.egorpugin.primitives.tools.sqlpp11.sqlite2cpp" PRIMITIVES_VERSION ""_dep,
            t, t.SourceDir / "schema.sql", "schema.h", "db");
    }
}

