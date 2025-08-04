#include "GameObject.hpp"

using namespace System;

namespace YYTKInterop
{
	bool GameObject::IsInstance()
	{
		return this->m_Object->m_ObjectKind == YYTK::OBJECT_KIND_CINSTANCE;
	}

	String^ GameObject::Name::get()
	{
		return gcnew String("Object");
	}
}