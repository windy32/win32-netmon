#include "stdafx.h"
#include "Realtime.h"

RealtimePlugin::RealtimePlugin()
{
    model = new RealtimeModel();
    view = new RealtimeView(model);
}

RealtimePlugin::~RealtimePlugin()
{
    delete model;
    delete view;
}

void RealtimePlugin::InsertPacket(PacketInfoEx *pi)
{
    model->InsertPacket(pi);
}

void RealtimePlugin::SetProcess(int puid)
{
    view->SetProcess(puid);
}

void RealtimePlugin::SaveDatabase()
{
    // nothing to do here
}

void RealtimePlugin::ClearDatabase()
{
    // nothing to do here
}

DLGPROC RealtimePlugin::GetDialogProc()
{
    return view->DlgProc;
}

TCHAR * RealtimePlugin::GetTemplateName()
{
    return TEXT("DLG_REALTIME");
}
