#include <ibex3D/core/inputClass.h>

void inputClass::initialize()
{
	for (int i = 0; i < 256; i++)
	{
		m_keys[i] = 0;
	}
}

void inputClass::onKeyDown(unsigned int key)
{
	m_keys[key] = true;
}

void inputClass::onKeyUp(unsigned int key)
{
	m_keys[key] = false;
}

bool inputClass::isKeyDown(unsigned int key) const
{
	return m_keys[key];
}
