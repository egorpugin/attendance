#pragma once

#include <primitives/filesystem.h>

#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLabel.h>

namespace Wt
{

class WLabel;
class WLineEdit;
class WCheckBox;
class WTextArea;

}

class CenterContentWidget : public Wt::WContainerWidget
{
public:
    CenterContentWidget();
};

template <class W>
struct LabeledWidget : public Wt::WContainerWidget
{
    Wt::WLabel *Label;
    W *Widget;

    LabeledWidget(const Wt::WString &label)
    {
        constexpr auto fw = std::is_base_of_v<Wt::WFormWidget, W>;

        Label = addWidget(std::make_unique<Wt::WLabel>(label));
        if constexpr (!fw)
        {
            //addWidget(std::make_unique<Wt::WBreak>());
            //addWidget(std::make_unique<Wt::WBreak>());
            Label->setAttributeValue("style", "font-weight: bold; margin-bottom: 5px;");
        }
        Widget = addWidget(std::make_unique<W>());
        if constexpr (fw)
            Label->setBuddy(Widget);
        addWidget(std::make_unique<Wt::WBreak>());
    }
};

struct FormLineEdit : LabeledWidget<Wt::WLineEdit>
{
    FormLineEdit(const Wt::WString &label, const Wt::WString &placeholder = Wt::WString())
        : LabeledWidget(label)
    {
        Widget->setPlaceholderText(placeholder);
    }

    auto text() const
    {
        return Widget->text();
    }
};

class IndexWidget : public CenterContentWidget
{
public:
    IndexWidget();

    void show();

private:
};

class SignInWidget : public Wt::WContainerWidget
{
public:
    SignInWidget();

private:
    FormLineEdit *Name;
    FormLineEdit *Password;
    Wt::WCheckBox *RememberMe;
    Wt::WLabel *Error;
};

class SignUpWidget : public Wt::WContainerWidget
{
public:
    SignUpWidget();

private:
    Wt::WLabel *Error;
};

struct UserWidget : public Wt::WContainerWidget
{
    void setUser(const String &u);
};
