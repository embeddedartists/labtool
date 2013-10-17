/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "uimainwindow.h"

#include <QDebug>
#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QMenuBar>
#include <QSettings>
#include <QToolBar>
#include <QMessageBox>
#include <QTabWidget>

#include <QDir>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFile>
#include <QDataStream>

#include "common/configuration.h"
#include "device/devicemanager.h"


/*!
    \class UiMainWindow
    \brief The UiMainWindow class is the main Window for this application.

    \ingroup main

    The UiMainWindow class is the starting point for this application.
    It will be created and initialized in main.cpp and is responsible
    for setting up the User Interface and load stored project
    settings.
*/

/*!
    Constructs the main window with the given \a parent. Initializes the
    User Interface and loads the last saved project settings (if any).
*/
UiMainWindow::UiMainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    // default file name for project settings

#if QT_VERSION >= 0x050000
    mProjectFile = QStandardPaths::writableLocation(
                QStandardPaths::DataLocation)
            + QDir::separator() + Configuration::ProjectFilename;
#else
    mProjectFile = QDesktopServices::storageLocation(
                QDesktopServices::DataLocation)
            + QDir::separator() + Configuration::ProjectFilename;
#endif


    mAppState = AppStateCapture;
    mCaptureTabIdx = 0;

    // Deallocation:
    //
    // In Qt, QObjects can organize themselves in object trees. When a parent
    // is deallocated all its children will also be deallocated.
    //
    // URL: http://qt-project.org/doc/qt-4.8/objecttrees.html
    //
    // When we are not using Qt's object tree during allocation or where
    // it isn't clear that object trees are used we will describe the
    // deallocation responsibility.
    mGenerator = new GeneratorApp(this, this);
    mCapture = new CaptureApp(this, this);

    createMenubar();
    createToolbar();
    createCentralWidget();

    loadSettings();
}

/*!
    This event handler is called with the given \a event when the
    application closes. The latest project settings will be saved
    to persistent storage.
*/
void UiMainWindow::closeEvent(QCloseEvent *event)
{
    saveSettings();
    QMainWindow::closeEvent(event);
}


/*
    ---------------------------------------------------------------------------
    #### Private methods
    ---------------------------------------------------------------------------
*/


/*!
    The active device will be changed to \a device and
    the UI update to reflect this change.
*/
void UiMainWindow::changeToDevice(Device* device)
{
    DeviceManager::instance().setActiveDevice(device);

    // Make sure the device is selected in the device menu
    QAction* action = findChild<QAction*>(device->name());
    if (action != NULL) {
        action->setChecked(true);
    }

    mDeviceLabel->setText(device->name());    

    // Enable supported app states (capture | generator)
    QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget());
    tabWidget->setTabEnabled(mCaptureTabIdx, device->supportsCaptureDevice());
    tabWidget->setTabEnabled(mGeneratorTabIdx,
                             device->supportsGeneratorDevice());

    changeDeviceStatus(device);

    mCapture->handleDeviceChanged(device);
    mGenerator->handleDeviceChanged(device);
}

/*!
    Changes the color scheme to the scheme with name \a scheme.
*/
void UiMainWindow::changeToScheme(QString &scheme)
{
    Configuration::instance().loadColorScheme(scheme);

    // Make sure the scheme is selected in the menu
    QAction* action = findChild<QAction*>(scheme);
    if (action != NULL) {
        action->setChecked(true);
    }
}

/*!
    Entry point to create the application menu bar
*/
void UiMainWindow::createMenubar()
{
    createFileMenu();
    createDeviceMenu();

    menuBar()->addMenu(mCapture->menu());

    createOptionMenu();
    createHelpMenu();
}

