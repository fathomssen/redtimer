function Component()
{
}

Component.prototype.createOperations = function()
{
    component.createOperations();

    if (systemInfo.productType === "windows")
    {
        component.addOperation("CreateShortcut", "@TargetDir@/redtimer.exe", "@StartMenuDir@/RedTimer.lnk",
            "workingDirectory=@TargetDir@", "iconPath=@TargetDir@/redtimer.exe",
            "iconId=1", "description=Redmine Time Tracker");
    }
}
