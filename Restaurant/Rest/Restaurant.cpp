#include "Restaurant.h"
#include "..\Events\ArrivalEvent.h"
#include "..\Events\CancellationEvent.h"
#include "..\Events\PromotionEvent.h"
#include <cstdlib>
#include <time.h>
#include <iostream>
using namespace std;


Restaurant::Restaurant()
{
	pGUI = NULL;
	C_count = 0;
	NRM_OrdCount = 0;
	VGN_OrdCount = 0;
	VIP_OrdCount = 0;
	NRM_SRVCount = 0;
	VGN_SRVCount = 0;
	VIP_SRVCount = 0;
	NRM_FinishedCount = 0;
	VGN_FinishedCount = 0;
	VIP_FinishedCount = 0;
	NRM_C = 0;
	VIP_C = 0;
	VGN_C = 0;
	AutoPromoted = 0;
	CurrentTimeStep = 1;
}

void Restaurant::RunSimulation()
{
	pGUI = new GUI;
	PROG_MODE mode = pGUI->getGUIMode();
	ifstream inputFile;
	ReadInputFile(inputFile);

	switch (mode)
	{
	case MODE_INTR:
		Interactive_Mode();
		break;
	case MODE_STEP:
		//StepByStep_Mode();
		break;
	case MODE_SLNT:
		break;
	default:
		break;
	};

}

//////////////////////////////////  Event handling functions   /////////////////////////////

//Executes ALL events that should take place at current timestep
void Restaurant::ExecuteEvents(int CurrentTimeStep)
{
	Event* pE;
	while (EventsQueue.peekFront(pE))	//as long as there are more events
	{
		if (pE->getEventTime() > CurrentTimeStep)	//no more events at current timestep
			return;

		pE->Execute(this);          //executes the event
		EventsQueue.dequeue(pE);	//remove event from the queue
		delete pE;					//deallocate event object from memory
	}
}

Restaurant::~Restaurant()
{
	if (pGUI)
		delete pGUI;
}

//This function should add ALL orders and Cooks to the drawing list
//It should get orders from orders lists/queues/stacks/whatever (same for Cooks)
void Restaurant::FillDrawingList()
{
	//Let's add ALL Cooks to GUI::DrawingList
	int C_size = 0;
	VIP_C = 0;
	VGN_C = 0;
	NRM_C = 0;
	Cook** CookArr = AvailableCooks.toArray(C_size);
	for (int i = 0; i < C_size; i++)
	{
		switch (CookArr[i]->GetType())
		{
		case TYPE_NRM:
			NRM_C++;
			break;
		case TYPE_VGAN:
			VGN_C++;
			break;
		case TYPE_VIP:
			VIP_C++;
			break;
		default:
			break;
		}

		pGUI->AddToDrawingList(CookArr[i]);
	}

	//Let's add ALL Ordes to GUI::DrawingList

	///////////Adding Normal Orders to GUI::DrawingList//////////
	Order* pOrd;
	NRM_OrdCount = 0;
	int size = 0;
	Order** NRM_Orders_Array = WaitingNormal.toArray(size);
	NRM_OrdCount += size;
	for (int i = 0; i < size; i++)
	{
		pOrd = NRM_Orders_Array[i];
		pGUI->AddToDrawingList(pOrd);
	}

	///////////Adding Vegan Orders to GUI::DrawingList//////////
	VGN_OrdCount = 0;
	Order** VGN_Orders_Array = WaitingVegan.toArray(size);
	VGN_OrdCount += size;
	for (int i = 0; i < size; i++)
	{
		pOrd = VGN_Orders_Array[i];
		pGUI->AddToDrawingList(pOrd);
	}

	///////////Adding VIP Orders to GUI::DrawingList//////////
	VIP_OrdCount = 0;
	Order** VIP_Orders_Array = WaitingVIP.toArray(size);
	VIP_OrdCount += size;
	for (int i = 0; i < size; i++)
	{
		pOrd = VIP_Orders_Array[i];
		pGUI->AddToDrawingList(pOrd);
	}

	///////////Adding In service Orders to GUI::DrawingList//////////
	Order* pServiceOrd;
	int SRVCount = 0;
	int Servicesize = 0;
	Order** Service_Orders_Array = InService.toArray(Servicesize);
	SRVCount += Servicesize;
	for (int i = 0; i < Servicesize; i++)
	{
		pServiceOrd = Service_Orders_Array[i];
		pGUI->AddToDrawingList(pServiceOrd);
	}

	///////////Adding the finished Orders to GUI::DrawingList//////////
	Order* pFinishedOrd;
	NRM_FinishedCount = 0;
	VGN_FinishedCount = 0;
	VIP_FinishedCount = 0;
	int finishedsize = 0;
	Order** Finished_Orders_Array = Finished.toArray(finishedsize);
	for (int i = 0; i < finishedsize; i++)
	{
		pFinishedOrd = Finished_Orders_Array[i];
		switch (pFinishedOrd->GetType())
		{
		case TYPE_NRM:
			NRM_FinishedCount++;
			break;
		case TYPE_VGAN:
			VGN_FinishedCount++;
			break;
		case TYPE_VIP:
			VIP_FinishedCount++;
			break;
		default:
			break;
		}
		pGUI->AddToDrawingList(pFinishedOrd);
	}
}

