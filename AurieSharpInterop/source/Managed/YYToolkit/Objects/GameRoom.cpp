#include "GameRoom.hpp"
#include "../Interface/Interface.hpp"

int YYTKInterop::GameRoom::Width::get()
{
	return this->m_RoomObject->m_Width;
}

void YYTKInterop::GameRoom::Width::set(int NewWidth)
{
	this->m_RoomObject->m_Width = NewWidth;
}

int YYTKInterop::GameRoom::Height::get()
{
	return this->m_RoomObject->m_Height;
}

void YYTKInterop::GameRoom::Height::set(int NewHeight)
{
	this->m_RoomObject->m_Height = NewHeight;
}

System::Collections::Generic::List<YYTKInterop::GameInstance^>^ YYTKInterop::GameRoom::ActiveInstances::get()
{
	auto instance_list = gcnew Gen::List<GameInstance^>(4);

	// Loop all active instances in the room
	for (
		YYTK::CInstance* inst = m_RoomObject->GetMembers().m_ActiveInstances.m_First;
		inst != nullptr;
		inst = inst->GetMembers().m_Flink
		)
	{
		instance_list->Add(gcnew GameInstance(inst));
	}

	return instance_list;
}

System::Collections::Generic::List<YYTKInterop::GameInstance^>^ YYTKInterop::GameRoom::InactiveInstances::get()
{
	auto instance_list = gcnew Gen::List<GameInstance^>(4);

	// Loop all active instances in the room
	for (
		YYTK::CInstance* inst = m_RoomObject->GetMembers().m_InactiveInstances.m_First;
		inst != nullptr;
		inst = inst->GetMembers().m_Flink
		)
	{
		instance_list->Add(gcnew GameInstance(inst));
	}

	return instance_list;
}
