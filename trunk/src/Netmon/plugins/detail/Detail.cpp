#include "stdafx.h"
#include "Detail.h"

DetailPlugin::DetailPlugin()
{
    model = new DetailModel();
    view = new DetailView(model);
}

DetailPlugin::~DetailPlugin()
{
    delete model;
    delete view;
}

void DetailPlugin::InsertPacket(PacketInfoEx *pi)
{
    model->InsertPacket(pi);
}

void DetailPlugin::SetProcess(int puid)
{
    view->SetProcess(puid);
}

void DetailPlugin::SaveDatabase()
{
    model->SaveDatabase();
}

void DetailPlugin::ClearDatabase()
{
    // TODO: Implement this method
}

DLGPROC DetailPlugin::GetDialogProc()
{
    return view->DlgProc;
}

TCHAR * DetailPlugin::GetTemplateName()
{
    return TEXT("DLG_DETAIL");
}