void Restaurant::ReadInputFile(ifstream& InputFile)
{
	do
	{
		pGUI->PrintMessage("Please enter the input text file name without .txt: ");
		string FileName = pGUI->GetString();
		FileName += ".txt";
		InputFile.open(FileName, ios::in);
		if (InputFile.is_open())
		{
			int N = 0, G = 0, V = 0;
			int SN = 0, SG = 0, SV = 0;
			int BO = 0, BN = 0, BG = 0, BV = 0;
			int AutoP = 0, M = 0;
			InputFile >> N >> G >> V;
			InputFile >> SN >> SG >> SV;
			InputFile >> BO >> BN >> BG >> BV;
			InputFile >> AutoP;
			InputFile >> M;

			int EventCnt = M;

			Order* pOrd;
			Event* pEv;

			srand(time(NULL));

			C_count = N + G + V;
			Cook* pCook;

			//Adding the normal cooks the cooks array
			for (int i = 0; i < N; i++)
			{
				pCook = new Cook;
				pCook->SetID(i + 1);
				pCook->SetSpeed(SN);
				pCook->SetType(TYPE_NRM);
				pCook->SetBreakDuration(BN);
				pCook->SetOrdToBreak(BO);
				AvailableCooks.enqueue(pCook, pCook->GetCurrOrd());
			}

			// Adding the vegan cooks the cooks array
			for (int i = N; i < C_count - V; i++)
			{
				pCook = new Cook;
				pCook->SetID(i + 1);
				pCook->SetSpeed(SN);
				pCook->SetType(TYPE_VGAN);
				pCook->SetBreakDuration(BN);
				AvailableCooks.enqueue(pCook, pCook->GetCurrOrd());
			}

			//Adding the VIP cooks the cooks array
			for (int i = N + G; i < C_count; i++)
			{
				pCook = new Cook;
				pCook->SetID(i + 1);
				pCook->SetSpeed(SN);
				pCook->SetType(TYPE_VIP);
				pCook->SetBreakDuration(BN);
				AvailableCooks.enqueue(pCook, pCook->GetCurrOrd());
			}

			int EvTime = 0;

			int O_id = 0;
			CancellationEvent* pCE;
			for (int i = 0; i < EventCnt; i++)
			{
				char EventType = ' ';
				InputFile >> EventType;

				switch (EventType)
				{
				case 'R':
					char OType;
					InputFile >> OType;
					int OrderType;                                          //
					(OType == 'N') ? OrderType = 0 :                        //These 3 lines convert the order type from char (N, G, V) to ORD_TYPE
						(OType == 'G') ? OrderType = 1 : OrderType = 2;     //
					InputFile >> EvTime;
					InputFile >> O_id;
					int OrderSize;
					double OrderCost;
					InputFile >> OrderSize >> OrderCost;
					pEv = new ArrivalEvent(EvTime, O_id, (ORD_TYPE)OrderType, OrderSize, OrderCost);
					EventsQueue.enqueue(pEv);
					break;

				case 'X':
					InputFile >> EvTime;
					InputFile >> O_id;
					pCE = new CancellationEvent(EvTime, O_id);
					EventsQueue.enqueue(pCE);
					break;

				case 'P':
					InputFile >> EvTime;
					InputFile >> O_id;
					int ExtMoney;
					InputFile >> ExtMoney;
					pEv = new PromotionEvent(EvTime, O_id, ExtMoney);
					EventsQueue.enqueue(pEv);
					break;

				default:
					break;
				}
			}
		}
		else
		{
			pGUI->PrintMessage("Couldn't find the file. Click to enter a valid name (Note: don't type .txt)");
			pGUI->waitForClick();
		}
	} while (!InputFile.is_open());

}

