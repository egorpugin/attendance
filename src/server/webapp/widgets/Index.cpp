#include "Index.h"

#include "app.h"
#include "database.h"
#include "user.h"

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Wt/WBreak.h>
#include <Wt/WComboBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WAny.h>
#include <Wt/WTemplate.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>
#include <Wt/WConfig.h>
#include <Wt/WServer.h>
#include <Wt/WIOService.h>
#include <Wt/WFormModel.h>
#include <Wt/WTable.h>
#include <Wt/WCheckBox.h>
#include <Wt/Http/Client.h>
#include <Wt/WMenuItem.h>
#include <Wt/WTabWidget.h>
#include <Wt/WTextArea.h>

#include <primitives/log.h>
DECLARE_STATIC_LOGGER(logger, "index");

CenterContentWidget::CenterContentWidget()
    : WContainerWidget()
{
}

IndexWidget::IndexWidget()
    : CenterContentWidget()
{
    //setContentAlignment(Wt::AlignmentFlag::Center);

    auto aa = ((AttendanceApplication*)Wt::WApplication::instance());
    if (aa->getUser().id == 0)
        return;

    //auto b = addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.CheckIn")));
    //addWidget(std::make_unique<Wt::WBreak>());
    //addWidget(std::make_unique<Wt::WBreak>());

    auto e = addWidget(std::make_unique<Wt::WLabel>(""));

    auto &u = aa->getUser();

    CheckInInfo i;
    i.person_id = u.id;

    auto f = [u, e]()
    {
        CheckInInfo i;
        i.person_id = u.id;

        auto &db = getAttendanceDatabase();
        auto r = db.checkIn(i);
        if (u.type_id == PersonType::Normal)
        switch (r)
        {
        case CheckStatus::Ok:
            e->setText(Wt::WString::tr("ui.CheckedIn"));
            e->setAttributeValue("style", "color: green;");
            break;
        case CheckStatus::AlreadyChecked:
            e->setText(Wt::WString::tr("ui.AlreadyChecked"));
            e->setAttributeValue("style", "color: red;");
            break;
        case CheckStatus::InappropriateTime:
            e->setText(Wt::WString::tr("ui.InappropriateTime"));
            e->setAttributeValue("style", "color: red;");
            break;
        }
    };

    f();
    //b->clicked().connect(f);

    if (u.type_id == PersonType::Normal)
        return;

    Wt::WTabWidget *tabW = addWidget(std::make_unique<Wt::WTabWidget>());
    tabW->setStyleClass("tabwidget");

    auto &db = getAttendanceDatabase();
    auto groups = db.getGroups();
    auto subjects = db.getSubjects();

    {
        auto c = std::make_unique<Wt::WContainerWidget>();
        c->addWidget(std::make_unique<Wt::WBreak>());

        auto ag = c->addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.AddGroupLabel")));
        auto b = c->addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.AddGroup")));
        b->clicked().connect([this, ag]()
        {
            auto &db = getAttendanceDatabase();
            db.addGroup(ag->Widget->text().toUTF8());
        });
        c->addWidget(std::make_unique<Wt::WBreak>());
        c->addWidget(std::make_unique<Wt::WBreak>());

        {
            auto table = std::make_unique<LabeledWidget<Wt::WTable>>("List of groups");
            auto T = table->Widget;
            T->addStyleClass("table");

            T->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("#"));
            T->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Name"));
            T->elementAt(0, 2)->addWidget(std::make_unique<Wt::WText>("Number of students"));

            int i = 1;
            for (auto &g : groups)
            {
                T->elementAt(i, 0)->addWidget(std::make_unique<Wt::WText>(std::to_string(i)));
                T->elementAt(i, 1)->addWidget(std::make_unique<Wt::WText>(g.name));
                T->elementAt(i, 2)->addWidget(std::make_unique<Wt::WText>("Number of students"));
                i++;
            }

            c->addWidget(std::move(table));
        }

        tabW->addTab(std::move(c), "Groups", Wt::ContentLoading::Eager);
    }

    {
        auto c = std::make_unique<Wt::WContainerWidget>();
        c->addWidget(std::make_unique<Wt::WBreak>());

        auto ag = c->addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.AddSubjectLabel")));
        auto b = c->addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.AddSubject")));
        b->clicked().connect([this, ag]()
        {
            auto &db = getAttendanceDatabase();
            db.addSubject(ag->Widget->text().toUTF8());
        });
        c->addWidget(std::make_unique<Wt::WBreak>());
        c->addWidget(std::make_unique<Wt::WBreak>());

        {
            auto table = std::make_unique<LabeledWidget<Wt::WTable>>("List of subjects");
            auto T = table->Widget;
            T->addStyleClass("table");

            T->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("#"));
            T->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Name"));

            int i = 1;
            for (auto &s : subjects)
            {
                T->elementAt(i, 0)->addWidget(std::make_unique<Wt::WText>(std::to_string(i)));
                T->elementAt(i, 1)->addWidget(std::make_unique<Wt::WText>(s.name));
                i++;
            }

            c->addWidget(std::move(table));
        }

        tabW->addTab(std::move(c), "Subjects", Wt::ContentLoading::Eager);
    }

    {
        auto c = std::make_unique<Wt::WContainerWidget>();
        c->addWidget(std::make_unique<Wt::WBreak>());

        auto sw = c->addWidget(std::make_unique<LabeledWidget<Wt::WComboBox>>(Wt::WString::tr("ui.Subject")));
        for (auto &s : subjects)
            sw->Widget->addItem(s.name);

        c->addWidget(std::make_unique<Wt::WLabel>(Wt::WString::tr("ui.Group")));
        c->addWidget(std::make_unique<Wt::WBreak>());
        c->addWidget(std::make_unique<Wt::WBreak>());
        auto cgw = c->addWidget(std::make_unique<Wt::WContainerWidget>());
        auto add_groups = [cgw, groups]()
        {
            auto gw = cgw->addWidget(std::make_unique<Wt::WComboBox>());
            for (auto &g : groups)
                gw->addItem(g.name);
        };
        add_groups();
        c->addWidget(std::make_unique<Wt::WBreak>());

        auto addg = c->addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.AddGroupMore")));
        addg->clicked().connect([add_groups]()
        {
            add_groups();
        });

        c->addWidget(std::make_unique<Wt::WBreak>());
        c->addWidget(std::make_unique<Wt::WBreak>());

        auto teachers = db.getTeachers();
        LabeledWidget<Wt::WComboBox> *tw = nullptr;
        if (u.type_id >= PersonType::Administrator)
        {
            auto teachers = db.getTeachers();
            tw = c->addWidget(std::make_unique<LabeledWidget<Wt::WComboBox>>(Wt::WString::tr("ui.Teacher")));
            for (auto &t : teachers)
                tw->Widget->addItem(t.getFio());
        }

        auto b = c->addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.AddCourse")));
        b->clicked().connect([this, cgw, sw, subjects, u, teachers, tw, groups]()
        {
            auto &db = getAttendanceDatabase();
            auto sid = subjects[sw->Widget->currentIndex()].id;
            auto tid = tw ? teachers[tw->Widget->currentIndex()].id : u.id;
            std::set<db_id> gids;
            for (auto &w : cgw->children())
            {
                auto cb = (Wt::WComboBox*)w;
                gids.insert(groups[cb->currentIndex()].id);
            }

            // sem
            auto time = boost::posix_time::second_clock::local_time();
            auto d = time.date();
            auto sem = (d.year() % 100) * 10;
            sem += d.month() > 6 ? 1 : 2;

            db.addCourse(sid, tid, gids, sem);
        });
        c->addWidget(std::make_unique<Wt::WBreak>());
        c->addWidget(std::make_unique<Wt::WBreak>());

        {
            auto table = std::make_unique<LabeledWidget<Wt::WTable>>("List of courses");
            auto T = table->Widget;
            T->addStyleClass("table");

            T->elementAt(0, 0)->addWidget(std::make_unique<Wt::WText>("#"));
            T->elementAt(0, 1)->addWidget(std::make_unique<Wt::WText>("Name"));

            int i = 1;
            for (auto &s : subjects)
            {
                T->elementAt(i, 0)->addWidget(std::make_unique<Wt::WText>(std::to_string(i)));
                T->elementAt(i, 1)->addWidget(std::make_unique<Wt::WText>(s.name));
                i++;
            }

            c->addWidget(std::move(table));
        }

        tabW->addTab(std::move(c), "Courses", Wt::ContentLoading::Eager);
    }

}