/*!
    Create the 'File' menu.
*/
void UiMainWindow::createFileMenu()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));

    //
    //  New
    //
    QAction* action = new QAction(tr("&New"), this);
    action->setShortcut(tr("CTRL+N"));
    action->setToolTip(tr("Create a new project"));
    connect(action, SIGNAL(triggered()), this, SLOT(newProject()));
    fileMenu->addAction(action);

    //
    //  Open
    //
    action = new QAction(tr("&Open"), this);
    action->setShortcut(tr("CTRL+O"));
    action->setToolTip(tr("Open an existing project"));
    connect(action, SIGNAL(triggered()), this, SLOT(openProject()));
    fileMenu->addAction(action);

    //
    //  Save
    //
    action = new QAction(tr("&Save"), this);
    action->setShortcut(tr("CTRL+S"));
    action->setToolTip(tr("Save the project"));
    connect(action, SIGNAL(triggered()), this, SLOT(saveProject()));
    fileMenu->addAction(action);

    //
    //  Save As
    //
    action = new QAction(tr("Save &As"), this);
    action->setToolTip(tr("Save the project as..."));
    connect(action, SIGNAL(triggered()), this, SLOT(saveProjectAs()));
    fileMenu->addAction(action);

    fileMenu->addSeparator();

    //
    //  Exit
    //
    action = new QAction(tr("E&xit"), this);
    action->setShortcuts(QKeySequence::Quit);
    action->setToolTip(tr("Exit application"));
    connect(action, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction(action);
}

/*!
    Create the 'Device' menu.
*/
void UiMainWindow::createDeviceMenu()
{
    mDeviceMenu = menuBar()->addMenu(tr("&Devices"));

    //
    //  Add supported devices to the device menu
    //

    QActionGroup* deviceGroup = new QActionGroup(this);

    QList<Device*> devices = DeviceManager::instance().devices();
    for (int i = 0; i < devices.size(); i++) {
        Device* dev = devices.at(i);
        QAction* action = new QAction(dev->name(), this);
        action->setData(dev->name());
        action->setObjectName(dev->name());

        connect(action, SIGNAL(triggered()), this, SLOT(changeDevice()));
        connect(dev, SIGNAL(availableStatusChanged(Device*)),
                this, SLOT(changeDeviceStatus(Device*)));

        action->setCheckable(true);

        if (!dev->isAvailable()) {
            action->setEnabled(false);
        }

        mDeviceMenu->addAction(action);
        deviceGroup->addAction(action);
    }

    deviceGroup->setExclusive(true);
}

/*!
    Create the 'Options' menu.
*/
void UiMainWindow::createOptionMenu()
{
    QMenu* menu = menuBar()->addMenu(tr("&Options"));
    menu->setObjectName(QString::fromUtf8("optionsMenu"));

    //
    // Color scheme sub menu
    //

    mColorSchemeMenu = menu->addMenu(tr("Color scheme"));

    //
    // Add supported schemes to sub menu
    //

    QActionGroup* schemeGroup = new QActionGroup(this);

    QList<QString> schemes = Configuration::instance().colorSchemes();
    for (int i = 0; i < schemes.size(); i++) {
        QString scheme = schemes.at(i);
        QAction* action = new QAction(scheme, this);
        action->setData(scheme);
        action->setObjectName(scheme);
        connect(action, SIGNAL(triggered()), this, SLOT(changeColorScheme()));

        action->setCheckable(true);
        if (scheme == Configuration::instance().activeColorScheme()) {
            action->setChecked(true);
        }

        mColorSchemeMenu->addAction(action);
        schemeGroup->addAction(action);
    }

    schemeGroup->setExclusive(true);
}

/*!
    Create the 'Help' menu.
*/
void UiMainWindow::createHelpMenu()
{
    QMenu* menu = menuBar()->addMenu(tr("&Help"));

    //  About

    QAction* action = new QAction(tr("A&bout"), this);
    action->setToolTip(tr("About"));
    connect(action, SIGNAL(triggered()), this, SLOT(about()));
    menu->addAction(action);

}

/*!
    Updates the menu bar to reflect the current application state. The
    content of the menu bar is context sensitive and depends on Capture
    or Generator being active.
*/
void UiMainWindow::updateMenu()
{

    QMenu* captureMenu = mCapture->menu();

    switch(mAppState) {
    case AppStateCapture:
        captureMenu->menuAction()->setVisible(true);

        break;
    case AppStateGenerator:
        captureMenu->menuAction()->setVisible(false);

        break;
    default:
        break;
    }
}

/*!
    Entry point to create the application toolbar
*/
void UiMainWindow::createToolbar()
{
    createDeviceToolbar();

    addToolBar(mCapture->toolBar());
    addToolBar(mGenerator->toolBar());
}

