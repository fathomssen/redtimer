function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    try
    {
        if( installer.value("os") === "win" )
        {
            component.addOperation( "CreateShortcut", "@TargetDir@/redtimer.exe", "@StartMenuDir@/RedTimer.lnk",
                                    "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/redtimer.exe",
                                    "iconId=1", "description=Redmine Time Tracker");
        }
    }
    catch( e )
    {
        print( e );
    }
}