void IndexWidget::show()
{
}

SignInWidget::SignInWidget()
{
    auto H = addWidget(std::make_unique<Wt::WLabel>(Wt::WString::tr("ui.SignIn")));
    H->setAttributeValue("style", "font-size: 18px;");
    addWidget(std::make_unique<Wt::WBreak>());
    addWidget(std::make_unique<Wt::WBreak>());
    Name = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.Login")));
    Password = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.Password")));
    Password->Widget->setEchoMode(Wt::EchoMode::Password);
    RememberMe = addWidget(std::make_unique<Wt::WCheckBox>(Wt::WString::tr("ui.RememberMe")));
    addWidget(std::make_unique<Wt::WBreak>());
    addWidget(std::make_unique<Wt::WBreak>());
    addWidget(std::make_unique<Wt::WBreak>());
    auto L = addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.Login")));
    addWidget(std::make_unique<Wt::WBreak>());
    addWidget(std::make_unique<Wt::WBreak>());
    Error = addWidget(std::make_unique<Wt::WLabel>(""));
    Error->setAttributeValue("style", "color: red;");
    L->addStyleClass("btn-success");
    L->clicked().connect([this]()
    {
        auto n = Name->text();
        auto p = Password->text();
        auto c = RememberMe->isChecked();
        if (n.empty() || p.empty())
        {
            Error->setText(Wt::WString::tr("ui.AllFieldsFilled"));
            return;
        }
        if (auto t = login(n.toUTF8(), p.toUTF8(), c); std::get<0>(t).id == 0)
        {
            Error->setText(std::get<2>(t));
            return;
        }
        Wt::WApplication::instance()->setInternalPath("/", true);
        doJavaScript(R"(location.reload();)");
    });
}

