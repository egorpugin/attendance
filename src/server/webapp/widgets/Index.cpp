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
    setContentAlignment(Wt::AlignmentFlag::Center);

    //auto I = addWidget(std::make_unique<Wt::WTemplate>(Wt::WString::tr("index")));

    //auto H = addWidget(std::make_unique<Wt::WLabel>(Wt::WString::tr("ui.SignIn")));

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

    if (u.type_id > PersonType::Normal)
    {
        auto ag = addWidget(std::make_unique<FormLineEdit>(Wt::WString::tr("ui.AddGroupLabel")));
        auto b = addWidget(std::make_unique<Wt::WPushButton>(Wt::WString::tr("ui.AddGroup")));
        b->clicked().connect([this, ag]()
        {
            auto &db = getAttendanceDatabase();
            db.addGroup(ag->Widget->text().toUTF8());
        });
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