void Restaurant::WritingOutputFile()
{
	ofstream outputFile;
	pGUI->PrintMessage("Please enter the output text file name without .txt: ");
	string FileName = pGUI->GetString();
	FileName += ".txt";
	outputFile.open(FileName, ios::out);

	outputFile << "FT  ID  AT  WT  ST\n";

	int finishedOrders_Size = 0;
	float TotalWaitingTime = 0, TotalServingTime = 0;;
	Order** finishedOrders = Finished.toArray(finishedOrders_Size);
	for (int i = 0; i < finishedOrders_Size; i++)
	{
		TotalWaitingTime += finishedOrders[i]->GetInServiceTime() - finishedOrders[i]->GetArrivalTime();
		TotalServingTime += finishedOrders[i]->GetInServiceTime();
		outputFile << finishedOrders[i]->GetFinishTime() << "  " << finishedOrders[i]->GetID() << "   " << finishedOrders[i]->GetArrivalTime()
			<< "  " << finishedOrders[i]->GetInServiceTime() - finishedOrders[i]->GetArrivalTime() << "  "
			<< CurrentTimeStep - finishedOrders[i]->GetInServiceTime() << endl;
	}

	outputFile << "Orders: " << NRM_FinishedCount + VGN_FinishedCount + VIP_FinishedCount << " [Norm: "
		<< NRM_FinishedCount << ", Veg: " << VGN_FinishedCount << ", VIP: " << VIP_FinishedCount << "]\n";
	outputFile << "Cooks: " << NRM_C + VGN_C + VIP_C << " [Norm: " << NRM_C << ", Veg: " << VGN_C << ", VIP: " << VIP_C << "]\n";
	outputFile << "Avg Wait = " << TotalWaitingTime / finishedOrders_Size << ",  Avg Serv = " << TotalServingTime / finishedOrders_Size << endl;
	outputFile << "Auto-promoted: " << (float(AutoPromoted) / float(NRM_FinishedCount)) * 100.0 << "%\n";

	pGUI->PrintMessage("Output text file is generated, click to END the program.");
	pGUI->waitForClick();
}

void Restaurant::Interactive_Mode()
{
	bool notFinished = true;

	while (notFinished)	//as long as events queue is not empty yet
	{
		ExecuteEvents(CurrentTimeStep);	//execute all events at current time step
		UpdateInServiceOrders();
		UpdateCooks();
		Assigning();
		FillDrawingList();
		pGUI->UpdateInterface();
		PrintData();
		pGUI->ResetDrawingList();
		pGUI->waitForClick();
		CurrentTimeStep++;	//advance timestep
		notFinished = !EventsQueue.isEmpty() || InService.GetCount() != 0 || !WaitingNormal.isEmpty() || !WaitingVegan.isEmpty() || !WaitingVIP.isEmpty();
	}
	pGUI->PrintEndProgram("generation done, click to save the information in the output text file.");
	pGUI->waitForClick();
	WritingOutputFile();
}

