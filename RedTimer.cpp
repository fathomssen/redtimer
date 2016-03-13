#include "RedTimer.h"

#include "logging.h"

#include <QAbstractButton>
#include <QQuickItem>

using namespace redtimer;
using namespace std;

RedTimer::RedTimer( int argc, char *argv[] )
  : app_( argc, argv )
{
  qEnter() << _(argc) << _(argv);

  settings_ = new Settings( this );

  win_ = new QQuickView();
  win_->setSource( QUrl(QStringLiteral("qrc:/Redtimer.qml")) );
  win_->setModality( Qt::ApplicationModal );

  // Connect the settings button
  connect( win_->rootObject()->findChild<QObject*>("settings"), SIGNAL(clicked()),
           settings_, SLOT(display()) );

  if( !win_->isVisible() )
  {
    qDebug() << "Displaying main window";
    win_->show();
  }

  settings_->load();

//  redmineClient_ = make_shared<RedmineClient>( url_, apikey_ );
}

int
RedTimer::display()
{
  qEnter();
  qReturn( app_.exec() );
}
