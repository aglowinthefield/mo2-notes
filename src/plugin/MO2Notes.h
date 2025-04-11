#pragma once

#include "IPluginPanel.h"

class MO2Notes final : public IPluginPanel {
    Q_OBJECT
    Q_INTERFACES(MOBase::IPlugin IPluginPanel)
    Q_PLUGIN_METADATA(IID "org.tannin.MO2Notes" FILE "mo2notes.json")

  public:
    inline static const QString NAME = QStringLiteral("MO2 Notes");

    MO2Notes() = default;

    // IPlugin

    QString name() const override;
    std::vector<std::shared_ptr<const MOBase::IPluginRequirement>> requirements() const override;
    QString author() const override;
    QString description() const override;
    MOBase::VersionInfo version() const override;
    QList<MOBase::PluginSetting> settings() const override;
    bool enabledByDefault() const override;

    // IPluginPanel

    bool initPlugin(MOBase::IOrganizer* organizer) override;
    QWidget* createWidget(IPanelInterface* panelInterface, QWidget* parent) override;
    QString label() const override;
    Position position() const override;

  private:
    MOBase::IOrganizer* m_Organizer;
};