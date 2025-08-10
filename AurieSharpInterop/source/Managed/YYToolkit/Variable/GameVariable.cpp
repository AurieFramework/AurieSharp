#include "GameVariable.hpp"
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace YYTKInterop
{
	GameVariable^ GameVariable::CreateFromRValue(const YYTK::RValue& Value)
	{
		GameVariable^ variable = gcnew GameVariable();
		variable->InitializeFromRValue(Value);

		return variable;
	}

	void GameVariable::InitializeFromRValue(const YYTK::RValue& Value)
	{
		this->m_Value = static_cast<YYTK::RValue*>(Aurie::MmAllocateMemory(Aurie::g_ArSelfModule, sizeof(YYTK::RValue)));

		if (!this->m_Value)
			throw gcnew System::OutOfMemoryException("Failed to allocate RValue memory!");
		
		*m_Value = Value;
	}

	YYTK::RValue& GameVariable::ToRValue()
	{
		return *this->m_Value;
	}

	GameVariable::GameVariable()
	{
		InitializeFromRValue(YYTK::RValue());
	}

	GameVariable::GameVariable(System::Int32 Value)
	{
		InitializeFromRValue(YYTK::RValue(Value));
	}

	GameVariable::GameVariable(System::Int64 Value)
	{
		InitializeFromRValue(YYTK::RValue(Value));
	}

	GameVariable::GameVariable(double Value)
	{
		InitializeFromRValue(YYTK::RValue(Value));
	}

	GameVariable::GameVariable(GameObject^ Value)
	{
		InitializeFromRValue(YYTK::RValue(Value->m_Object));
	}

	GameVariable::GameVariable(GameInstance^ Value)
	{
		InitializeFromRValue(YYTK::RValue(Value->m_Object));
	}

	GameVariable::GameVariable(System::String^ Value)
	{
		// Note: This is okay, since YYCreateString (RV_CreateFromString internals) copy it to a GM-owned buffer.
		InitializeFromRValue(YYTK::RValue(marshal_as<std::string>(Value).c_str()));
	}

	GameVariable::operator GameVariable ^ (System::Int32 Value)
	{
		return gcnew GameVariable(Value);
	}

	GameVariable::operator GameVariable ^ (System::Int64 Value)
	{
		return gcnew GameVariable(Value);
	}

	GameVariable::operator GameVariable ^ (GameObject^ Value)
	{
		return gcnew GameVariable(Value);
	}

	GameVariable::operator GameVariable ^ (GameInstance^ Value)
	{
		return gcnew GameVariable(Value);
	}

	GameVariable::operator GameVariable ^ (System::String^ Value)
	{
		return gcnew GameVariable(Value);
	}

	System::Int32 GameVariable::ToInt32()
	{
		System::Int32 value = 0;
		if (TryGetInt32(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to Int32!");
	}

	System::Int64 GameVariable::ToInt64()
	{
		System::Int64 value = 0;
		if (TryGetInt64(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to Int64!");
	}

	double GameVariable::ToDouble()
	{
		double value = 0;
		if (TryGetDouble(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to Double!");
	}

	System::String^ GameVariable::ToString()
	{
		System::String^ value = nullptr;
		if (TryGetString(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to String!");
	}

	GameObject^ GameVariable::ToGameObject()
	{
		GameObject^ value = nullptr;
		if (TryGetGameObject(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to GameObject!");
	}

	GameInstance^ GameVariable::ToGameInstance()
	{
		GameInstance^ value = nullptr;
		if (TryGetGameInstance(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to GameInstance!");
	}

	Gen::IReadOnlyList<GameVariable^>^ GameVariable::ToArrayView()
	{
		Gen::IReadOnlyList<GameVariable^>^ value = nullptr;
		if (TryGetArrayView(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to Array!");
	}


	GameVariable::operator double(GameVariable^ Variable)
	{
		return Variable->ToDouble();
	}

	GameVariable::operator System::Int32(GameVariable^ Variable)
	{
		return Variable->ToInt32();
	}

	GameVariable::operator System::Int64(GameVariable^ Variable)
	{
		return Variable->ToInt64();
	}

	GameVariable::operator System::String^(GameVariable^ Variable)
	{
		return Variable->ToString();
	}

	GameVariable::operator GameObject^(GameVariable^ Variable)
	{
		return Variable->ToGameObject();
	}

	GameVariable::operator GameInstance^(GameVariable^ Variable)
	{
		return Variable->ToGameInstance();
	}

	GameVariable::operator Gen::IReadOnlyList<GameVariable^>^ (GameVariable^ Variable)
	{
		return Variable->ToArrayView();
	}

	bool GameVariable::TryGetInt32(System::Int32% Value)
	{
		if (!m_Value->IsNumberConvertible())
			return false;

		Value = m_Value->ToInt32();
		return true;
	}

	bool GameVariable::TryGetInt64(System::Int64% Value)
	{
		if (!m_Value->IsNumberConvertible())
			return false;

		Value = m_Value->ToInt64();
		return true;
	}

	bool GameVariable::TryGetDouble(double% Value)
	{
		if (!m_Value->IsNumberConvertible())
			return false;

		Value = m_Value->ToDouble();
		return true;
	}

	bool GameVariable::TryGetGameObject(GameObject^% Value)
	{
		if (!m_Value->IsStruct())
			return false;

		Value = gcnew GameObject(m_Value->ToObject());
		return true;
	}

	bool GameVariable::TryGetGameInstance(GameInstance^% Value)
	{
		if (!m_Value->IsStruct())
			return false;

		YYTK::YYObjectBase* object_base = m_Value->ToObject();
		if (object_base->m_ObjectKind != YYTK::OBJECT_KIND_CINSTANCE)
			return false;

		Value = gcnew GameInstance(m_Value->ToInstance());
		return true;
	}

	bool GameVariable::TryGetString(System::String^% Value)
	{
		if (!m_Value->IsString())
			return false;

		Value = gcnew System::String(m_Value->ToCString());
		return true;
	}

	bool GameVariable::TryGetArrayView(Gen::IReadOnlyList<GameVariable^>^% Value)
	{
		if (!m_Value->IsArray())
			return false;

		auto native_array_vector = m_Value->ToVector();
		auto managed_list = gcnew Gen::List<GameVariable^>(static_cast<int>(native_array_vector.size()));

		for (size_t i = 0; i < native_array_vector.size(); i++)
			managed_list->Add(GameVariable::CreateFromRValue(native_array_vector[i]));

		Value = managed_list->AsReadOnly();
		return true;
	}

	GameVariable::~GameVariable()
	{
		this->!GameVariable();
	}

	GameVariable::!GameVariable()
	{
		if (this->m_Value) Aurie::MmFreeMemory(Aurie::g_ArSelfModule, this->m_Value);
		this->m_Value = nullptr;
	}

	GameVariable^ GameVariable::Undefined::get()
	{
		return GameVariable::CreateFromRValue(YYTK::RValue());
	}

	System::String^ GameVariable::Type::get()
	{
		return gcnew System::String(m_Value->GetKindName().c_str());
	}

	GameVariable^ GameVariable::default::get(System::String^ Name)
	{
		if (!m_Value->IsStruct())
			throw gcnew System::InvalidCastException("Cannot access struct members of a non-struct variable!");

		YYTK::RValue* native_value = m_Value->GetRefMember(marshal_as<std::string>(Name));
		if (!native_value)
			throw gcnew System::InvalidCastException("Cannot access non-existing member of a struct variable!");

		return GameVariable::CreateFromRValue(*native_value);
	}

	void GameVariable::default::set(System::String^ Name, GameVariable^ Value)
	{
		if (!m_Value->IsStruct())
			throw gcnew System::InvalidCastException("Cannot access struct members of a non-struct variable!");

		(*m_Value)[marshal_as<std::string>(Name)] = Value->ToRValue();
	}

	GameVariable^ GameVariable::default::get(int Index)
	{
		if (!m_Value->IsArray())
			throw gcnew System::InvalidCastException("Cannot index a non-array variable!");

		auto native_array = m_Value->ToVector();
		if (Index >= native_array.size())
			throw gcnew System::IndexOutOfRangeException("Cannot index past end of array RValue!");

		return GameVariable::CreateFromRValue(native_array[Index]);
	}

	void GameVariable::default::set(int Index, GameVariable^ Value)
	{
		if (!m_Value->IsArray())
			throw gcnew System::InvalidCastException("Cannot index a non-array variable!");

		auto native_array = m_Value->ToRefVector();
		if (Index >= native_array.size())
			throw gcnew System::IndexOutOfRangeException("Cannot index past end of array RValue!");

		*native_array[Index] = Value->ToRValue();
	}
}