//void Restaurant::StepByStep_Mode()
//{
//	bool notFinished = !EventsQueue.isEmpty() || InServiceVGN.GetCount() != 0 || InServiceVIP.GetCount() != 0 || InServiceNRM.GetCount() != 0 || !WaitingNormal.isEmpty() || !WaitingVegan.isEmpty() || !WaitingVIP.isEmpty();
//
//	Order* pNRMOrd = nullptr;
//	Order* pVGNOrd = nullptr;
//	Order* pVIPOrd = nullptr;
//	Order* pFinishedOrd = nullptr;
//
//	while (notFinished)	//as long as events queue is not empty yet
//	{
//		//a) Executing Events at this current step 
//		ExecuteEvents(CurrentTimeStep);	//execute all events at current time step
//
//		FillDrawingList();
//
//		/////////////////////////////////////////////////////////////////////////////////////////
//
//		string waitingNRM = to_string(NRM_OrdCount);
//		string waitingVGN = to_string(VGN_OrdCount);
//		string waitingVIP = to_string(VIP_OrdCount);
//
//		string finishedNRM = to_string(NRM_FinishedCount);
//		string finishedVGN = to_string(VGN_FinishedCount);
//		string finishedVIP = to_string(VIP_FinishedCount);
//
//		string C_NRM = to_string(NRM_C);
//		string C_VGN = to_string(VGN_C);
//		string C_VIP = to_string(VIP_C);
//
//		string timestep = to_string(CurrentTimeStep);
//
//		pGUI->PrintMessage("TS: " + timestep);	//print current timestep
//		pGUI->PrintWaitingOrders("Waiting: (Normal = " + waitingNRM + "), (Vegan = " + waitingVGN + "), (VIP = " + waitingVIP + ")");	//print waiting orders numbers
//		pGUI->PrintAvailableCooks("Cooks: (Normal = " + C_NRM + "), (Vegan = " + C_VGN + "), (VIP = " + C_VIP + ")");		//Prints the number if cooks
//		pGUI->PrintFinishedOrders("Finished: (Normal = " + finishedNRM + "), (Vegan = " + finishedVGN + "), (VIP = " + finishedVIP + ")");	//print finished orders numbers
//
//		int csize = 0, i = 0;
//		Cook** C_Arr = AvailableCooks.toArray(csize);
//
//		for (int j = 0, count = 0; j < csize; j++)
//		{
//			if (C_Arr[j]->GetAssignedOrder())
//			{
//				char ctype;
//				if (C_Arr[j]->GetType() == 0) ctype = 'N';
//				else if (C_Arr[j]->GetType() == 1) ctype = 'G';
//				else ctype = 'V';
//
//				char otype;
//				if (C_Arr[j]->GetAssignedOrder()->GetType() == 0) otype = 'N';
//				else if (C_Arr[j]->GetAssignedOrder()->GetType() == 1) otype = 'G';
//				else otype = 'V';
//
//				string C_ID = to_string(C_Arr[j]->GetID());
//				string O_ID = to_string(C_Arr[j]->GetAssignedOrder()->GetID());
//
//				pGUI->PrintAssignedOrders(ctype + C_ID + "(" + otype + O_ID + ")", count);
//				count++;
//			}
//		}
//
//		pNRMOrd = nullptr;
//		pVGNOrd = nullptr;
//		pVIPOrd = nullptr;
//
//		//b) Picking 1 order from each type from Waiting to be InService		
//		if (WaitingNormal.dequeue(pNRMOrd))
//		{
//			pNRMOrd->SetStatus(SRV);
//			pNRMOrd->SetInServiceTime(CurrentTimeStep + 1);
//			pNRMOrd->SetAssignedCook(C_Arr[i]);
//			InServiceNRM.InsertBeg(pNRMOrd);
//			C_Arr[i]->SetAssignedOrder(pNRMOrd);
//			C_Arr[i]->SetCurrOrd(C_Arr[i]->GetCurrOrd() + 1);
//			AvailableCooks.InsertEnd(AvailableCooks.RemoveFirst()->getItem());
//			i++;
//		}
//
//		if (WaitingVegan.dequeue(pVGNOrd))
//		{
//			pVGNOrd->SetStatus(SRV);
//			pVGNOrd->SetInServiceTime(CurrentTimeStep + 1);
//			pVGNOrd->SetAssignedCook(C_Arr[i]);
//			InServiceVGN.InsertBeg(pVGNOrd);
//			C_Arr[i]->SetAssignedOrder(pVGNOrd);
//			C_Arr[i]->SetCurrOrd(C_Arr[i]->GetCurrOrd() + 1);
//			AvailableCooks.InsertEnd(AvailableCooks.RemoveFirst()->getItem());
//			i++;
//		}
//
//		if (WaitingVIP.dequeue(pVIPOrd))
//		{
//			pVIPOrd->SetStatus(SRV);
//			pVIPOrd->SetInServiceTime(CurrentTimeStep + 1);
//			pVIPOrd->SetAssignedCook(C_Arr[i]);
//			InServiceVIP.InsertBeg(pVIPOrd);
//			C_Arr[i]->SetAssignedOrder(pVIPOrd);
//			C_Arr[i]->SetCurrOrd(C_Arr[i]->GetCurrOrd() + 1);
//			AvailableCooks.InsertEnd(AvailableCooks.RemoveFirst()->getItem());
//			i++;
//		}
//
//		//c)each 5 timesteps moving order of each type from InService to Finished list
//		if ((CurrentTimeStep + 1) % 5 == 0)
//		{
//			pFinishedOrd = InServiceNRM.Remove();
//			if (pFinishedOrd)
//			{
//				pFinishedOrd->SetStatus(DONE);
//				FinishedNRM.enqueue(pFinishedOrd);
//				pFinishedOrd->GetAssignedCook()->SetAssignedOrder(nullptr);
//				pFinishedOrd->SetAssignedCook(nullptr);
//			}
//
//			pFinishedOrd = InServiceVGN.Remove();
//			if (pFinishedOrd)
//			{
//				pFinishedOrd->SetStatus(DONE);
//				FinishedVGN.enqueue(pFinishedOrd);
//				pFinishedOrd->GetAssignedCook()->SetAssignedOrder(nullptr);
//				pFinishedOrd->SetAssignedCook(nullptr);
//			}
//
//			pFinishedOrd = InServiceVIP.Remove();
//			if (pFinishedOrd)
//			{
//				pFinishedOrd->SetStatus(DONE);
//				FinishedVIP.enqueue(pFinishedOrd);
//				pFinishedOrd->GetAssignedCook()->SetAssignedOrder(nullptr);
//				pFinishedOrd->SetAssignedCook(nullptr);
//			}
//
//		}
//
//		notFinished = !EventsQueue.isEmpty() || InServiceVGN.GetCount() != 0 || InServiceVIP.GetCount() != 0 || InServiceNRM.GetCount() != 0 || !WaitingNormal.isEmpty() || !WaitingVegan.isEmpty() || !WaitingVIP.isEmpty();
//
//		pGUI->UpdateInterface();
//		pGUI->ResetDrawingList();
//		Sleep(1000);
//		CurrentTimeStep++;	//advance timestep
//
//	}
//
//	FillDrawingList();
//	pGUI->UpdateInterface();
//	pGUI->PrintEndProgram("generation done, click to END program");
//	pGUI->waitForClick();
//}

