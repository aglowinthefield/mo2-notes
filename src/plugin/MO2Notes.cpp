#include "MO2Notes.h"
#include "gui/NotesWidget.h"

// #include "BSPluginList/PluginsWidget.h"
// #include "Settings.h"

using namespace Qt::Literals::StringLiterals;

bool MO2Notes::initPlugin(MOBase::IOrganizer* organizer)
{
    m_Organizer = organizer;
    //  Settings::init(organizer);
    return true;
}

QString MO2Notes::name() const { return NAME; }

std::vector<std::shared_ptr<const MOBase::IPluginRequirement> > MO2Notes::requirements() const
{
    return { Requirements::gameDependency({ u"Oblivion"_s, u"Fallout 3"_s, u"New Vegas"_s, u"Skyrim"_s, u"Enderal"_s,
                                            u"Fallout 4"_s, u"Skyrim Special Edition"_s, u"Enderal Special Edition"_s,
                                            u"Skyrim VR"_s, u"Fallout 4 VR"_s,
                                            u"Starfield"_s }) };
}

QString MO2Notes::author() const { return u"aglowinthefield"_s; }

QString MO2Notes::description() const { return tr("A handy notes panel for MO2"); }

MOBase::VersionInfo MO2Notes::version() const
{
    return MOBase::VersionInfo(0, 0, 1, 0, MOBase::VersionInfo::RELEASE_ALPHA);
}

QList<MOBase::PluginSetting> MO2Notes::settings() const
{
    return {
        //      {u"enable_sort_button"_s, u"Enable the Sort button in the Plugins panel"_s, true},
        //      {u"external_change_warning"_s,
        //       u"Warn if load order changes while running an executable"_s, true},
        //      {u"loot_show_dirty"_s,
        //       u"LOOT: Show information about plugins that can be cleaned"_s, true},
        //      {u"loot_show_messages"_s,
        //       u"LOOT: Show general information and warning messages"_s, true},
        //      {u"loot_show_problems"_s,
        //       u"LOOT: Show information about incompatibilities and missing masters"_s, true},
    };
}

bool MO2Notes::enabledByDefault() const { return true; }

QWidget* MO2Notes::createWidget(IPanelInterface*, QWidget* parent)
{
    const auto widget = new NotesWidget(parent);
    return widget;
}

QString MO2Notes::label() const { return tr("Notes"); }

IPluginPanel::Position MO2Notes::position() const { return Position::atEnd(); }
