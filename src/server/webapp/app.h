#pragma once

#include <Wt/WApplication.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WEnvironment.h>

#include "widgets/Index.h"

#include "user.h"

class AttendanceApplication : public Wt::WApplication
{
public:
    AttendanceApplication(const Wt::WEnvironment &env);

    bool isAuthorized() const { return user.id != 0; }
    const Person &getUser() const { return user; }

    Wt::WLineEdit *SearchField = nullptr;

private:
    Wt::WStackedWidget *MainStack = nullptr;
    IndexWidget *Index = nullptr;
    SignInWidget *SignIn = nullptr;
    SignUpWidget *SignUp = nullptr;

    Person user;

    void setContents();
    void handleInternalPath(const std::string &internalPath);
};