void  Restaurant::AddtoVIPQueue(Order* po)	//adds an order to the vip orders queue
{	// To calculate the Priority of the order
	int p = po->GetOrderMoney() + po->GetOrderSize() + 2 * po->GetArrivalTime();
	WaitingVIP.enqueue(po, p);
}

void  Restaurant::AddtoNormalQueue(Order* po) //adds an order to the normal orders queue
{
	WaitingNormal.enqueue(po);
}

void  Restaurant::AddtoVeganQueue(Order* po) //adds an order to the vegan orders queue
{
	WaitingVegan.enqueue(po);
}

bool  Restaurant::DeleteNormalQueue(int id) // Removes order from normal queue if cancelled
{
	Order* CancellatedOrder;
	bool IsFound = WaitingNormal.SearchForOrder(id, CancellatedOrder);
	return IsFound;
}

Order*& Restaurant::PromotOrder(int id, double Extra)
{
	Order* PromotedOrder = NULL;
	bool IsFound = WaitingNormal.SearchForOrder(id, PromotedOrder);
	if (IsFound)
	{
		PromotedOrder->SetOrderMoney(PromotedOrder->GetOrderMoney() + Extra);
		PromotedOrder->SetType(TYPE_VIP);
	}
	return PromotedOrder;
}

void Restaurant::UpdateCooks()
{
	int C_size = 0;
	Cook** CookArr = AvailableCooks.toArray(C_size);
	////////////////////////////////UBDATE SERVING COOKS////////////////////////////////
	for (int i = 0; i < C_size; i++)
		if (CookArr[i] && CookArr[i]->GetAssignedOrder() && !CookArr[i]->GetInBreak())
		{
			if (CookArr[i]->GetOrdToBreak() == CookArr[i]->GetCurrOrd() && CookArr[i]->GetAssignedOrder()->GetStatus() == DONE)
			{   //Check for cooks that should be in break
				CookArr[i]->SetInBreak(true);
				CookArr[i]->SetCurrOrd(0);
				CookArr[i]->SetBreakTS(CurrentTimeStep);
				CookArr[i]->SetAssignedOrder(NULL);
				if (CookArr[i]->GetType() == TYPE_NRM) NRM_C--;
				else if (CookArr[i]->GetType() == TYPE_VGAN) VGN_C--;
				else VIP_C--;
			}
			else if ((CookArr[i]->GetOrdToBreak() > CookArr[i]->GetCurrOrd()) && CookArr[i]->GetAssignedOrder() && (CookArr[i]->GetAssignedOrder()->GetStatus() == DONE))
			{	//Check for cooks should be not assigned
				CookArr[i]->SetAssignedOrder(NULL);
			}
		}

	Cook* pCook;
	while (AvailableCooks.dequeue(pCook));
	for (int i = 0; i < C_size; i++)//Removing in break cooks from available list & adding them to in break list
	{                               //resorting available cooks list
		if (!CookArr[i]->GetInBreak())
			AvailableCooks.enqueue(CookArr[i], CookArr[i]->GetCurrOrd());
		else InBreakCooks.InsertBeg(CookArr[i]);
	}
	////////////////////////////////UBDATE INBREAK COOKS////////////////////////////////
	Cook** CookinBreak = InBreakCooks.toArray(C_size);
	for (int i = 0; i < C_size; i++)//Check for cooks should go back to available list
		if (CurrentTimeStep - CookinBreak[i]->GetBreakTS() == CookinBreak[i]->GetBreakDuration())
		{
			InBreakCooks.DeleteNode(CookinBreak[i]);
			CookinBreak[i]->SetInBreak(false);
			AvailableCooks.enqueue(CookinBreak[i], CookinBreak[i]->GetCurrOrd());
			if (CookArr[i]->GetType() == TYPE_NRM) NRM_C++;
			else if (CookArr[i]->GetType() == TYPE_VGAN) VGN_C++;
			else VIP_C++;
		}
}

