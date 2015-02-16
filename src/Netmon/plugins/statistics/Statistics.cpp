#include "stdafx.h"
#include "Statistics.h"

StatisticsPlugin::StatisticsPlugin()
{
    model = new StatisticsModel();
    view = new StatisticsView(model);
}

StatisticsPlugin::~StatisticsPlugin()
{
    delete model;
    delete view;
}

void StatisticsPlugin::InsertPacket(PacketInfoEx *pi)
{
    model->InsertPacket(pi);
}

void StatisticsPlugin::SetProcess(int puid)
{
    view->SetProcess(puid);
}

void StatisticsPlugin::SaveDatabase()
{
    model->SaveDatabase();
}

void StatisticsPlugin::ClearDatabase()
{
    model->ClearDatabase();
}

const TCHAR * StatisticsPlugin::GetName()
{
    return Language::GetString(IDS_TAB_STATISTICS);
}

const TCHAR * StatisticsPlugin::GetTemplateName()
{
    return TEXT("DLG_STATISTICS");
}

DLGPROC StatisticsPlugin::GetDialogProc()
{
    return view->DlgProc;
}
