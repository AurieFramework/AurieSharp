#include "GameObject.hpp"
#include "../Variable/GameVariable.hpp"
#include <msclr/marshal_cppstd.h>

using namespace msclr::interop;

namespace YYTKInterop
{
	bool GameObject::IsInstance()
	{
		return this->m_Object->m_ObjectKind == YYTK::OBJECT_KIND_CINSTANCE;
	}

	System::String^ GameObject::Name::get()
	{
		return gcnew System::String("Object");
	}

	Gen::IReadOnlyDictionary<System::String^, GameVariable^>^ GameObject::Members::get()
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

	GameVariable^ GameObject::default::get(System::String^ Name)
	{
		YYTK::RValue self = YYTK::RValue(this->m_Object);

		YYTK::RValue* native_value = self.GetRefMember(marshal_as<std::string>(Name));
		if (!native_value)
			throw gcnew System::InvalidCastException("Cannot access non-existing member of a struct variable!");

		return GameVariable::CreateFromRValue(*native_value);
	}

	void GameObject::default::set(System::String^ Name, GameVariable^ Value)
	{
		YYTK::RValue self = YYTK::RValue(this->m_Object);

		self[marshal_as<std::string>(Name)] = Value->ToRValue();
	}
}