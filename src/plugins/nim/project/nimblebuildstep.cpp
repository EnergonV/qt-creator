// Copyright (C) Filippo Cucchetto <filippocucchetto@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0+ OR GPL-3.0 WITH Qt-GPL-exception-1.0

#include "nimblebuildstep.h"

#include "nimconstants.h"
#include "nimbuildsystem.h"
#include "nimoutputtaskparser.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/processparameters.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/runconfigurationaspects.h>

using namespace ProjectExplorer;
using namespace Utils;

namespace Nim {

class NimbleBuildStep : public AbstractProcessStep
{
    Q_DECLARE_TR_FUNCTIONS(Nim::NimbleBuilStep)

public:
    NimbleBuildStep(BuildStepList *parentList, Id id);

    void setupOutputFormatter(OutputFormatter *formatter) final;

private:
    QString defaultArguments() const;
};

NimbleBuildStep::NimbleBuildStep(BuildStepList *parentList, Id id)
    : AbstractProcessStep(parentList, id)
{
    auto arguments = addAspect<ArgumentsAspect>(macroExpander());
    arguments->setSettingsKey(Constants::C_NIMBLEBUILDSTEP_ARGUMENTS);
    arguments->setResetter([this] { return defaultArguments(); });
    arguments->setArguments(defaultArguments());

    setCommandLineProvider([this, arguments] {
        return CommandLine(Nim::nimblePathFromKit(kit()), {"build", arguments->arguments()});
    });
    setWorkingDirectoryProvider([this] { return project()->projectDirectory(); });
    setEnvironmentModifier([this](Environment &env) {
        env.appendOrSetPath(Nim::nimPathFromKit(kit()));
    });

    setSummaryUpdater([this] {
        ProcessParameters param;
        setupProcessParameters(&param);
        return param.summary(displayName());
    });

    QTC_ASSERT(buildConfiguration(), return);
    QObject::connect(buildConfiguration(), &BuildConfiguration::buildTypeChanged,
                     arguments, &ArgumentsAspect::resetArguments);
    QObject::connect(arguments, &ArgumentsAspect::changed,
                     this, &AbstractProcessStep::updateSummary);
}

void NimbleBuildStep::setupOutputFormatter(OutputFormatter *formatter)
{
    const auto parser = new NimParser();
    parser->addSearchDir(project()->projectDirectory());
    formatter->addLineParser(parser);
    AbstractProcessStep::setupOutputFormatter(formatter);
}

QString NimbleBuildStep::defaultArguments() const
{
    if (buildType() == BuildConfiguration::Debug)
        return {"--debugger:native"};
    return {};
}

NimbleBuildStepFactory::NimbleBuildStepFactory()
{
    registerStep<NimbleBuildStep>(Constants::C_NIMBLEBUILDSTEP_ID);
    setDisplayName(NimbleBuildStep::tr("Nimble Build"));
    setSupportedStepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    setSupportedConfiguration(Constants::C_NIMBLEBUILDCONFIGURATION_ID);
    setRepeatable(true);
}

} // Nim
