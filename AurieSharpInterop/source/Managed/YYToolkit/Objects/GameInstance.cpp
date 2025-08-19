#include "GameInstance.hpp"
#include "../Variable/GameVariable.hpp"
#include <msclr/marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;

namespace YYTKInterop
{
	void GameInstance::ThrowIfInvalidNativeInstance()
	{
		if (!m_Object || m_Object->m_ObjectKind != YYTK::OBJECT_KIND_CINSTANCE)
			throw gcnew InvalidCastException("Tried to instantiate instance with non-instance object!");
	}

	YYTK::CInstance* GameInstance::GetNativeInstance()
	{
		return static_cast<YYTK::CInstance*>(m_Object);
	}

	GameInstance::GameInstance(YYTK::CInstance* Instance)
	{
		m_Object = Instance;
		ThrowIfInvalidNativeInstance();
	}

	GameInstance^ GameInstance::FromInstanceID(int InstanceID)
	{
		return gcnew GameInstance(YYTK::CInstance::FromInstanceID(InstanceID));
	}

	GameInstance^ GameInstance::FromObject(GameObject^ Object)
	{
		return gcnew GameInstance(static_cast<YYTK::CInstance*>(Object->m_Object));
	}

	String^ GameInstance::Name::get()
	{
		YYTK::RValue value = this->m_Object;

		std::string rvalue_kind_name = YYTK::GetPrivateInterface()->RV_GetKindName(&value);
		std::string rvalue_specific_kind_name = YYTK::GetPrivateInterface()->RV_GetObjectSpecificKind(&value);

		if (!_stricmp(rvalue_kind_name.c_str(), rvalue_specific_kind_name.c_str()))
			return gcnew System::String(value.GetKindName().c_str());

		return gcnew System::String(std::format("{} {}", rvalue_kind_name, rvalue_specific_kind_name).c_str());
	}

	double GameInstance::X::get()
	{
		YYTK::RValue x;

		if (!Aurie::AurieSuccess(YYTK::GetInterface()->GetBuiltin("x", GetNativeInstance(), NULL_INDEX, x)))
			throw gcnew InvalidOperationException("Failed to get X coordinate!");

		return x.ToDouble();
	}

	void GameInstance::X::set(double NewX)
	{
		YYTK::RValue new_x = NewX;

		if (!Aurie::AurieSuccess(YYTK::GetInterface()->SetBuiltin("x", GetNativeInstance(), NULL_INDEX, new_x)))
			throw gcnew InvalidOperationException("Failed to set X coordinate!");
	}

	double GameInstance::Y::get()
	{
		YYTK::RValue y;

		if (!Aurie::AurieSuccess(YYTK::GetInterface()->GetBuiltin("y", GetNativeInstance(), NULL_INDEX, y)))
			throw gcnew InvalidOperationException("Failed to get Y coordinate!");

		return y.ToDouble();
	}

	void GameInstance::Y::set(double NewY)
	{
		YYTK::RValue new_y = NewY;

		if (!Aurie::AurieSuccess(YYTK::GetInterface()->SetBuiltin("y", GetNativeInstance(), NULL_INDEX, new_y)))
			throw gcnew InvalidOperationException("Failed to set Y coordinate!");
	}

	int GameInstance::ID::get()
	{
		YYTK::RValue id;

		if (!Aurie::AurieSuccess(YYTK::GetInterface()->GetBuiltin("id", GetNativeInstance(), NULL_INDEX, id)))
			throw gcnew InvalidOperationException("Failed to get ID!");

		return id.ToInt32();
	}

	Gen::IReadOnlyDictionary<System::String^, GameVariable^>^ GameInstance::Members::get()
	{
		YYTK::RValue my_struct = this->m_Object;
		std::map<std::string, YYTK::RValue*> my_map = my_struct.ToRefMap();

		auto managed_map = gcnew Gen::Dictionary<System::String^, GameVariable^>(static_cast<int>(my_map.size()));

		for (const auto& [Key, Value] : my_map)
		{
			System::String^ key = gcnew System::String(Key.c_str());
			GameVariable^ value = GameVariable::CreateFromRValue(*Value);
			managed_map->Add(key, value);
		}

		return managed_map;
	}

	GameVariable^ GameInstance::default::get(System::String^ Name)
	{
		YYTK::RValue self = YYTK::RValue(this->m_Object);

		YYTK::RValue* native_value = self.GetRefMember(marshal_as<std::string>(Name));
		if (!native_value)
			throw gcnew System::InvalidCastException("Cannot access non-existing member of a struct variable!");

		return GameVariable::CreateFromRValue(*native_value);
	}

	void GameInstance::default::set(System::String^ Name, GameVariable^ Value)
	{
		YYTK::RValue self = YYTK::RValue(this->m_Object);

		self[marshal_as<std::string>(Name)] = Value->ToRValue();
	}
}