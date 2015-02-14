#include "stdafx.h"
#include "Month.h"

MonthPlugin::MonthPlugin()
{
    model = new MonthModel();
    view = new MonthView(model);
}

MonthPlugin::~MonthPlugin()
{
    delete model;
    delete view;
}

void MonthPlugin::InsertPacket(PacketInfoEx *pi)
{
    model->InsertPacket(pi);
}

void MonthPlugin::SetProcess(int puid)
{
    view->SetProcess(puid);
}

void MonthPlugin::SaveDatabase()
{
    model->SaveDatabase();
}

void MonthPlugin::ClearDatabase()
{
    // TODO: Implement this method
}

DLGPROC MonthPlugin::GetDialogProc()
{
    return view->DlgProc;
}

TCHAR * MonthPlugin::GetTemplateName()
{
    return TEXT("DLG_MONTH");
}
