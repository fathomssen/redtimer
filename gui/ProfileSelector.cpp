#include "qtredmine/Logging.h"

#include "ProfileSelector.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSettings>

using namespace qtredmine;
using namespace std;

namespace redtimer {

ProfileSelector::ProfileSelector( const QStringList& ids, MainWindow* mainWindow )
    : Window( "ProfileSelector", mainWindow ),
      profileIds_( !ids.isEmpty() ? ids : profileIds() )
{
    ENTER()(ids)(profileIds_);

    if( profileIds_.count() == 0 )
        RETURN();

    // Profile selector window initialisation
    setModality( Qt::ApplicationModal );
    setFlags( Qt::Dialog );
    setTitle( "RedTimer - Select a profile" );

    // Set the models
    setCtxProperty( "profileModel", &profileModel_ );

    // Connect the profile selected signal to the profileSelected slot
    connect( qml("profile"), SIGNAL(activated(int)), this, SLOT(profileSelected(int)) );

    // Connect the create button clicked signal to the save slot
    connect( qml("ok"), SIGNAL(clicked()), this, SLOT(apply()) );

    // Connect the cancel button clicked signal to the close slot
    connect( qml("cancel"), SIGNAL(clicked()), this, SLOT(close()) );

    // Connect the closed signal to the close slot
    connect( this, &Window::closed, [=](){ close(); } );

    RETURN();
}

void
ProfileSelector::apply()
{
    ENTER();

    emit applied();

    emitClosedSignal_ = false;
    Window::close();

    RETURN();
}

void
ProfileSelector::display()
{
    ENTER();

    int id = 0;
    for( const auto& profileId : profileIds_ )
        profileModel_.push_back( SimpleItem(id++, profileId) );

    DEBUG()(profileModel_);

    qml("profile")->setProperty( "currentIndex", -1 );
    qml("profile")->setProperty( "currentIndex", 0 );

    profileId_ = profileIds_[0];
    emit selected( profileId_ );

    show();

    RETURN();
}

QStringList
ProfileSelector::profileIds()
{
    ENTER();

    QSettings settings( QSettings::IniFormat, QSettings::UserScope, "Thomssen IT", "RedTimer" );

    QStringList profileIds;
    for( const auto& group : settings.childGroups() )
    {
        QRegularExpressionMatch match = QRegularExpression("profile-(\\d+)").match( group );

        // Not a profile group entry
        if( !match.hasMatch() )
            continue;

        profileIds.push_back( settings.value(group+"/name").toString() );
    }

    RETURN( profileIds );
}

void
ProfileSelector::profileSelected( int index )
{
    ENTER();

    profileId_ = profileModel_.at(index).name();
    DEBUG()(index)(profileId_);

    emit selected( profileId_ );

    RETURN();
}

} // redtimer
