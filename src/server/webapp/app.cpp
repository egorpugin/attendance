#include "app.h"

#include "widgets/Index.h"

#include "user.h"

#include <primitives/filesystem.h>
#include <primitives/log.h>

#include <Wt/WBootstrapTheme.h>
#include <Wt/WTemplate.h>
#include <Wt/WAnchor.h>
#include <Wt/WPushButton.h>
#include <Wt/WNavigationBar.h>
#include <Wt/WPopupMenu.h>
#include <Wt/WLineEdit.h>
#include <Wt/WApplication.h>
#include <Wt/WEnvironment.h>

#include <regex>

AttendanceApplication::AttendanceApplication(const Wt::WEnvironment &env)
    : Wt::WApplication(env)
{
    setTitle(Wt::WString::tr("ui.AttendanceJournal"));

    useStyleSheet("css/cppan.css");
    require("js/cppan.js");

    messageResourceBundle().use(appRoot() + "strings");
    messageResourceBundle().use(appRoot() + "templates");

    auto bt = std::make_shared<Wt::WBootstrapTheme>();
    bt->setVersion(Wt::BootstrapVersion::v3);
    setTheme(bt);

    // check cookie
    auto c_uid = env.getCookie(COOKIE_USER_ID);
    auto c_us = env.getCookie(COOKIE_USER_SESSION);
    if (c_uid && c_us)
        user = loginByCookie(*c_uid, *c_us);

    setContents();

    internalPathChanged().connect(this, &AttendanceApplication::handleInternalPath);
    handleInternalPath(env.internalPath());
}

void AttendanceApplication::setContents()
{
    root()->setMargin(0);

    auto container = std::make_unique<Wt::WContainerWidget>();
    Wt::WNavigationBar *navigation =
        container->addWidget(std::make_unique<Wt::WNavigationBar>());
    navigation->setTitle(Wt::WString::tr("ui.AttendanceJournal"), Wt::WLink(Wt::LinkType::InternalPath, "/"));
    navigation->setResponsive(true);
    navigation->setId("navigation_x");
    //navigation->addStyleClass("navbarxcontainer");
    //navigation->setAttributeValue("style", "padding-left: 100px; padding-right: 100px;");
    doJavaScript(R"(
        css = document.getElementById('main_content_x');
        nav = document.getElementById('navigation_x');
        nav.style.paddingLeft = nav.style.paddingRight = Math.floor(($(window).width() - parseInt(window.getComputedStyle(css).width, 10)) / 2 - 4) + "px";

        window.addEventListener('resize', function(event){
            css = document.getElementById('main_content_x');
            nav = document.getElementById('navigation_x');
            nav.style.paddingLeft = nav.style.paddingRight = Math.floor(($(window).width() - parseInt(window.getComputedStyle(css).width, 10)) / 2 - 4) + "px";
        });
        )");
    //navigation->addStyleClass("container");
    //navigation->addStyleClass("main-content");

    MainStack = container->addWidget(std::make_unique<Wt::WStackedWidget>());
    container->addWidget(std::make_unique<Wt::WBreak>());
    container->addWidget(std::make_unique<Wt::WBreak>());

    MainStack->addStyleClass("contents");
    MainStack->addStyleClass("container");
    MainStack->addStyleClass("main-content");
    MainStack->setId("main_content_x");

    // Setup a Left-aligned menu.
    auto leftMenu = std::make_unique<Wt::WMenu>(MainStack);
    auto leftMenu_ = navigation->addMenu(std::move(leftMenu));

    // Setup a Right-aligned menu.
    auto rightMenu = std::make_unique<Wt::WMenu>();
    auto rightMenu_ = navigation->addMenu(std::move(rightMenu), Wt::AlignmentFlag::Right);

    if (!isAuthorized())
    {
        rightMenu_->addItem(Wt::WString::tr("ui.SignIn"))
            ->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/signin"));
        rightMenu_->addItem(Wt::WString::tr("ui.SignUp"))
            ->setLink(Wt::WLink(Wt::LinkType::InternalPath, "/signup"));
    }
    else
    {
        {
            auto M = std::make_unique<Wt::WPopupMenu>();
            auto logged = M->addItem("");
            logged->anchor()->setTextFormat(Wt::TextFormat::XHTML);
            logged->anchor()->setText(Wt::WString::tr("ui.LoggedInAs") + " <span style=\"font-weight: bold;\">" + user.login + "</span>");
            M->addSeparator();
            M->addItem(Wt::WString::tr("ui.LogOut"))->triggered().connect([this] {
                logout();
                setInternalPath("/", true);
                doJavaScript(R"(location.reload();)");
            });

            auto item = std::make_unique<Wt::WMenuItem>(Wt::WString(user.getFio()) + " ");
            item->setMenu(std::move(M));
            rightMenu_->addItem(std::move(item));
        }
    }

    root()->addWidget(std::move(container));
}

void AttendanceApplication::handleInternalPath(const std::string &p)
{
    if (SearchField)
        SearchField->show();

    if (p == "/signin" && !isAuthorized())
    {
        if (!SignIn)
            SignIn = MainStack->addWidget(std::make_unique<SignInWidget>());
        MainStack->setCurrentWidget(SignIn);
        return;
    }
    else if (p == "/signup" && !isAuthorized())
    {
        if (!SignUp)
            SignUp = MainStack->addWidget(std::make_unique<SignUpWidget>());
        MainStack->setCurrentWidget(SignUp);
        return;
    }

    if (!Index)
        Index = MainStack->addWidget(std::make_unique<IndexWidget>());
    if (SearchField)
        SearchField->hide();
    Index->show();
    MainStack->setCurrentWidget(Index);
}