/*!
    Creates the 'Device' toolbar to show the name of the
    active device.
*/
void UiMainWindow::createDeviceToolbar()
{
    QToolBar* toolBar = addToolBar(tr("Device toolbar"));
    toolBar->setObjectName(QString::fromUtf8("deviceToolbar"));

    // Deallocation:
    //   The toolbar will take ownership of mDeviceLabel when calling
    //   addWidget below.
    mDeviceLabel = new QLabel();
    mDeviceLabel->setToolTip("Selected device");
    toolBar->addWidget(mDeviceLabel);
}

/*!
    Updates the toolbar to reflect the current application state. The
    content of the toolbar is context sensitive and depends on Capture
    or Generator being active.
*/
void UiMainWindow::updateToolbar()
{
    switch(mAppState) {
    case AppStateCapture:

        mCapture->toolBar()->setVisible(true);
        mCapture->toolBar()->setEnabled(true);

        mGenerator->toolBar()->setVisible(false);
        mGenerator->toolBar()->setEnabled(false);

        break;
    case AppStateGenerator:

        mCapture->toolBar()->setVisible(false);
        mCapture->toolBar()->setEnabled(false);

        mGenerator->toolBar()->setVisible(true);
        mGenerator->toolBar()->setEnabled(true);

        break;
    default:
        break;
    }

}

/*!
    Create and set the central widget of this application.
*/
void UiMainWindow::createCentralWidget()
{
    QTabWidget* tabWidget = new QTabWidget(this);
    connect(tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(handleTabChanged(int)));

    setCentralWidget(tabWidget);

    mCaptureTabIdx = tabWidget->addTab(mCapture->captureArea(), tr("Capture"));
    mGeneratorTabIdx = tabWidget->addTab(mGenerator->generatorArea(),
                                         tr("Generator"));
}

/*!
    Save project settings to persitent storage.
*/
void UiMainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue("mainwindow/size", size());
    settings.setValue("mainwindow/pos", pos());
    settings.setValue("mainwindow/lastproject", mProjectFile);

    saveProject(mProjectFile);
}

/*!
    Load project settings from persitent storage.
*/
void UiMainWindow::loadSettings()
{

    QSettings settings;

    QDesktopWidget* desktop = QApplication::desktop();
    QRect desktopGeom = desktop->geometry();

    QPoint pos = settings.value("mainwindow/pos", QPoint(0,0)).toPoint();
    QSize winSize = settings.value("mainwindow/size", QSize(600,400)).toSize();

    // move to 0,0 if outside of desktop area
    if (!desktopGeom.contains(pos, true)) {
        pos = QPoint(0,0);
    }

    // too small window -> increase size
    if (winSize.width() < 200 || winSize.height() < 200) {
        winSize.setWidth(600);
        winSize.setHeight(400);
    }
    resize(winSize);
    move(pos);


    // mProjectFile is initialized to the default path in the constructor
    QString lastProject = settings.value("mainwindow/lastproject",
                                         mProjectFile).toString();
    setActiveProjectFile(lastProject);
    openProject(lastProject);
}

/*!
    Open and load project settings from the file with name \a projectFile.
*/
void UiMainWindow::openProject(QString projectFile)
{    
    QSettings proj(projectFile, QSettings::IniFormat);

    // ###### app state ######
    int as = proj.value("appState", AppStateCapture).toInt();
    if (as >= 0 && as < AppStateNum) {
        mAppState = (AppState) as;

        QTabWidget* tabWidget = qobject_cast<QTabWidget*>(centralWidget());
        switch(mAppState) {
        case AppStateCapture:
            tabWidget->setCurrentIndex(mCaptureTabIdx);
            handleTabChanged(mCaptureTabIdx);
            break;
        case AppStateGenerator:
            tabWidget->setCurrentIndex(mGeneratorTabIdx);
            handleTabChanged(mGeneratorTabIdx);
            break;
        default:
            break;
        }

    }

    // ###### device settings ######

    QList<Device*> devices = DeviceManager::instance().devices();
    proj.beginGroup("device");
    // first device in device list is considered default device
    QString deviceName = proj.value("name", devices.at(0)->name()).toString();
    proj.endGroup();

    Device* device = NULL;

    for (int i = 0; i < devices.size(); i++) {
        device = devices.at(i);
        if (device->name() == deviceName) {
            changeToDevice(device);
            break;
        }
    }

    // ###### color scheme ######
    proj.beginGroup("colorScheme");
    QString scheme = proj.value("scheme", Configuration::instance()
                                .activeColorScheme()).toString();
    changeToScheme(scheme);
    proj.endGroup();


    // ###### capture settings ######
    mCapture->openProject(proj);

    // ###### generator settings ######
    mGenerator->openProject(proj);
}