SignUpWidget::SignUpWidget()
{
    auto H = addWidget(std::make_unique<Wt::WLabel>(Wt::WString::tr("ui.Register")));
    H->setAttributeValue("style", "font-size: 18px;");
    addWidget(std::make_unique<Wt::WBreak>());
    addWidget(std::make_unique<Wt::WBreak>());

    auto Login = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.Login")));
    auto Password = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.Password")));
    Password->Widget->setEchoMode(Wt::EchoMode::Password);
    auto RepeatPassword = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.RepeatPassword")));
    RepeatPassword->Widget->setEchoMode(Wt::EchoMode::Password);

    auto Surame = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.Surame")));
    auto FirstName = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.FirstName")));
    auto MiddleName = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.MiddleName")));
    //auto Email = addWidget(std::make_unique<FormLineEdit>("Email address"));

    auto &db = getAttendanceDatabase();
    auto groups = db.getGroups();

    auto combo = addWidget(std::make_unique<LabeledWidget<Wt::WComboBox>>(Wt::WString::tr("ui.Group")));
    for (auto &g : groups)
        combo->Widget->addItem(g.name);

    addWidget(std::make_unique<Wt::WBreak>());

    auto L = addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.Register")));
    addWidget(std::make_unique<Wt::WBreak>());
    addWidget(std::make_unique<Wt::WBreak>());
    Error = addWidget(std::make_unique<Wt::WLabel>(""));
    Error->setAttributeValue("style", "color: red;");
    L->addStyleClass("btn-success");
    L->clicked().connect([=]()
    {
        Person p;

        p.login = Login->text().toUTF8();
        p.password = Password->text().toUTF8();
        auto rp = RepeatPassword->text().toUTF8();

        p.surname = Surame->text().toUTF8();
        p.first_name = FirstName->text().toUTF8();
        p.middle_name = MiddleName->text().toUTF8();

        Group g;
        g.id = groups[combo->Widget->currentIndex()].id;
        p.groups.push_back(g);

        if (p.login.empty() || p.password.empty()
            //|| e.empty()
            || p.surname.empty()
            || p.first_name.empty()
            || p.middle_name.empty()
            )
        {
            Error->setText(Wt::WString::tr("ui.AllFieldsFilled"));
            return;
        }
        if (p.password != rp)
        {
            Error->setText(Wt::WString::tr("ui.PassDoNotMatch"));
            return;
        }
        if (auto t = register_user(p); !std::get<0>(t))
        {
            Error->setText(std::get<1>(t));
            return;
        }
        Wt::WApplication::instance()->setInternalPath("/signin", true);
    });
}

void UserWidget::setUser(const String &u)
{

}
