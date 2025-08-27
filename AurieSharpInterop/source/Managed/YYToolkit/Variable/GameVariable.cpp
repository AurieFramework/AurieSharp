#include "GameVariable.hpp"
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace YYTKInterop
{
	GameVariable::GameVariable(std::nullptr_t)
	{
		this->m_Value = nullptr;
	}

	GameVariable^ GameVariable::CreateFromRValue(const YYTK::RValue& Value)
	{
		// Create an uninitialized object.
		GameVariable^ variable = gcnew GameVariable(nullptr);

		variable->InitializeFromRValue(Value);
		return variable;
	}

	void GameVariable::InitializeFromRValue(const YYTK::RValue& Value)
	{
		System::Diagnostics::Debug::Assert(this->m_Value == nullptr, gcnew System::String("Overwriting an already initialized RValue"));
		this->m_Value = new YYTK::RValue(Value);
	}

	YYTK::RValue& GameVariable::ToRValue()
	{
		return *this->m_Value;
	}

	GameVariable::GameVariable()
	{
		InitializeFromRValue(YYTK::RValue());
	}

	GameVariable::GameVariable(bool Value)
	{
		InitializeFromRValue(YYTK::RValue(Value));
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

	GameVariable::GameVariable(float Value)
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
		InitializeFromRValue(YYTK::RValue(marshal_as<std::string>(Value)));
	}

	GameVariable::operator GameVariable ^ (double Value)
	{
		return gcnew GameVariable(Value);
	}

	GameVariable::operator GameVariable ^ (bool Value)
	{
		return gcnew GameVariable(Value);
	}

	GameVariable::operator GameVariable ^ (float Value)
	{
		return gcnew GameVariable(Value);
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

	bool GameVariable::ToBoolean()
	{
		bool value = 0;
		if (TryGetBoolean(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to Boolean!");
	}

	float GameVariable::ToFloat()
	{
		float value = 0;
		if (TryGetFloat(value))
			return value;

		throw gcnew System::InvalidCastException("RValue is not castable to Float!");
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

	GameVariable::operator bool(GameVariable^ Variable)
	{
		return Variable->ToBoolean();
	}

	GameVariable::operator float(GameVariable^ Variable)
	{
		return Variable->ToFloat();
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

	bool GameVariable::TryGetBoolean(bool% Value)
	{
		if (!m_Value->IsNumberConvertible())
			return false;

		Value = m_Value->ToBoolean();
		return true;
	}

	bool GameVariable::TryGetFloat(float% Value)
	{
		if (!m_Value->IsNumberConvertible())
			return false;

		Value = static_cast<float>(m_Value->ToDouble());
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
		// YYTK can safely convert all these types to strings!
		if (!m_Value->IsString() &&
			!m_Value->IsNumberConvertible() &&
			!m_Value->IsStruct() &&
			!m_Value->IsArray() &&
			!m_Value->IsUndefined() &&
			m_Value->m_Kind != YYTK::VALUE_REF
		)
		{
			return false;
		}

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
		System::GC::SuppressFinalize(this);
	}

	GameVariable::!GameVariable()
	{
		if (this->m_Value)
			delete this->m_Value;

		this->m_Value = nullptr;
	}

	GameVariable^ GameVariable::Undefined::get()
	{
		return GameVariable::CreateFromRValue(YYTK::RValue());
	}

	System::String^ GameVariable::Type::get()
	{
		if (m_Value->m_Kind == YYTK::VALUE_OBJECT)
		{
			std::string rvalue_kind_name = YYTK::GetPrivateInterface()->RV_GetKindName(m_Value);
			std::string rvalue_specific_kind_name = YYTK::GetPrivateInterface()->RV_GetObjectSpecificKind(m_Value);

			if (!_stricmp(rvalue_kind_name.c_str(), rvalue_specific_kind_name.c_str()))
				return gcnew System::String(m_Value->GetKindName().c_str());

			return gcnew System::String(std::format("{} {}", rvalue_kind_name, rvalue_specific_kind_name).c_str());
		}

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

	Gen::IReadOnlyDictionary<System::String^, GameVariable^>^ GameVariable::Members::get()
	{
		std::map<std::string, YYTK::RValue*> my_map = m_Value->ToRefMap();

		auto managed_map = gcnew Gen::Dictionary<System::String^, GameVariable^>(static_cast<int>(my_map.size()));

		for (const auto& [Key, Value] : my_map)
		{
			System::String^ key = gcnew System::String(Key.c_str());
			GameVariable^ value = GameVariable::CreateFromRValue(*Value);
			managed_map->Add(key, value);
		}

		return managed_map;
	}
}