/*!
    Save project settings to the file with name \a projectFile.
*/
void UiMainWindow::saveProject(QString projectFile)
{
    QSettings proj(projectFile, QSettings::IniFormat);

    // ###### app state ######
    proj.setValue("appState", mAppState);

    // ###### device settings ######

    Device* device = DeviceManager::instance().activeDevice();
    proj.beginGroup("device");
    proj.setValue("name", device->name());
    proj.endGroup();

    // ###### color scheme ######
    proj.beginGroup("colorScheme");
    proj.setValue("scheme", Configuration::instance().activeColorScheme());
    proj.endGroup();


    // ###### capture settings ######
    mCapture->saveProject(proj);

    // ###### generator settings ######
    mGenerator->saveProject(proj);

}

/*!
    Change the current project file \a file
*/
void UiMainWindow::setActiveProjectFile(QString file)
{
    mProjectFile = file;
    setWindowTitle(QCoreApplication::applicationName() + " - " + file);
}

/*!
    Returns true if the user was warned about an active state (capture
    or generation) when trying to do an action described by \a action.
*/
bool UiMainWindow::warnedAboutActiveState(QString action)
{
    if (mCapture->hasActiveState() || mGenerator->hasActiveState()) {

        QString msg = QString("Please stop the ongoing %1 before %2!")
                .arg(mCapture->hasActiveState() ? "Capture" : "Signal generation")
                .arg(action);

        QMessageBox::warning(this,
                             "Stop ongoing action!",
                             msg);


        return true;
    }

    return false;
}

/*!
    Handle that the active tab in the tab widget has changed to tab with
    \a index. For this application that means that the state has changed
    (capture | generate).
*/
void UiMainWindow::handleTabChanged(int index)
{
    if (mCaptureTabIdx == index) {
        mAppState = AppStateCapture;
    }
    else {
        mAppState = AppStateGenerator;
    }

    updateMenu();
    updateToolbar();
}

/*!
    Called when a user changes the device
*/
void UiMainWindow::changeDevice()
{

    QList<QAction*> list = mDeviceMenu->actions();
    QString deviceName = "";

    // find name of selected device in device menu
    for (int i = 0; i < list.size(); i++) {
        QAction* action = list.at(i);
        if (action->isChecked()) {
            deviceName = action->data().toString();
            break;
        }
    }

    // find the device instance
    Device* device = NULL;
    QList<Device*> deviceList = DeviceManager::instance().devices();
    for (int i = 0; i < deviceList.size(); i++) {

        if (deviceList.at(i)->name() == deviceName) {
            device = deviceList.at(i);
            break;
        }
    }

    do {
        if (device == NULL) break;
        // the active device has been reselected -> nothing to do.
        if (device == DeviceManager::instance().activeDevice()) break;

        if (warnedAboutActiveState("changing device")) {

            // change back the active device in the device menu
            QAction* action = findChild<QAction*>(DeviceManager::instance()
                                                  .activeDevice()->name());
            if (action != NULL) {
                action->setChecked(true);
            }

            break;
        }

        changeToDevice(device);

    } while (false);


    updateToolbar();
}

/*!
    Called when the device status is changed.
*/
void UiMainWindow::changeDeviceStatus(Device *device)
{

    QAction* action = findChild<QAction*>(device->name());
    if (action != NULL) {
        action->setEnabled(device->isAvailable());
    }

    // update the color for the display label
    QPalette palette = mDeviceLabel->palette();
    if (device->isAvailable()) {
        mDeviceLabel->setToolTip(tr("Device is available"));
        palette.setColor(mDeviceLabel->foregroundRole(), Qt::black);
    }
    else {
        mDeviceLabel->setToolTip(tr("Device is NOT available"));
        palette.setColor(mDeviceLabel->foregroundRole(), Qt::red);
    }
    mDeviceLabel->setPalette(palette);


    mCapture->handleDeviceStatusChanged(device);
    mGenerator->handleDeviceStatusChanged(device);
}

