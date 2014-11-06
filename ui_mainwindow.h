/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QLabel *label;
    QLabel *label_3;
    QGroupBox *loginout;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *usr;
    QLineEdit *pw;
    QPushButton *login;
    QGroupBox *fservergroup;
    QPushButton *FileSend;
    QLineEdit *ip_addr;
    QLineEdit *ip_port;
    QLabel *label_6;
    QLabel *label_7;
    QGroupBox *camera;
    QPushButton *PlayBut;
    QPushButton *actionRecord;
    QTextEdit *NameEdit;
    QPushButton *SavePic;
    QLabel *label_2;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(660, 760);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        label = new QLabel(centralWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setEnabled(true);
        label->setGeometry(QRect(0, 0, 640, 480));
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QStringLiteral("label_3"));
        label_3->setEnabled(true);
        label_3->setGeometry(QRect(650, 0, 640, 480));
        loginout = new QGroupBox(centralWidget);
        loginout->setObjectName(QStringLiteral("loginout"));
        loginout->setGeometry(QRect(330, 610, 321, 91));
        loginout->setCheckable(false);
        label_4 = new QLabel(loginout);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(10, 30, 68, 17));
        label_5 = new QLabel(loginout);
        label_5->setObjectName(QStringLiteral("label_5"));
        label_5->setGeometry(QRect(10, 60, 68, 17));
        usr = new QLineEdit(loginout);
        usr->setObjectName(QStringLiteral("usr"));
        usr->setGeometry(QRect(80, 30, 113, 27));
        pw = new QLineEdit(loginout);
        pw->setObjectName(QStringLiteral("pw"));
        pw->setGeometry(QRect(80, 60, 113, 27));
        pw->setInputMethodHints(Qt::ImhHiddenText|Qt::ImhNoAutoUppercase|Qt::ImhNoPredictiveText|Qt::ImhSensitiveData);
        pw->setEchoMode(QLineEdit::Password);
        login = new QPushButton(loginout);
        login->setObjectName(QStringLiteral("login"));
        login->setGeometry(QRect(200, 30, 111, 51));
        QFont font;
        font.setBold(true);
        font.setUnderline(true);
        font.setWeight(75);
        login->setFont(font);
        fservergroup = new QGroupBox(centralWidget);
        fservergroup->setObjectName(QStringLiteral("fservergroup"));
        fservergroup->setGeometry(QRect(330, 500, 321, 80));
        FileSend = new QPushButton(fservergroup);
        FileSend->setObjectName(QStringLiteral("FileSend"));
        FileSend->setEnabled(false);
        FileSend->setGeometry(QRect(200, 20, 111, 51));
        ip_addr = new QLineEdit(fservergroup);
        ip_addr->setObjectName(QStringLiteral("ip_addr"));
        ip_addr->setEnabled(false);
        ip_addr->setGeometry(QRect(42, 20, 151, 27));
        ip_port = new QLineEdit(fservergroup);
        ip_port->setObjectName(QStringLiteral("ip_port"));
        ip_port->setEnabled(false);
        ip_port->setGeometry(QRect(42, 50, 151, 27));
        label_6 = new QLabel(fservergroup);
        label_6->setObjectName(QStringLiteral("label_6"));
        label_6->setGeometry(QRect(10, 20, 31, 17));
        label_7 = new QLabel(fservergroup);
        label_7->setObjectName(QStringLiteral("label_7"));
        label_7->setGeometry(QRect(10, 50, 41, 17));
        camera = new QGroupBox(centralWidget);
        camera->setObjectName(QStringLiteral("camera"));
        camera->setGeometry(QRect(10, 500, 300, 191));
        PlayBut = new QPushButton(camera);
        PlayBut->setObjectName(QStringLiteral("PlayBut"));
        PlayBut->setEnabled(false);
        PlayBut->setGeometry(QRect(0, 20, 300, 27));
        actionRecord = new QPushButton(camera);
        actionRecord->setObjectName(QStringLiteral("actionRecord"));
        actionRecord->setEnabled(false);
        actionRecord->setGeometry(QRect(0, 100, 300, 81));
        actionRecord->setIconSize(QSize(32, 32));
        NameEdit = new QTextEdit(camera);
        NameEdit->setObjectName(QStringLiteral("NameEdit"));
        NameEdit->setEnabled(false);
        NameEdit->setGeometry(QRect(80, 60, 111, 31));
        SavePic = new QPushButton(camera);
        SavePic->setObjectName(QStringLiteral("SavePic"));
        SavePic->setEnabled(false);
        SavePic->setGeometry(QRect(200, 60, 100, 31));
        label_2 = new QLabel(camera);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(0, 60, 81, 17));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 660, 25));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        label->setText(QString());
        label_3->setText(QString());
        loginout->setTitle(QApplication::translate("MainWindow", "Login", 0));
        label_4->setText(QApplication::translate("MainWindow", "UserName", 0));
        label_5->setText(QApplication::translate("MainWindow", "Password", 0));
        login->setText(QApplication::translate("MainWindow", "Login", 0));
        fservergroup->setTitle(QApplication::translate("MainWindow", "File Server Setting", 0));
        FileSend->setText(QApplication::translate("MainWindow", "Sending", 0));
        label_6->setText(QApplication::translate("MainWindow", "IP", 0));
        label_7->setText(QApplication::translate("MainWindow", "Port", 0));
        camera->setTitle(QApplication::translate("MainWindow", "Camera", 0));
        PlayBut->setText(QApplication::translate("MainWindow", "Start", 0));
        actionRecord->setText(QApplication::translate("MainWindow", "STOP Rec", 0));
        SavePic->setText(QApplication::translate("MainWindow", "Save Picture", 0));
        label_2->setText(QApplication::translate("MainWindow", "File name", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
