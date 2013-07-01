#ifndef MONTH_MODEL_H
#define MONTH_MODEL_H

#include "NetModel.h"
#include "../Utils/SQLite.h"

class MonthModel : public NetModel
{
public:
	// Item Definition
	// 1. The Item of a Process for a Month, 496 Bytes -------
	typedef struct tagMonthItem
	{
		__int64 dayTx[31]; // In Bytes
		__int64 dayRx[31];

		__int64 sumTx;
		__int64 sumRx;

		struct tagMonthItem()
		{
			RtlZeroMemory(dayTx, sizeof(dayTx));
			RtlZeroMemory(dayRx, sizeof(dayRx));

			sumTx = 0;
			sumRx = 0;
		}
	} MonthItem;

private:
	// 2. The Item of a Process ------------------------------
	typedef struct tagMtModelItem
	{
		static int firstMonth; // Jan 1970 = 0, Feb 1970 = 1 ...
		std::vector<MonthItem> months;
	} MtModelItem;

	// Items
	std::map<int, MtModelItem> _items;

	// Others
	static MonthModel *_this;

private:
	void Fill();
	void InitDatabase();
	static void InitDatabaseCallback(SQLiteRow *row);

public:
	MonthModel();

	// Modify the Model
	void InsertPacket(PacketInfoEx *pi);

	// Export Model Info
	void Export(int process, int curMonth, MonthItem &item);

	int GetFirstMonth();
	int GetLastMonth();

	// Save Model Info to Database
	void SaveDatabase();
};

#endif
