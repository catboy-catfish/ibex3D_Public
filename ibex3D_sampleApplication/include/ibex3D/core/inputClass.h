#pragma once

class inputClass
{
public:
	void initialize();

	void onKeyDown(unsigned int key);
	void onKeyUp(unsigned int key);
	bool isKeyDown(unsigned int key) const;

private:
	bool m_keys[256];
};