/*!
    Called when the user changes the color scheme.
*/
void UiMainWindow::changeColorScheme()
{

    QList<QAction*> list = mColorSchemeMenu->actions();
    QString scheme = "";

    // find selected device in device menu
    for (int i = 0; i < list.size(); i++) {
        QAction* action = list.at(i);
        if (action->isChecked()) {
            scheme = action->data().toString();
            break;
        }
    }

    Configuration::instance().loadColorScheme(scheme);

    mCapture->updateUi();
}

/*!
    Called when the user wants to create a new project.
*/
void UiMainWindow::newProject()
{
    if (warnedAboutActiveState("creating a new project")) return;

    QString name = QFileDialog::getSaveFileName(
                this,
                tr("New Project"),
                QDir::currentPath(),
                "Projects (*.prj)");

    if (!name.isNull() && !name.isEmpty()) {
        setActiveProjectFile(name);

        mCapture->resetProject();
        mGenerator->resetProject();
    }

}

/*!
    Called when the user wants to open a project.
*/
void UiMainWindow::openProject()
{
    if (warnedAboutActiveState("opening a new project")) return;

    QString name = QFileDialog::getOpenFileName(
                this,
                tr("Open Project"),
                QDir::currentPath(),
                "Projects (*.prj)");

    if (!name.isNull() && !name.isEmpty()) {
        setActiveProjectFile(name);
        openProject(name);
    }
}

/*!
    Called when the user wants to save a project.
*/
void UiMainWindow::saveProject()
{
    saveProject(mProjectFile);
}

/*!
    Called when a user wants to save a project with a new file name.
*/
void UiMainWindow::saveProjectAs()
{

    QString name = QFileDialog::getSaveFileName(
                this,
                tr("Save Project As"),
                QDir::currentPath(),
                "Projects (*.prj)");

    if (!name.isNull() && !name.isEmpty()) {
        setActiveProjectFile(name);
        saveProject(name);
    }

}

/*!
    Called when the user clicks the about menu item.
*/
void UiMainWindow::about()
{
    QString progVer = "0.01";
    QString gitCommit = "b248827341fa420f300d098435706d19a8b092b7";

    QString msg = "";
    msg.append("<h2>About ");
    msg.append(QCoreApplication::applicationName());
    msg.append("</h2>");
    msg.append(QString("Version %1<br><br>").arg(progVer));
    msg.append("Built on ");
    msg.append(QString("%1").arg(__DATE__));
    msg.append(" at ");
    msg.append(QString("%1").arg(__TIME__));
    msg.append("   using Qt ");
    msg.append(QT_VERSION_STR);

    msg.append("<br><br>From revision <a href=\"http://github.com/embeddedartists/labtool/commit/");
    msg.append(gitCommit);
    msg.append("\">");
    msg.append(gitCommit.left(10));
    msg.append("</a>");
    msg.append("<br><br>");

    QString url = "http://www.embeddedartists.com/products/app/labtool.php";
    msg.append(QString("User's Guide available on product page: <a href=\"%1\">%2</a>").arg(url).arg(url));
    msg.append("<br><br>");

    msg.append("Copyright 2013 ");
    msg.append(QCoreApplication::organizationName());

    msg.append("<br><br>");

    msg.append("Licensed under the Apache License, Version 2.0 (the \"License\");"
               " you may not use this software except in compliance with the License."
               " You may obtain a copy of the License at"
               "<br><br>"
               "&nbsp;&nbsp;&nbsp;http://www.apache.org/licenses/LICENSE-2.0"
               "<br><br>"
               "Unless required by applicable law or agreed to in writing, software"
               " distributed under the License is distributed on an \"AS IS\" BASIS,"
               " WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied."
               " See the License for the specific language governing permissions and"
               " limitations under the License.");


    QMessageBox::about(this, QString("About %1").arg(
                           QCoreApplication::applicationName()), msg);
}


