#include "ProfileData.h"

namespace redtimer {

bool
ProfileData::isValid( QString* errmsg ) const
{
    ENTER();

    bool result = !url.isEmpty() && !apiKey.isEmpty();

    if( !result && errmsg )
        *errmsg = "Redmine URL and API key required";

    RETURN( result );
}

} // redtimer
