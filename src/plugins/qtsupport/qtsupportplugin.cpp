// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "qtsupportplugin.h"

#include "codegenerator.h"
#include "codegensettingspage.h"
#include "gettingstartedwelcomepage.h"
#include "profilereader.h"
#include "qscxmlcgenerator.h"
#include "qtkitinformation.h"
#include "qtoptionspage.h"
#include "qtoutputformatter.h"
#include "qtsupporttr.h"
#include "qtversionmanager.h"
#include "qtversions.h"
#include "translationwizardpage.h"
#include "uicgenerator.h"

#include <coreplugin/icore.h>
#include <coreplugin/jsexpander.h>

#include <projectexplorer/jsonwizard/jsonwizardfactory.h>
#include <projectexplorer/project.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projecttree.h>
#include <projectexplorer/runcontrol.h>
#include <projectexplorer/session.h>
#include <projectexplorer/target.h>

#include <utils/infobar.h>
#include <utils/macroexpander.h>

using namespace Core;
using namespace ProjectExplorer;

namespace QtSupport {
namespace Internal {

class QtSupportPluginPrivate
{
public:
    QtVersionManager qtVersionManager;

    DesktopQtVersionFactory desktopQtVersionFactory;
    EmbeddedLinuxQtVersionFactory embeddedLinuxQtVersionFactory;

    CodeGenSettingsPage codeGenSettingsPage;
    QtOptionsPage qtOptionsPage;

    ExamplesWelcomePage examplesPage{true};
    ExamplesWelcomePage tutorialPage{false};

    QtKitAspect qtKiAspect;

    QtOutputFormatterFactory qtOutputFormatterFactory;

    UicGeneratorFactory uicGeneratorFactory;
    QScxmlcGeneratorFactory qscxmlcGeneratorFactory;
};

QtSupportPlugin::~QtSupportPlugin()
{
    delete d;
}

bool QtSupportPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments)
    Q_UNUSED(errorMessage)
    QMakeParser::initialize();
    ProFileEvaluator::initialize();
    new ProFileCacheManager(this);

    JsExpander::registerGlobalObject<CodeGenerator>("QtSupport");
    ProjectExplorer::JsonWizardFactory::registerPageFactory(new TranslationWizardPageFactory);
    ProjectExplorerPlugin::showQtSettings();

    d = new QtSupportPluginPrivate;

    QtVersionManager::initialized();

    return true;
}

const char kLinkWithQtInstallationSetting[] = "LinkWithQtInstallation";

static void askAboutQtInstallation()
{
    // if the install settings exist, the Qt Creator installation is (probably) already linked to
    // a Qt installation, so don't ask
    if (!QtOptionsPage::canLinkWithQt() || QtOptionsPage::isLinkedWithQt()
        || !ICore::infoBar()->canInfoBeAdded(kLinkWithQtInstallationSetting))
        return;

    Utils::InfoBarEntry info(
        kLinkWithQtInstallationSetting,
        Tr::tr(
            "Link with a Qt installation to automatically register Qt versions and kits? To do "
            "this later, select Edit > Preferences > Kits > Qt Versions > Link with Qt."),
        Utils::InfoBarEntry::GlobalSuppression::Enabled);
    info.addCustomButton(Tr::tr("Link with Qt"), [] {
        ICore::infoBar()->removeInfo(kLinkWithQtInstallationSetting);
        QTimer::singleShot(0, ICore::dialogParent(), &QtOptionsPage::linkWithQt);
    });
    ICore::infoBar()->addInfo(info);
}

void QtSupportPlugin::extensionsInitialized()
{
    Utils::MacroExpander *expander = Utils::globalMacroExpander();

    static const auto currentQtVersion = []() -> const QtVersion * {
        ProjectExplorer::Project *project = ProjectExplorer::ProjectTree::currentProject();
        if (!project || !project->activeTarget())
            return nullptr;
        return QtKitAspect::qtVersion(project->activeTarget()->kit());
    };
    static const char kCurrentHostBins[] = "CurrentDocument:Project:QT_HOST_BINS";
    expander->registerVariable(
        kCurrentHostBins,
        Tr::tr("Full path to the host bin directory of the Qt version in the active kit "
               "of the project containing the current document."),
        []() {
            const QtVersion * const qt = currentQtVersion();
            return qt ? qt->hostBinPath().toUserOutput() : QString();
        });

    expander->registerVariable(
        "CurrentDocument:Project:QT_INSTALL_BINS",
        Tr::tr("Full path to the target bin directory of the Qt version in the active kit "
               "of the project containing the current document.<br>You probably want %1 instead.")
            .arg(QString::fromLatin1(kCurrentHostBins)),
        []() {
            const QtVersion * const qt = currentQtVersion();
            return qt ? qt->binPath().toUserOutput() : QString();
        });

    expander->registerVariable(
        "CurrentDocument:Project:QT_HOST_LIBEXECS",
        Tr::tr("Full path to the host libexec directory of the Qt version in the active kit "
               "of the project containing the current document."),
        []() {
            const QtVersion *const qt = currentQtVersion();
            return qt ? qt->hostLibexecPath().toUserOutput() : QString();
        });

    static const auto activeQtVersion = []() -> const QtVersion * {
        ProjectExplorer::Project *project = SessionManager::startupProject();
        if (!project || !project->activeTarget())
            return nullptr;
        return QtKitAspect::qtVersion(project->activeTarget()->kit());
    };
    static const char kActiveHostBins[] = "ActiveProject:QT_HOST_BINS";
    expander->registerVariable(
        kActiveHostBins,
        Tr::tr("Full path to the host bin directory of the Qt version in the active kit "
               "of the active project."),
        []() {
            const QtVersion * const qt = activeQtVersion();
            return qt ? qt->hostBinPath().toUserOutput() : QString();
        });

    expander->registerVariable(
        "ActiveProject:QT_INSTALL_BINS",
        Tr::tr("Full path to the target bin directory of the Qt version in the active kit "
               "of the active project.<br>You probably want %1 instead.")
            .arg(QString::fromLatin1(kActiveHostBins)),
        []() {
            const QtVersion * const qt = activeQtVersion();
            return qt ? qt->binPath().toUserOutput() : QString();
        });

    expander->registerVariable(
        "ActiveProject::QT_HOST_LIBEXECS",
        Tr::tr("Full path to the libexec bin directory of the Qt version in the active kit "
               "of the active project."),
        []() {
            const QtVersion *const qt = activeQtVersion();
            return qt ? qt->hostLibexecPath().toUserOutput() : QString();
        });

    askAboutQtInstallation();
}

} // Internal
} // QtSupport