void Restaurant::UpdateInServiceOrders()
{
	int O_size = 0;
	Order** pOrd = InService.toArray(O_size);
	for (int i = 0; i < O_size; i++)
	{
		if (CurrentTimeStep == pOrd[i]->GetFinishTime())
		{
			pOrd[i]->SetStatus(DONE);
			pOrd[i]->SetAssignedCook(NULL);
			InService.DeleteNode(pOrd[i]);
			Finished.enqueue(pOrd[i]);
			if (pOrd[i]->GetType() == TYPE_NRM) NRM_FinishedCount++;
			else if (pOrd[i]->GetType() == TYPE_VGAN) VGN_FinishedCount++;
			else VIP_FinishedCount++;
		}
	}
}

void Restaurant::Assigning()
{
	Order* pOrd;
	int C_size = 0;
	Cook** CookArr = AvailableCooks.toArray(C_size);
	int VIP_size=0;
	Order** VIP_Order = WaitingVIP.toArray(VIP_size);
	///////////////////////////////////Assigning VIP orders///////////////////////////////////
	while (WaitingVIP.peekFront(pOrd) && pOrd->GetArrivalTime() <= CurrentTimeStep)//while there is VIP orders in waiting list till this current time step
	{
		for (int i = 0; i < VIP_size; i++)
		{
			if ((CurrentTimeStep - (VIP_Order[i]->GetArrivalTime()) > VIP_WT))
			{
				pOrd = VIP_Order[i];
				WaitingVIP.DeleteNode(VIP_Order[i]->GetID());
				UrgentOrders.enqueue(pOrd);
			}
		}
		if((UrgentOrders.peekFront(pOrd)))
		for (int i = 0; i < C_size; i++)//Searching for available VIP cook.
		{
			if ((UrgentOrders.peekFront(pOrd))&&(CookArr[i]->GetType() == TYPE_VIP) && ((CookArr[i]->GetInBreak()) || !CookArr[i]->GetAssignedOrder()))
			{
				UrgentOrders.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[i]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}

		if (!C_size) return;

		for (int i = 0; i < C_size; i++)//Searching for available VIP cook.
		{
			if ((CookArr[i]) && (CookArr[i]->GetType() == TYPE_VIP) && (!CookArr[i]->GetAssignedOrder()))
			{
				WaitingVIP.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[i]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}

		if (pOrd->GetAssignedCook()) continue;

		for (int i = 0; i < C_size; i++)//Searching for available Normal cook.
		{
			if (CookArr[i] && CookArr[i]->GetType() == TYPE_NRM && !CookArr[i]->GetAssignedOrder())
			{
				WaitingVIP.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[i]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}

		if (pOrd->GetAssignedCook()) continue;

		for (int i = 0; i < C_size; i++)//Searching for available Vegan cook.
		{
			if (CookArr[i] && CookArr[0]->GetType() == TYPE_VGAN && !CookArr[i]->GetAssignedOrder())
			{
				WaitingVIP.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[0]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}
	}

	///////////////////////////////////Assigning Vegan Orders///////////////////////////////////
	while (WaitingVegan.peekFront(pOrd) && pOrd->GetArrivalTime() <= CurrentTimeStep)//while there is vegan orders in waiting list till this current time step
	{
		if (!C_size) return;
		for (int i = 0; i < C_size; i++)//Searching for available Vegan cook.
		{
			if (CookArr[i] && CookArr[i]->GetType() == TYPE_VGAN && !CookArr[i]->GetAssignedOrder())
			{
				WaitingVegan.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[0]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}
	}

	///////////////////////////////////Assigning Normal Orders///////////////////////////////////
	while (WaitingNormal.peekFront(pOrd) && pOrd->GetArrivalTime() <= CurrentTimeStep)//while there is more normal orders in waiting list till this current time step
	{
		if (!C_size)return;
		for (int i = 0; i < C_size; i++)//Searching for available Normal cook.
		{
			if (CookArr[i] && CookArr[0]->GetType() == TYPE_NRM && !CookArr[i]->GetAssignedOrder())
			{
				WaitingNormal.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[0]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}

		if (pOrd->GetAssignedCook()) continue;

		for (int i = 0; i < C_size; i++)//Searching for available VIP cook.
		{
			if (CookArr[i] && CookArr[i]->GetType() == TYPE_VIP && !CookArr[i]->GetAssignedOrder())
			{
				WaitingNormal.dequeue(pOrd);
				CookArr[i]->SetAssignedOrder(pOrd);
				CookArr[i]->SetCurrOrd(CookArr[i]->GetCurrOrd() + 1);
				pOrd->SetAssignedCook(CookArr[i]);
				pOrd->SetInServiceTime(CurrentTimeStep);
				pOrd->SetFinishTime(CurrentTimeStep + ceil(pOrd->GetOrderSize() / CookArr[i]->GetSpeed()));
				pOrd->SetStatus(SRV);
				InService.InsertEnd(pOrd);
				break;
			}
		}
	}

}

void Restaurant::PrintData()
{
	string waitingNRM = to_string(NRM_OrdCount);
	string waitingVGN = to_string(VGN_OrdCount);
	string waitingVIP = to_string(VIP_OrdCount);

	string finishedNRM = to_string(NRM_FinishedCount);
	string finishedVGN = to_string(VGN_FinishedCount);
	string finishedVIP = to_string(VIP_FinishedCount);

	string C_NRM = to_string(NRM_C);
	string C_VGN = to_string(VGN_C);
	string C_VIP = to_string(VIP_C);

	string timestep = to_string(CurrentTimeStep);

	pGUI->PrintMessage("TS: " + timestep);	//print current timestep
	pGUI->PrintWaitingOrders("Waiting: (Normal = " + waitingNRM + "), (Vegan = " + waitingVGN + "), (VIP = " + waitingVIP + ")");	//print waiting orders numbers
	pGUI->PrintAvailableCooks("Cooks: (Normal = " + C_NRM + "), (Vegan = " + C_VGN + "), (VIP = " + C_VIP + ")");		//Prints the number if cooks
	pGUI->PrintFinishedOrders("Finished: (Normal = " + finishedNRM + "), (Vegan = " + finishedVGN + "), (VIP = " + finishedVIP + ")");	//print finished orders numbers

	int csize = 0, i = 0;
	Cook** C_Arr = AvailableCooks.toArray(csize);

	for (int j = 0, count = 0; j < csize; j++)
	{
		if (C_Arr[j]->GetAssignedOrder())
		{
			char ctype;
			if (C_Arr[j]->GetType() == 0) ctype = 'N';
			else if (C_Arr[j]->GetType() == 1) ctype = 'G';
			else ctype = 'V';

			char otype;
			if (C_Arr[j]->GetAssignedOrder()->GetType() == 0) otype = 'N';
			else if (C_Arr[j]->GetAssignedOrder()->GetType() == 1) otype = 'G';
			else otype = 'V';

			string C_ID = to_string(C_Arr[j]->GetID());
			string O_ID = to_string(C_Arr[j]->GetAssignedOrder()->GetID());

			pGUI->PrintAssignedOrders(ctype + C_ID + "(" + otype + O_ID + ")", count);
			count++;
		}
	}
}