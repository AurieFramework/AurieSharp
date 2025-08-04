#include "GameInstance.hpp"
using namespace System;

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
		// Try to get the owning object
		auto owning_object = GetNativeInstance()->m_Object;

		if (!owning_object || !owning_object->m_Name)
			return gcnew String("Instance");

		return gcnew String(owning_object->m_Name);
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
}